#ifndef ARTCCEL_CORE_COMPUTE_COMPUTE_HPP
#define ARTCCEL_CORE_COMPUTE_COMPUTE_HPP
#pragma once

#include "../util/enum_bitset.hpp" // import util::Bitset_of, util::Enum_bitset, util::bitset_value, util::enum_bitset_operators
#include "../util/meta.hpp"        // import util::type_name
#include "../util/semantics.hpp"   // import util::Owner
#include "../util/utility_extras.hpp" // import util::forward_apply
#include <cinttypes>                  // import std::uint8_t
#include <concepts>   // import std::copyable, std::derived_from, std::invocable
#include <functional> // import std::function
#include <future>     // import std::packaged_task, std::shared_future
#include <memory> // import std::enable_shared_from_this, std::make_shared, std::make_unique, std::static_pointer_cast, std::unique_ptr, std::weak_ptr
#include <mutex> // import std::defer_lock, std::lock, std::mutex, std::unique_lock
#include <optional> // import std::optional
#include <string>   // import std::literals::string_literals
#include <type_traits> // import std::is_nothrow_move_constructible_v, std::remove_cv_t
#include <utility> // import std::forward, std::move, std::swap

namespace artccel::core::compute {
using namespace std::literals::string_literals;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace util::enum_bitset_operators;

template <std::copyable R> class Compute_io;
template <typename Signature> class Compute_in;
template <std::copyable R, R V> class Compute_constant;
template <std::copyable R> class Compute_value;
template <std::copyable R> class Compute_out;
enum struct Compute_option : std::uint8_t {
  identity = util::bitset_value(0U),
  concurrent = util::bitset_value(1U),
  defer = util::bitset_value(2U),
};
using Compute_options = util::Bitset_of<Compute_option>;
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Reset_t {
  consteval Reset_t() noexcept = default;
};
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Extract_t {
  consteval Extract_t() noexcept = default;
};
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Out_t {
  consteval Out_t() noexcept = default;
};

template <std::copyable R> class Compute_io {
public:
  using return_type = R;
  virtual auto clone_unmodified
      [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_io<R> &> = 0;
  virtual auto clone [[deprecated(/*u8*/ "Unsafe"),
                       nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_io<R> &> = 0;
  constexpr static auto clone_valid_options{Compute_option::concurrent |
                                            Compute_option::defer};
  virtual auto operator()() const -> R = 0;

  virtual ~Compute_io() noexcept = default;
  Compute_io(Compute_io<R> const &) = delete;
  auto operator=(Compute_io<R> const &) = delete;
  Compute_io(Compute_io<R> &&) = delete;
  auto operator=(Compute_io<R> &&) = delete;

protected:
  constexpr Compute_io() noexcept = default;
};

template <std::copyable R, typename... Args>
class Compute_in<R(Args...)>
    : public Compute_io<R>,
      public std::enable_shared_from_this<Compute_in<R(Args...)>> {
private:
  // NOLINTNEXTLINE(altera-struct-pack-align)
  struct Friend {
    consteval Friend() noexcept = default;
  };

public:
  using return_type = typename Compute_io<R>::return_type;
  using signature_type = R(Args...);
  template <typename... ForwardArgs>
  explicit Compute_in([[maybe_unused]] Friend /*unused*/, ForwardArgs &&...args)
      : Compute_in{std::forward<ForwardArgs>(args)...} {}

private:
  std::unique_ptr<std::mutex> const mutex_;
  std::function<signature_type> function_;
  std::function<R()> bound_;
  mutable bool invoked_{};
  mutable std::packaged_task<R()> task_{package(invoked_, bound_)};
  std::shared_future<R> future_{task_.get_future()};

protected:
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  explicit Compute_in(std::function<signature_type> function,
                      ForwardArgs &&...args)
      : Compute_in{Compute_option::concurrent | Compute_option::defer, function,
                   std::forward<ForwardArgs>(args)...} {}
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  Compute_in(Compute_options const &options,
             std::function<signature_type> function, ForwardArgs &&...args)
      : mutex_{(options & Compute_option::concurrent).any()
                   ? std::make_unique<std::mutex>()
                   : std::unique_ptr<std::mutex>{}},
        function_{std::move(function)}, bound_{bind(function_,
                                                    std::forward<ForwardArgs>(
                                                        args)...)},
        invoked_{(options & Compute_option::defer).none()} {
    constexpr static auto valid_options{Compute_option::concurrent |
                                        Compute_option::defer};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
  }
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  static auto bind(std::function<signature_type> function,
                   ForwardArgs &&...args) {
    return
        [function,
         ... args = std::forward<ForwardArgs>(
             args)]() mutable noexcept(noexcept(function(std::
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
  static auto package(bool invoke, std::function<R()> bound) {
    std::packaged_task<R()> ret{bound};
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
    return std::make_shared<Compute_in<signature_type> const>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }

public:
  template <typename... ForwardArgs>
  static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_in<signature_type>>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const [[nodiscard]] (ForwardArgs &&...args) {
    return create_const_0(std::forward<ForwardArgs>(args)...);
  }
  auto invoke() const {
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    if (!invoked_) {
      task_();
      invoked_ = true;
    }
    return std::shared_future{future_}.get();
  }
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
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
    invoked_ = (options & Compute_option::defer).none();
    task_ = package(invoked_, bound_);
    future_ = task_.get_future();
    if (invoked_) {
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
    if ((invoked_ = (options & Compute_option::defer).none())) {
      task_();
      return std::shared_future{future_}.get();
    }
    return {};
  }

  auto operator()() const -> R override { return invoke(); }
  template <template <typename...> typename Tuple, typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  void operator<<(Tuple<ForwardArgs...> &&t_args) {
    util::forward_apply(
        [this](ForwardArgs &&...args) mutable {
          bind(util::Enum_bitset{} | Compute_option::defer,
               std::forward<ForwardArgs>(args)...);
        },
        std::forward<Tuple<ForwardArgs...>>(t_args));
  }
  template <template <typename...> typename Tuple, typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  auto operator<<=(Tuple<ForwardArgs...> &&t_args) {
    return util::forward_apply(
        [this](ForwardArgs &&...args) mutable {
          return bind(util::Enum_bitset{} | Compute_option::identity,
                      std::forward<ForwardArgs>(args)...)
              .value();
        },
        std::forward<Tuple<ForwardArgs...>>(t_args));
  }
  void operator<<([[maybe_unused]] Reset_t /*unused*/) {
    reset(util::Enum_bitset{} | Compute_option::defer);
  }
  auto operator<<=([[maybe_unused]] Reset_t /*unused*/) {
    return reset(util::Enum_bitset{} | Compute_option::identity).value();
  }

  auto clone_unmodified [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_in<signature_type> &> override {
    return *new Compute_in{*this};
  };
  auto clone [[deprecated(/*u8*/ "Unsafe"),
               nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_in<signature_type> &> override {
    util::check_bitset(Compute_io<R>::clone_valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    return *new Compute_in{*this,
                           (options | Compute_option::concurrent).any()
                               ? std::make_unique<std::mutex>()
                               : std::unique_ptr<std::mutex>{},
                           (options | Compute_option::defer).none()};
  };
  ~Compute_in() noexcept override = default;

protected:
  void swap(Compute_in<signature_type> &other) noexcept {
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
  Compute_in(Compute_in<R(Args...)> const &other) noexcept(noexcept(Compute_in{
      other,
      other.mutex_ ? std::make_unique<std::mutex>()
                   : std::unique_ptr<std::mutex>{},
      other.invoked_}))
      : Compute_in{other,
                   other.mutex_ ? std::make_unique<std::mutex>()
                                : std::unique_ptr<std::mutex>{},
                   other.invoked_} {}
  auto operator=(Compute_in<R(Args...)> const &right) noexcept(
      noexcept(this == &right) &&noexcept(swap(right)) &&noexcept(*this))
      -> Compute_in<R(Args...)> & {
    if (this == &right) {
      // copy constructor cannot handle self-assignment
      return *this;
    }
    swap(right);
    return *this;
  };
  Compute_in(Compute_in<R(Args...)> &&other) noexcept
      : mutex_{other.mutex_ ? std::make_unique<std::mutex>()
                            : std::unique_ptr<std::mutex>{}},
        function_{std::move(other.function_)}, bound_{std::move(other.bound_)},
        invoked_{std::move(other.invoked_)}, task_{std::move(other.task_)},
        future_{std::move(other.future_)} {}
  auto operator=(Compute_in<R(Args...)> &&right) noexcept
      -> Compute_in<R(Args...)> & {
    Compute_in{std::move(right)}.swap(*this);
    return *this;
  };

  Compute_in(Compute_in<R(Args...)> const &other,
             std::remove_cv_t<decltype(mutex_)> mutex,
             decltype(invoked_) invoked) noexcept(noexcept(decltype(mutex_){
      std::move(mutex)}) &&noexcept(decltype(function_){
      other.function_}) &&noexcept(decltype(bound_){
      other.bound_}) &&noexcept(decltype(invoked_){
      invoked}) &&noexcept(decltype(task_){
      package(invoked_, bound_)}) &&noexcept(decltype(future_){
      task_.get_future()}))
      : mutex_{std::move(mutex)}, function_{other.function_},
        bound_{other.bound_}, invoked_{invoked} {}
};
template <std::copyable R, typename... Args>
explicit Compute_in(std::function<R(Args...)> function, auto &&...args)
    -> Compute_in<R(Args...)>;
template <std::copyable R, typename... Args>
Compute_in(Compute_options const &, std::function<R(Args...)> function,
           auto &&...args) -> Compute_in<R(Args...)>;

template <std::copyable R, R V>
class Compute_constant
    : public Compute_io<R>,
      public std::enable_shared_from_this<Compute_constant<R, V>> {
private:
  // NOLINTNEXTLINE(altera-struct-pack-align)
  struct Friend {
    consteval Friend() noexcept = default;
  };

public:
  using return_type = typename Compute_io<R>::return_type;
  constexpr static auto value_{V};
  template <typename... ForwardArgs>
  constexpr explicit Compute_constant([[maybe_unused]] Friend /*unused*/,
                                      ForwardArgs &&...args)
      : Compute_constant{std::forward<ForwardArgs>(args)...} {}

  template <typename... ForwardArgs>
  constexpr static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_constant<R, V>>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  constexpr static auto create_const [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_constant<R, V> const>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }
  constexpr auto operator() [[nodiscard]] () const -> R override {
    return value_;
  }

  auto clone_unmodified [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_constant<R, V> &> override {
    return *new Compute_constant{/* *this */};
  };
  auto clone [[deprecated(/*u8*/ "Unsafe"),
               nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_constant<R, V> &> override {
    util::check_bitset(Compute_io<R>::clone_valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    return *new Compute_constant{/* *this */};
  };
  constexpr ~Compute_constant() noexcept override = default;
  Compute_constant(Compute_constant<R, V> const &) = delete;
  auto operator=(Compute_constant<R, V> const &) = delete;
  Compute_constant(Compute_constant<R, V> &&) = delete;
  auto operator=(Compute_constant<R, V> &&) = delete;

protected:
  constexpr Compute_constant() noexcept = default;
};

template <std::copyable R>
class Compute_value : public Compute_io<R>,
                      public std::enable_shared_from_this<Compute_value<R>> {
private:
  // NOLINTNEXTLINE(altera-struct-pack-align)
  struct Friend {
    consteval Friend() noexcept = default;
  };

public:
  using return_type = typename Compute_io<R>::return_type;
  template <typename... ForwardArgs>
  explicit Compute_value([[maybe_unused]] Friend /*unused*/,
                         ForwardArgs &&...args)
      : Compute_value{std::forward<ForwardArgs>(args)...} {}

private:
  std::unique_ptr<std::mutex> const mutex_;
  R value_;

protected:
  explicit Compute_value(R const &value)
      : Compute_value{util::Enum_bitset{} | Compute_option::concurrent, value} {
  }
  Compute_value(Compute_options const &options, R const &value)
      : mutex_{(options & Compute_option::concurrent).any()
                   ? std::make_unique<std::mutex>()
                   : std::unique_ptr<std::mutex>{}},
        value_{value} {
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
    return std::make_shared<Compute_value<R> const>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }

public:
  template <typename... ForwardArgs>
  static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::make_shared<Compute_value<R>>(
        Friend{}, std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const [[nodiscard]] (ForwardArgs &&...args) {
    return create_const_0(std::forward<ForwardArgs>(args)...);
  }
  auto get [[nodiscard]] () const {
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    return value_;
  }
  auto set(R const &value) {
    auto value_copy{value};
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    using std::swap;
    swap(value_, value_copy);
    return value_copy;
  }

  auto operator() [[nodiscard]] () const -> R override { return get(); }
  auto operator<<(R const &value) { return set(value); }

  auto clone_unmodified [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_value<R> &> override {
    return *new Compute_value{*this};
  };
  auto clone [[deprecated(/*u8*/ "Unsafe"),
               nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_value<R> &> override {
    util::check_bitset(Compute_io<R>::clone_valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    return *new Compute_value{*this,
                              (options | Compute_option::concurrent).any()
                                  ? std::make_unique<std::mutex>()
                                  : std::unique_ptr<std::mutex>{}};
  };
  ~Compute_value() noexcept override = default;

protected:
  void swap(Compute_value<R> &other) noexcept {
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
  Compute_value(Compute_value<R> const &other) noexcept(noexcept(Compute_value{
      other, other.mutex_ ? std::make_unique<std::mutex>()
                          : std::unique_ptr<std::mutex>{}}))
      : Compute_value{other, other.mutex_ ? std::make_unique<std::mutex>()
                                          : std::unique_ptr<std::mutex>{}} {}
  auto operator=(Compute_value<R> const &right) noexcept(
      noexcept(Compute_value{right}.swap(*this)) &&noexcept(*this))
      -> Compute_value<R> & {
    Compute_value{right}.swap(*this);
    return *this;
  };
  Compute_value(Compute_value<R> &&other) noexcept
      : mutex_{other.mutex_ ? std::make_unique<std::mutex>()
                            : std::unique_ptr<std::mutex>{}},
        value_{std::move(other.value_)} {}
  auto operator=(Compute_value<R> &&right) noexcept -> Compute_value<R> & {
    Compute_value{std::move(right)}.swap(*this);
    return *this;
  };

  Compute_value(Compute_value<R> const &other,
                std::remove_cv_t<decltype(mutex_)>
                    mutex) noexcept(noexcept(decltype(mutex_){
      std::move(mutex)}) &&noexcept(decltype(value_){other.value_}))
      : mutex_{std::move(mutex)}, value_{other.value_} {}
};

template <std::copyable R> class Compute_out : private Compute_io<R> {
public:
  using return_type = typename Compute_io<R>::return_type;

private:
  std::weak_ptr<Compute_io<R> const> c_in_{};
  R return_{};

public:
  constexpr Compute_out() noexcept = default;
  template <std::derived_from<Compute_io<R>> In>
  requires std::derived_from<In, std::enable_shared_from_this<In>>
  explicit Compute_out(In const &c_in)
      : c_in_{std::static_pointer_cast<Compute_io<R> const>(
            static_cast<std::enable_shared_from_this<In> const &>(c_in)
                .shared_from_this())},
        return_{c_in()} {}

  auto get [[nodiscard]] () const
      noexcept(noexcept(return_) &&
               std::is_nothrow_move_constructible_v<decltype(return_)>) {
    return return_;
  }
  auto extract() {
    if (auto const c_in{c_in_.lock()}) {
      return_ = (*c_in)();
    }
    return return_;
  }

  auto operator() [[nodiscard]] () const
      noexcept(noexcept(R{get()}) && std::is_nothrow_move_constructible_v<R>)
          -> R override {
    return get();
  }
  auto operator()([[maybe_unused]] Extract_t /*unused*/) { return extract(); }
  auto operator>>(R &right) const
      noexcept(noexcept(right = get()) &&
               std::is_nothrow_move_constructible_v<decltype(right = get())>) {
    return right = get();
  }
  auto operator>>=(R &right) { return right = extract(); }

  auto clone_unmodified [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> util::Owner<Compute_out &> override {
    return *new Compute_out{*this};
  };
  auto clone [[deprecated(/*u8*/ "Unsafe"),
               nodiscard]] (Compute_options const &options) const
      -> util::Owner<Compute_out &> override {
    util::check_bitset(Compute_io<R>::clone_valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    return *new Compute_out{*this};
  };
  ~Compute_out() noexcept override = default;
  void swap(Compute_out<R> &other) noexcept {
    using std::swap;
    swap(c_in_, other.c_in_);
    swap(return_, other.return_);
  }
  Compute_out(Compute_out<R> const &other) noexcept(noexcept(decltype(c_in_){
      other.c_in_}) &&noexcept(decltype(return_){other.return_}))
      : c_in_{other.c_in_}, return_{other.return_} {}
  auto operator=(Compute_out<R> const &right) noexcept(noexcept(
      Compute_out{right}.swap(*this)) &&noexcept(*this)) -> Compute_out<R> & {
    Compute_out{right}.swap(*this);
    return *this;
  };
  Compute_out(Compute_out<R> &&other) noexcept
      : c_in_{std::move(other.c_in_)}, return_{std::move(other.return_)} {}
  auto operator=(Compute_out<R> &&right) noexcept -> Compute_out<R> & {
    Compute_out{std::move(right)}.swap(*this);
    return *this;
  };
};
template <std::copyable R>
explicit Compute_out(Compute_io<R> const &) -> Compute_out<R>;
template <std::copyable R>
void swap(Compute_out<R> &left, Compute_out<R> &right) noexcept {
  left.swap(right);
}

auto operator<<([[maybe_unused]] Out_t /*unused*/, auto const &right) {
  return Compute_out{right};
}
} // namespace artccel::core::compute

#endif
