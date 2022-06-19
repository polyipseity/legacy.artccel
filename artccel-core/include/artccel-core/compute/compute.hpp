#ifndef ARTCCEL_CORE_COMPUTE_COMPUTE_HPP
#define ARTCCEL_CORE_COMPUTE_COMPUTE_HPP
#pragma once

#include "../util/concurrent.hpp" // import util::Nullable_lockable, util::Semiregular_once_flag
#include "../util/enum_bitset.hpp" // import util::Bitset_of, util::Enum_bitset, util::bitset_value, util::enum_bitset_operators
#include "../util/meta.hpp"        // import util::type_name
#include "../util/semantics.hpp"   // import util::Owner
#include "../util/utility_extras.hpp" // import util::forward_apply
#include <cassert>                    // import assert
#include <cinttypes>                  // import std::uint8_t
#include <concepts>   // import std::copyable, std::derived_from, std::invocable
#include <functional> // import std::function, std::invoke
#include <memory> // import std::enable_shared_from_this, std::make_shared, std::make_unique, std::weak_ptr
#include <mutex>  // import std::lock_guard, std::mutex, std::scoped_lock
#include <optional>    // import std::nullopt, std::optional
#include <string>      // import std::literals::string_literals
#include <type_traits> // import std::is_invocable_r_v, std::is_nothrow_move_constructible_v, std::remove_cv_t
#include <utility> // import std::declval, std::exchange, std::forward, std::move, std::swap

namespace artccel::core::compute {
using namespace std::literals::string_literals;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace util::enum_bitset_operators;

enum struct Compute_option : std::uint8_t;
using Compute_options = util::Bitset_of<Compute_option>;
template <std::copyable R> class Compute_io;
template <typename Derived, std::copyable R> class Compute_in;
template <std::copyable R> class Compute_out;
template <std::copyable R, R V> class Compute_constant;
template <std::copyable R, auto F>
requires std::is_invocable_r_v<R, decltype(F)>
class Compute_function_constant;
template <std::copyable R> class Compute_value;
template <typename Signature> class Compute_function;
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Reset_t {
  explicit consteval Reset_t() noexcept = default;
};
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Extract_t {
  explicit consteval Extract_t() noexcept = default;
};
struct Out_t;

template <typename T, typename R>
concept Compute_in_c =
    std::derived_from<T, Compute_in<T, R>> && std::copyable<R>;
template <typename T>
concept Compute_in_any_c = Compute_in_c<T, typename T::return_type>;

enum struct Compute_option : std::uint8_t {
  identity = util::bitset_value(0U),
  concurrent = util::bitset_value(1U),
  defer = util::bitset_value(2U),
};

template <std::copyable R> class Compute_io {
public:
  using return_type = R;
  virtual auto operator()() const -> R = 0;

  virtual ~Compute_io() noexcept = default;
  Compute_io(Compute_io const &) = delete;
  auto operator=(Compute_io const &) = delete;
  Compute_io(Compute_io &&) = delete;
  auto operator=(Compute_io &&) = delete;

protected:
  constexpr Compute_io() noexcept = default;
};

template <typename Derived, std::copyable R>
class Compute_in : public Compute_io<R>,
                   public std::enable_shared_from_this<Derived> {
public:
  using return_type = typename Compute_in::return_type;
  virtual auto clone_unmodified
      [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_in &> = 0;
  constexpr static auto clone_valid_options{Compute_option::concurrent |
                                            Compute_option::defer};
  virtual auto clone [[deprecated(/*u8*/ "Unsafe"),
                       nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_in &> = 0;

protected:
  using Compute_in::Compute_io::Compute_io;
};

template <std::copyable R> class Compute_out : public Compute_io<R> {
public:
  using return_type = typename Compute_out::return_type;
  using in_type = Compute_io<R>;

private:
  std::weak_ptr<in_type const> c_in_{};
  R return_{};

public:
  constexpr Compute_out() noexcept = default;
  explicit Compute_out(Compute_in_c<R> auto const &c_in)
      : c_in_{c_in.weak_from_this()}, return_{c_in()} {}

  auto operator() [[nodiscard]] () const
      noexcept(noexcept(R{return_}) && std::is_nothrow_move_constructible_v<R>)
          -> R override {
    return return_;
  }
  auto operator()([[maybe_unused]] Extract_t /*unused*/) -> R {
    if (auto const c_in{c_in_.lock()}) {
      return_ = *c_in();
    }
    return return_;
  }
  auto operator>>(R &right) const
      noexcept(noexcept(R{right = *this()}) &&
               std::is_nothrow_move_constructible_v<R>) -> R {
    return right = *this();
  }
  auto operator>>=(R &right) -> R { return right = *this(Extract_t{}); }

  ~Compute_out() noexcept override = default;
  constexpr void swap(Compute_out &other) noexcept {
    using std::swap;
    swap(c_in_, other.c_in_);
    swap(return_, other.return_);
  }
  Compute_out(Compute_out const &other) noexcept(
      noexcept(decltype(c_in_){other.c_in_}, decltype(return_){other.return_}))
      : c_in_{other.c_in_}, return_{other.return_} {}
  auto operator=(Compute_out const &right) noexcept(
      noexcept(Compute_out{right}.swap(*this), *this)) -> Compute_out & {
    Compute_out{right}.swap(*this);
    return *this;
  };
  Compute_out(Compute_out &&other) noexcept
      : c_in_{std::move(other.c_in_)}, return_{std::move(other.return_)} {}
  auto operator=(Compute_out &&right) noexcept -> Compute_out & {
    Compute_out{std::move(right)}.swap(*this);
    return *this;
  };
};
template <std::copyable R> Compute_out(Compute_io<R> const &) -> Compute_out<R>;
template <std::copyable R>
constexpr void swap(Compute_out<R> &left, Compute_out<R> &right) noexcept {
  left.swap(right);
}

template <std::copyable R, R V>
class Compute_constant : public Compute_in<Compute_constant<R, V>, R> {
private:
  // NOLINTNEXTLINE(altera-struct-pack-align)
  struct Friend {
    consteval Friend() noexcept = default;
  };

public:
  using return_type = typename Compute_constant::return_type;
  constexpr static auto value_{V};
  template <typename... ForwardArgs>
  constexpr explicit Compute_constant([[maybe_unused]] Friend /*unused*/,
                                      ForwardArgs &&...args)
      : Compute_constant{std::forward<ForwardArgs>(args)...} {}

  template <typename... ForwardArgs>
  constexpr static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_constant>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  constexpr static auto create_const [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_constant const>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }
  constexpr auto operator() [[nodiscard]] () const
      noexcept(noexcept(R{value_}) && std::is_nothrow_move_constructible_v<R>)
          -> R override {
    return value_;
  }

  auto clone_unmodified [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_constant &> override {
    return *new Compute_constant{/* *this */};
  };
  auto clone [[deprecated(/*u8*/ "Unsafe"),
               nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_constant &> override {
    util::check_bitset(Compute_constant::clone_valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    return *new Compute_constant{/* *this */};
  };
  constexpr ~Compute_constant() noexcept override = default;
  Compute_constant(Compute_constant const &) = delete;
  auto operator=(Compute_constant const &) = delete;
  Compute_constant(Compute_constant &&) = delete;
  auto operator=(Compute_constant &&) = delete;

protected:
  constexpr Compute_constant() noexcept = default;
};

template <std::copyable R, auto F>
requires std::is_invocable_r_v<R, decltype(F)>
class Compute_function_constant
    : public Compute_in<Compute_function_constant<R, F>, R> {
private:
  // NOLINTNEXTLINE(altera-struct-pack-align)
  struct Friend {
    consteval Friend() noexcept = default;
  };

public:
  using return_type = typename Compute_function_constant::return_type;
  constexpr static auto function_{F};
  template <typename... ForwardArgs>
  constexpr explicit Compute_function_constant(
      [[maybe_unused]] Friend /*unused*/, ForwardArgs &&...args)
      : Compute_function_constant{std::forward<ForwardArgs>(args)...} {}

  template <typename... ForwardArgs>
  constexpr static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_function_constant>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  constexpr static auto create_const [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_function_constant const>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }
  constexpr auto operator() [[nodiscard]] () const
      noexcept(noexcept(R{std::invoke(F)}) &&
               std::is_nothrow_move_constructible_v<R>) -> R override {
    return std::invoke(F);
  }

  auto clone_unmodified [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_function_constant &> override {
    return *new Compute_function_constant{/* *this */};
  };
  auto clone [[deprecated(/*u8*/ "Unsafe"),
               nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_function_constant &> override {
    util::check_bitset(Compute_function_constant::clone_valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    return *new Compute_function_constant{/* *this */};
  };
  constexpr ~Compute_function_constant() noexcept override = default;
  Compute_function_constant(Compute_function_constant const &) = delete;
  auto operator=(Compute_function_constant const &) = delete;
  Compute_function_constant(Compute_function_constant &&) = delete;
  auto operator=(Compute_function_constant &&) = delete;

protected:
  constexpr Compute_function_constant() noexcept = default;
};

template <std::copyable R>
class Compute_value : public Compute_in<Compute_value<R>, R> {
private:
  // NOLINTNEXTLINE(altera-struct-pack-align)
  struct Friend {
    consteval Friend() noexcept = default;
  };

public:
  using return_type = typename Compute_value::return_type;
  template <typename... ForwardArgs>
  explicit Compute_value([[maybe_unused]] Friend /*unused*/,
                         ForwardArgs &&...args)
      : Compute_value{std::forward<ForwardArgs>(args)...} {}

private:
  util::Nullable_lockable<std::mutex> const mutex_;
  R value_;

protected:
  explicit Compute_value(R value)
      : Compute_value{util::Enum_bitset{} | Compute_option::concurrent,
                      std::move(value)} {}
  Compute_value(Compute_options const &options, R value)
      : mutex_{(options & Compute_option::concurrent).any()
                   ? std::make_unique<std::mutex>()
                   : nullptr},
        value_{std::move(value)} {
    constexpr static auto valid_options{util::Enum_bitset{} |
                                        Compute_option::concurrent};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
  }

private:
  template <typename... ForwardArgs>
  static auto create_const_0
      [[nodiscard]] (Compute_options const &options, ForwardArgs &&...args) {
    constexpr static auto valid_options{~Compute_option::concurrent};
    util::check_bitset(valid_options,
                       u8"Unnecessary "s + util::type_name<Compute_option>(),
                       options);
    return create_const_1(options, std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const_0 [[nodiscard]] (ForwardArgs &&...args) {
    return create_const_1(util::Enum_bitset{} | Compute_option::identity,
                          std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const_1 [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_value const>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }

public:
  template <typename... ForwardArgs>
  static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_value>(Friend{},
                                           std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const [[nodiscard]] (ForwardArgs &&...args) {
    return create_const_0(std::forward<ForwardArgs>(args)...);
  }

  auto operator() [[nodiscard]] () const -> R override {
    std::lock_guard const guard{mutex_};
    return value_;
  }
  auto operator<<(R const &value) -> R { return *this << R{value}; }
  auto operator<<(R &&value) -> R {
    std::lock_guard const guard{mutex_};
    return std::exchange(value_, std::move(value));
  }
  auto operator<<=(R const &value) -> R { return *this <<= R{value}; }
  auto operator<<=(R &&value) -> R {
    std::lock_guard const guard{mutex_};
    return value_ = std::move(value);
  }

  auto clone_unmodified [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_value &> override {
    return *new Compute_value{*this};
  };
  auto clone [[deprecated(/*u8*/ "Unsafe"),
               nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_value &> override {
    util::check_bitset(Compute_value::clone_valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    return *new Compute_value{*this,
                              (options | Compute_option::concurrent).any()
                                  ? std::make_unique<std::mutex>()
                                  : nullptr};
  };
  ~Compute_value() noexcept override = default;

protected:
  void swap(Compute_value &other) noexcept {
    std::scoped_lock const guard{mutex_, other.mutex_};
    using std::swap;
    swap(value_, other.value_);
  }
  Compute_value(Compute_value const &other) noexcept(noexcept(Compute_value{
      other, other.mutex_ ? std::make_unique<std::mutex>() : nullptr}))
      : Compute_value{other, other.mutex_ ? std::make_unique<std::mutex>()
                                          : nullptr} {}
  auto operator=(Compute_value const &right) noexcept(
      noexcept(Compute_value{right}.swap(*this), *this)) -> Compute_value & {
    Compute_value{right}.swap(*this);
    return *this;
  };
  Compute_value(Compute_value &&other) noexcept
      : mutex_{other.mutex_ ? std::make_unique<std::mutex>() : nullptr},
        value_{std::move(other.value_)} {}
  auto operator=(Compute_value &&right) noexcept -> Compute_value & {
    Compute_value{std::move(right)}.swap(*this);
    return *this;
  };

  Compute_value(Compute_value const &other,
                std::remove_cv_t<decltype(mutex_)>
                    mutex) noexcept(noexcept(decltype(mutex_){std::move(mutex)},
                                             decltype(value_){other.value_}))
      : mutex_{std::move(mutex)}, value_{other.value_} {}
};

template <std::copyable R, std::copyable... Args>
class Compute_function<R(Args...)>
    : public Compute_in<Compute_function<R(Args...)>, R> {
private:
  // NOLINTNEXTLINE(altera-struct-pack-align)
  struct Friend {
    consteval Friend() noexcept = default;
  };

public:
  using return_type = typename Compute_function::return_type;
  using signature_type = R(Args...);
  template <typename... ForwardArgs>
  explicit Compute_function([[maybe_unused]] Friend /*unused*/,
                            ForwardArgs &&...args)
      : Compute_function{std::forward<ForwardArgs>(args)...} {}

protected:
  enum class Bound_action : bool {
    compute = false,
    reset = true,
  };

private:
  util::Nullable_lockable<std::mutex> const mutex_;
  std::function<signature_type> function_;
  std::function<std::optional<R>(Bound_action)> bound_;

protected:
  template <typename F, typename... ForwardArgs>
  requires std::invocable<F, ForwardArgs...>
  explicit Compute_function(F &&function, ForwardArgs &&...args)
      : Compute_function{Compute_option::concurrent | Compute_option::defer,
                         std::forward<F>(function),
                         std::forward<ForwardArgs>(args)...} {}
  template <typename F, typename... ForwardArgs>
  requires std::invocable<F, ForwardArgs...>
  Compute_function(Compute_options const &options, F &&function,
                   ForwardArgs &&...args)
      : mutex_{(options & Compute_option::concurrent).any()
                   ? std::make_unique<std::mutex>()
                   : nullptr},
        function_{std::forward<F>(function)},
        bound_{bind((options & Compute_option::defer).none(), function_,
                    std::forward<ForwardArgs>(args)...)} {
    constexpr static auto valid_options{Compute_option::concurrent |
                                        Compute_option::defer};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
  }
  template <typename... ForwardArgs>
  requires std::invocable<decltype(function_), ForwardArgs...>
  static auto bind(bool invoke, decltype(function_) const &function,
                   ForwardArgs &&...args) {
    auto bound{[flag{util::Semiregular_once_flag{}}, ret{std::optional<R>{}},
                function, ... args{std::forward<ForwardArgs>(args)}](
                   Bound_action action) mutable {
      switch (action) {
      case Bound_action::compute:
        flag.call_once(
            [&ret, &function](ForwardArgs &&...args) noexcept(
                noexcept(ret = function(std::forward<ForwardArgs>(args)...))) {
              ret = function(std::forward<ForwardArgs>(args)...);
            },
            std::forward<ForwardArgs>(args)...);
        return ret;
      case Bound_action::reset:
        flag = {};
        return std::optional<R>{};
      default:
        // clang-format off
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
        /* clang-format on */ assert(false);
        break;
      }
    }};
    if (invoke) {
      bound(Bound_action::compute);
    }
    return bound;
  }

private:
  template <typename... ForwardArgs>
  static auto create_const_0
      [[nodiscard]] (Compute_options const &options, ForwardArgs &&...args) {
    constexpr static auto valid_options{~Compute_option::concurrent};
    util::check_bitset(valid_options,
                       u8"Unnecessary "s + util::type_name<Compute_option>(),
                       options);
    return create_const_1(options, std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const_0 [[nodiscard]] (ForwardArgs &&...args) {
    return create_const_1(util::Enum_bitset{} | Compute_option::defer,
                          std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const_1 [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_function const>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }

public:
  template <typename... ForwardArgs>
  static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_function>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const [[nodiscard]] (ForwardArgs &&...args) {
    return create_const_0(std::forward<ForwardArgs>(args)...);
  }

  template <typename... ForwardArgs>
  requires std::invocable<decltype(function_), ForwardArgs...>
  auto bind(Compute_options const &options, ForwardArgs &&...args)
      -> std::optional<R> {
    constexpr static auto valid_options{util::Enum_bitset{} |
                                        Compute_option::defer};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    auto const invoke{(options & Compute_option::defer).none()};
    std::lock_guard const guard{mutex_};
    bound_ = bind(invoke, function_, std::forward<ForwardArgs>(args)...);
    return invoke ? bound_(Bound_action::compute) : std::nullopt;
  }
  auto reset(Compute_options const &options) -> std::optional<R> {
    constexpr static auto valid_options{util::Enum_bitset{} |
                                        Compute_option::defer};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    auto const invoke{(options & Compute_option::defer).none()};
    std::lock_guard const guard{mutex_};
    bound_(Bound_action::reset);
    return invoke ? bound_(Bound_action::compute) : std::nullopt;
  }

  auto operator()() const -> R override {
    std::lock_guard const guard{mutex_};
    return bound_(Bound_action::compute).value();
  }
  template <template <typename...> typename Tuple, typename... ForwardArgs>
  requires std::invocable<decltype(function_), ForwardArgs...>
  void operator<<(Tuple<ForwardArgs...> &&t_args) {
    util::forward_apply(
        [this](ForwardArgs &&...args) mutable {
          bind(util::Enum_bitset{} | Compute_option::defer,
               std::forward<ForwardArgs>(args)...);
        },
        std::forward<Tuple<ForwardArgs...>>(t_args));
  }
  template <template <typename...> typename Tuple, typename... ForwardArgs>
  requires std::invocable<decltype(function_), ForwardArgs...>
  auto operator<<=(Tuple<ForwardArgs...> &&t_args) -> R {
    return util::forward_apply(
        [this](ForwardArgs &&...args) mutable -> decltype(auto) {
          return bind(util::Enum_bitset{} | Compute_option::identity,
                      std::forward<ForwardArgs>(args)...)
              .value();
        },
        std::forward<Tuple<ForwardArgs...>>(t_args));
  }
  void operator<<([[maybe_unused]] Reset_t /*unused*/) {
    reset(util::Enum_bitset{} | Compute_option::defer);
  }
  auto operator<<=([[maybe_unused]] Reset_t /*unused*/) -> R {
    return reset(util::Enum_bitset{} | Compute_option::identity).value();
  }

  auto clone_unmodified [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_function &> override {
    return *new Compute_function{*this};
  };
  auto clone [[deprecated(/*u8*/ "Unsafe"),
               nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_function &> override {
    util::check_bitset(Compute_function::clone_valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    return *new Compute_function{*this,
                                 (options | Compute_option::concurrent).any()
                                     ? std::make_unique<std::mutex>()
                                     : nullptr};
  };
  ~Compute_function() noexcept override = default;

protected:
  void swap(Compute_function &other) noexcept {
    std::scoped_lock const guard{mutex_, other.mutex_};
    using std::swap;
    swap(function_, other.function_);
    swap(bound_, other.bound_);
  }
  Compute_function(Compute_function const &other) noexcept(
      noexcept(Compute_function{
          other, other.mutex_ ? std::make_unique<std::mutex>() : nullptr}))
      : Compute_function{other, other.mutex_ ? std::make_unique<std::mutex>()
                                             : nullptr} {}
  auto operator=(Compute_function const &right) noexcept(
      noexcept(this == &right, swap(right), *this)) -> Compute_function & {
    Compute_function{right}.swap(*this);
    return *this;
  };
  Compute_function(Compute_function &&other) noexcept
      : mutex_{other.mutex_ ? std::make_unique<std::mutex>() : nullptr},
        function_{std::move(other.function_)}, bound_{std::move(other.bound_)} {
  }
  auto operator=(Compute_function &&right) noexcept -> Compute_function & {
    Compute_function{std::move(right)}.swap(*this);
    return *this;
  };

  Compute_function(
      Compute_function const &other,
      std::remove_cv_t<decltype(mutex_)>
          mutex) noexcept(noexcept(decltype(mutex_){std::move(mutex)},
                                   decltype(function_){other.function_},
                                   decltype(bound_){other.bound_}))
      : mutex_{std::move(mutex)}, function_{other.function_},
        bound_{other.bound_} {}
};
template <typename F>
Compute_function(F &&, auto &&...) -> Compute_function<
    decltype(decltype(std::function{std::declval<F>()})::operator())>;
template <typename F>
Compute_function(Compute_options const &, F &&, auto &&...) -> Compute_function<
    decltype(decltype(std::function{std::declval<F>()})::operator())>;

// NOLINTNEXTLINE(altera-struct-pack-align)
struct Out_t {
  explicit consteval Out_t() noexcept = default;
  auto operator<<(Compute_in_any_c auto const &right) {
    return Compute_out{right};
  }
};
} // namespace artccel::core::compute

#endif
