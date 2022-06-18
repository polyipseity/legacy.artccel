#ifndef ARTCCEL_CORE_COMPUTE_COMPUTE_HPP
#define ARTCCEL_CORE_COMPUTE_COMPUTE_HPP
#pragma once

#include "../util/enum_bitset.hpp" // import util::Bitset_of, util::Enum_bitset, util::bitset_value, util::enum_bitset_operators
#include "../util/meta.hpp"        // import util::type_name
#include "../util/semantics.hpp"   // import util::Owner
#include "../util/utility_extras.hpp" // import util::forward_apply
#include <cinttypes>                  // import std::uint8_t
#include <concepts>   // import std::copyable, std::derived_from, std::invocable
#include <functional> // import std::function, std::invoke
#include <future>     // import std::packaged_task, std::shared_future
#include <memory> // import std::enable_shared_from_this, std::make_shared, std::make_unique, std::unique_ptr, std::weak_ptr
#include <mutex> // import std::defer_lock, std::lock, std::mutex, std::unique_lock
#include <optional>    // import std::optional
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
  consteval Reset_t() noexcept = default;
};
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Extract_t {
  consteval Extract_t() noexcept = default;
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
  constexpr Compute_in() noexcept = default;
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
  Compute_out(Compute_out const &other) noexcept(noexcept(decltype(c_in_){
      other.c_in_}) &&noexcept(decltype(return_){other.return_}))
      : c_in_{other.c_in_}, return_{other.return_} {}
  auto operator=(Compute_out const &right) noexcept(noexcept(
      Compute_out{right}.swap(*this)) &&noexcept(*this)) -> Compute_out & {
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
  std::unique_ptr<std::mutex> const mutex_;
  R value_;

protected:
  explicit Compute_value(R value)
      : Compute_value{util::Enum_bitset{} | Compute_option::concurrent,
                      std::move(value)} {}
  Compute_value(Compute_options const &options, R value)
      : mutex_{(options & Compute_option::concurrent).any()
                   ? std::make_unique<std::mutex>()
                   : std::unique_ptr<std::mutex>{}},
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
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    return value_;
  }
  auto operator<<(R const &value) -> R { return *this << R{value}; }
  auto operator<<(R &&value) -> R {
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    return std::exchange(value_, std::move(value));
  }
  auto operator<<=(R const &value) -> R { return *this <<= R{value}; }
  auto operator<<=(R &&value) -> R {
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
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
                                  : std::unique_ptr<std::mutex>{}};
  };
  ~Compute_value() noexcept override = default;

protected:
  void swap(Compute_value &other) noexcept {
    auto this_guard{mutex_ ? std::unique_lock{*mutex_, std::defer_lock}
                           : std::unique_lock<std::mutex>{}};
    auto other_guard{other.mutex_
                         ? std::unique_lock{other.*mutex_, std::defer_lock}
                         : std::unique_lock<std::mutex>{}};
    if (this_guard.mutex() && other_guard.mutex()) {
      std::lock(this_guard, other_guard);
    } else if (this_guard.mutex() != nullptr) {
      this_guard.lock();
    } else if (other_guard.mutex()) {
      other_guard.lock();
    }
    using std::swap;
    swap(value_, other.value_);
  }
  Compute_value(Compute_value const &other) noexcept(noexcept(Compute_value{
      other, other.mutex_ ? std::make_unique<std::mutex>()
                          : std::unique_ptr<std::mutex>{}}))
      : Compute_value{other, other.mutex_ ? std::make_unique<std::mutex>()
                                          : std::unique_ptr<std::mutex>{}} {}
  auto operator=(Compute_value const &right) noexcept(noexcept(
      Compute_value{right}.swap(*this)) &&noexcept(*this)) -> Compute_value & {
    Compute_value{right}.swap(*this);
    return *this;
  };
  Compute_value(Compute_value &&other) noexcept
      : mutex_{other.mutex_ ? std::make_unique<std::mutex>()
                            : std::unique_ptr<std::mutex>{}},
        value_{std::move(other.value_)} {}
  auto operator=(Compute_value &&right) noexcept -> Compute_value & {
    Compute_value{std::move(right)}.swap(*this);
    return *this;
  };

  Compute_value(Compute_value const &other,
                std::remove_cv_t<decltype(mutex_)>
                    mutex) noexcept(noexcept(decltype(mutex_){
      std::move(mutex)}) &&noexcept(decltype(value_){other.value_}))
      : mutex_{std::move(mutex)}, value_{other.value_} {}
};

template <std::copyable R, typename... Args>
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

private:
  std::unique_ptr<std::mutex> const mutex_;
  std::function<signature_type> function_;
  std::function<R()> bound_;
  mutable std::packaged_task<R()> task_;
  std::shared_future<R> future_{task_.get_future()};
  mutable bool invoked_{false};

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
                   : std::unique_ptr<std::mutex>{}},
        function_{std::forward<F>(function)},
        bound_{bind(function_, std::forward<ForwardArgs>(args)...)},
        task_{package((options & Compute_option::defer).none(), bound_)},
        invoked_{(options & Compute_option::defer).none()} {
    constexpr static auto valid_options{Compute_option::concurrent |
                                        Compute_option::defer};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
  }
  template <typename... ForwardArgs>
  requires std::invocable<decltype(function_), ForwardArgs...>
  static auto bind(decltype(function_) const &function, ForwardArgs &&...args) {
    return
        [function,
         ... args{std::forward<ForwardArgs>(
             args)}]() mutable noexcept(noexcept(function(std::
                                                              forward<
                                                                  ForwardArgs>(
                                                                  args)...)) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(function(
                                                std::forward<ForwardArgs>(
                                                    args)...))>) {
          return function(std::forward<ForwardArgs>(args)...);
        };
  }
  static auto package(bool invoke, decltype(bound_) const &bound) {
    decltype(task_) ret{bound};
    if (invoke) {
      ret();
    }
    return ret;
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
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    bound_ = bind(function_, std::forward<ForwardArgs>(args)...);
    auto const invoke{(options & Compute_option::defer).none()};
    task_ = package(invoke, bound_);
    future_ = task_.get_future();
    if ((invoked_ = invoke)) {
      return std::shared_future{future_}.get();
    }
    return {};
  }
  auto reset(Compute_options const &options) -> std::optional<R> {
    constexpr static auto valid_options{util::Enum_bitset{} |
                                        Compute_option::defer};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    task_.reset();
    future_ = task_.get_future();
    if ((options & Compute_option::defer).none()) {
      task_();
      invoked_ = true;
      return std::shared_future{future_}.get();
    }
    return {};
  }

  auto operator()() const -> R override {
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    if (!invoked_) {
      task_();
      invoked_ = true;
    }
    return std::shared_future{future_}.get();
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
                                     : std::unique_ptr<std::mutex>{},
                                 (options | Compute_option::defer).none()};
  };
  ~Compute_function() noexcept override = default;

protected:
  void swap(Compute_function &other) noexcept {
    auto this_guard{mutex_ ? std::unique_lock{*mutex_, std::defer_lock}
                           : std::unique_lock<std::mutex>{}};
    auto other_guard{other.mutex_
                         ? std::unique_lock{other.*mutex_, std::defer_lock}
                         : std::unique_lock<std::mutex>{}};
    if (this_guard.mutex() && other_guard.mutex()) {
      std::lock(this_guard, other_guard);
    } else if (this_guard.mutex() != nullptr) {
      this_guard.lock();
    } else if (other_guard.mutex()) {
      other_guard.lock();
    }
    using std::swap;
    swap(function_, other.function_);
    swap(bound_, other.bound_);
    swap(invoked_, other.invoked_);
    swap(task_, other.task_);
    swap(future_, other.future_);
  }
  Compute_function(Compute_function const &other) noexcept(
      noexcept(Compute_function{other,
                                other.mutex_ ? std::make_unique<std::mutex>()
                                             : std::unique_ptr<std::mutex>{},
                                other.invoked_}))
      : Compute_function{other,
                         other.mutex_ ? std::make_unique<std::mutex>()
                                      : std::unique_ptr<std::mutex>{},
                         other.invoked_} {}
  auto operator=(Compute_function const &right) noexcept(
      noexcept(this == &right) &&noexcept(swap(right)) &&noexcept(*this))
      -> Compute_function & {
    if (this == &right) {
      // copy constructor cannot handle self-assignment
      return *this;
    }
    swap(right);
    return *this;
  };
  Compute_function(Compute_function &&other) noexcept
      : mutex_{other.mutex_ ? std::make_unique<std::mutex>()
                            : std::unique_ptr<std::mutex>{}},
        function_{std::move(other.function_)}, bound_{std::move(other.bound_)},
        task_{std::move(other.task_)}, future_{std::move(other.future_)},
        invoked_{std::move(other.invoked_)} {}
  auto operator=(Compute_function &&right) noexcept -> Compute_function & {
    Compute_function{std::move(right)}.swap(*this);
    return *this;
  };

  Compute_function(
      Compute_function const &other, std::remove_cv_t<decltype(mutex_)> mutex,
      decltype(invoked_) invoked) noexcept(noexcept(decltype(mutex_){
      std::move(mutex)}) &&noexcept(decltype(function_){
      other.function_}) &&noexcept(decltype(bound_){
      other.bound_}) &&noexcept(decltype(task_){
      package(invoked, bound_)}) &&noexcept(decltype(future_){
      task_.get_future()}) &&noexcept(decltype(invoked_){invoked}))
      : mutex_{std::move(mutex)}, function_{other.function_},
        bound_{other.bound_}, task_{package(invoked, bound_)}, invoked_{
                                                                   invoked} {}
};
template <typename F>
Compute_function(F &&, auto &&...) -> Compute_function<
    decltype(decltype(std::function{std::declval<F>()})::operator())>;
template <typename F>
Compute_function(Compute_options const &, F &&, auto &&...) -> Compute_function<
    decltype(decltype(std::function{std::declval<F>()})::operator())>;

// NOLINTNEXTLINE(altera-struct-pack-align)
struct Out_t {
  consteval Out_t() noexcept = default;
  auto operator<<(Compute_in_any_c auto const &right) {
    return Compute_out{right};
  }
};
} // namespace artccel::core::compute

#endif
