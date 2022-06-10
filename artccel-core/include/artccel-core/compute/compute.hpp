#ifndef ARTCCEL_CORE_COMPUTE_COMPUTE_HPP
#define ARTCCEL_CORE_COMPUTE_COMPUTE_HPP
#pragma once

#include "../util/enum_bitset.hpp" // import util::Enum_bitset, util::bitset_of, util::bitset_operators, util::bitset_value
#include "../util/reflect.hpp"     // import util::type_name
#include <cinttypes>               // import std::uint8_t
#include <concepts>   // import std::copyable, std::derived_from, std::invocable
#include <functional> // import std::function
#include <future>     // import std::packaged_task, std::shared_future
#include <memory> // import std::enable_shared_from_this, std::make_unique, std::shared_ptr, std::static_pointer_cast, std::unique_ptr, std::weak_ptr
#include <mutex>  // import std::mutex, std::unique_lock
#include <optional>    // import std::optional
#include <string>      // import std::literals::string_literals
#include <type_traits> // import std::is_function_v, std::is_nothrow_copy_constructible_v, std::is_nothrow_move_constructible_v
#include <utility>     // import std::forward, std::move, std::swap

namespace artccel::core::compute {
using namespace std::literals::string_literals;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace util::bitset_operators;

template <std::copyable R> class Compute_io;
template <typename Signature>
requires std::is_function_v<Signature>
class Compute_in;
template <std::copyable R, R V> class Compute_constant;
template <std::copyable R> class Compute_value;
template <std::copyable R> class Compute_out;
enum struct Compute_option : std::uint8_t {
  none = util::bitset_value(0U),
  concurrent = util::bitset_value(1U),
  defer = util::bitset_value(2U),
};
using Compute_options = util::bitset_of<Compute_option>;
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Reset_tag {
  consteval Reset_tag() noexcept = default;
  constexpr ~Reset_tag() noexcept = default;
  constexpr Reset_tag(Reset_tag const &) noexcept = default;
  constexpr auto operator=(Reset_tag const &) noexcept -> Reset_tag & = default;
  constexpr Reset_tag(Reset_tag &&) noexcept = default;
  constexpr auto operator=(Reset_tag &&) noexcept -> Reset_tag & = default;
};
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Extract_tag {
  consteval Extract_tag() noexcept = default;
  constexpr ~Extract_tag() noexcept = default;
  constexpr Extract_tag(Extract_tag const &) noexcept = default;
  constexpr auto operator=(Extract_tag const &) noexcept
      -> Extract_tag & = default;
  constexpr Extract_tag(Extract_tag &&) noexcept = default;
  constexpr auto operator=(Extract_tag &&) noexcept -> Extract_tag & = default;
};
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Out_tag {
  consteval Out_tag() noexcept = default;
  constexpr ~Out_tag() noexcept = default;
  constexpr Out_tag(Out_tag const &) noexcept = default;
  constexpr auto operator=(Out_tag const &) noexcept -> Out_tag & = default;
  constexpr Out_tag(Out_tag &&) noexcept = default;
  constexpr auto operator=(Out_tag &&) noexcept -> Out_tag & = default;
};

template <std::copyable R> class Compute_io {
public:
  using return_type = R;
  virtual auto operator()() const -> return_type = 0;

  virtual ~Compute_io() noexcept = default;
  Compute_io(Compute_io<R> const &) = delete;
  auto operator=(Compute_io<R> const &) = delete;
  Compute_io(Compute_io<R> &&) = delete;
  auto operator=(Compute_io<R> &&) = delete;

protected:
  Compute_io() noexcept = default;
};

template <std::copyable R, typename... Args>
class Compute_in<R(Args...)>
    : public Compute_io<R>,
      public std::enable_shared_from_this<Compute_in<R(Args...)>> {
public:
  using return_type = typename Compute_io<R>::return_type;
  using signature_type = return_type(Args...);

private:
  std::function<signature_type> const function_;
  std::unique_ptr<std::mutex> const mutex_;
  mutable std::packaged_task<return_type()> task_;
  std::shared_future<return_type> future_;
  mutable bool invoked_;

protected:
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  // clang-format off
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init): false positive?
  /* clang-format on */ explicit Compute_in(
      std::function<signature_type> function, ForwardArgs &&...args)
      : Compute_in{Compute_option::concurrent | Compute_option::defer, function,
                   std::forward<ForwardArgs>(args)...} {}
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  Compute_in(Compute_options const &options,
             std::function<signature_type> function, ForwardArgs &&...args)
      : function_{std::move(function)},
        mutex_{(options & Compute_option::concurrent).any()
                   ? std::make_unique<std::mutex>()
                   : std::unique_ptr<std::mutex>{}},
        task_{[this, &options,
               ... args = std::forward<ForwardArgs>(args)]() mutable {
          std::packaged_task<return_type()> init{
              [this, ... args = std::forward<ForwardArgs>(args)]() mutable {
                return function_(std::forward<ForwardArgs>(args)...);
              }};
          if ((options & Compute_option::defer).none()) {
            init();
          }
          return init;
        }()},
        future_{task_.get_future()},
        invoked_{(options & Compute_option::defer).none()} {
    constexpr auto valid_options{Compute_option::concurrent |
                                 Compute_option::defer};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
  }

private:
  template <typename... ForwardArgs>
  static auto create_const_0
      [[nodiscard]] (Compute_options const &options, ForwardArgs &&...args) {
    constexpr auto valid_options{~Compute_option::concurrent};
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
    return std::shared_ptr<Compute_in<signature_type> const>{
        new Compute_in{std::forward<ForwardArgs>(args)...}};
  }

public:
  template <typename... ForwardArgs>
  static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::shared_ptr<Compute_in<signature_type>>{
        new Compute_in{std::forward<ForwardArgs>(args)...}};
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
      -> std::optional<return_type> {
    constexpr auto valid_options{util::Enum_bitset{} | Compute_option::defer};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    task_ = [this, ... args = std::forward<ForwardArgs>(args)]() mutable {
      return function_(std::forward<ForwardArgs>(args)...);
    };
    future_ = task_.get_future();
    if ((options & Compute_option::defer).any()) {
      invoked_ = false;
      return {};
    }
    task_();
    invoked_ = true;
    return std::shared_future{future_}.get();
  }
  auto reset(Compute_options const &options) -> std::optional<return_type> {
    constexpr auto valid_options{util::Enum_bitset{} | Compute_option::defer};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    task_.reset();
    future_ = task_.get_future();
    if ((options & Compute_option::defer).any()) {
      invoked_ = false;
      return {};
    }
    task_();
    invoked_ = true;
    return std::shared_future{future_}.get();
  }

  auto operator()() const -> return_type override { return invoke(); }
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  void operator<<(ForwardArgs &&...args) {
    bind(util::Enum_bitset{} | Compute_option::defer,
         std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  auto operator<<=(ForwardArgs &&...args) {
    return bind(util::Enum_bitset{} | Compute_option::none,
                std::forward<ForwardArgs>(args)...)
        .value();
  }
  void operator<<([[maybe_unused]] Reset_tag /*unused*/) {
    reset(util::Enum_bitset{} | Compute_option::defer);
  }
  auto operator<<=([[maybe_unused]] Reset_tag /*unused*/) {
    return reset(util::Enum_bitset{} | Compute_option::none).value();
  }

  ~Compute_in() noexcept override = default;
  Compute_in(Compute_in<R(Args...)> const &) = delete;
  auto operator=(Compute_in<R(Args...)> const &) = delete;
  Compute_in(Compute_in<R(Args...)> &&) = delete;
  auto operator=(Compute_in<R(Args...)> &&) = delete;
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
public:
  using return_type = typename Compute_io<R>::return_type;
  constexpr static auto value_{V};
  template <typename... ForwardArgs>
  constexpr static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::shared_ptr<Compute_constant<return_type, value_>>{
        new Compute_constant{std::forward<ForwardArgs>(args)...}};
  }
  template <typename... ForwardArgs>
  constexpr static auto create_const [[nodiscard]] (ForwardArgs &&...args) {
    return std::shared_ptr<Compute_constant<return_type, value_> const>{
        new Compute_constant{std::forward<ForwardArgs>(args)...}};
  }
  constexpr auto operator() [[nodiscard]] () const -> return_type override {
    return value_;
  }

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
public:
  using return_type = typename Compute_io<R>::return_type;

private:
  std::unique_ptr<std::mutex> const mutex_;
  return_type value_;

protected:
  explicit Compute_value(return_type const &value)
      : Compute_value{util::Enum_bitset{} | Compute_option::concurrent, value} {
  }
  Compute_value(Compute_options const &options, return_type const &value)
      : mutex_{(options & Compute_option::concurrent).any()
                   ? std::make_unique<std::mutex>()
                   : std::unique_ptr<std::mutex>{}},
        value_{value} {
    constexpr auto valid_options{util::Enum_bitset{} |
                                 Compute_option::concurrent};
    util::check_bitset(valid_options,
                       u8"Ignored "s + util::type_name<Compute_option>(),
                       options);
  }

private:
  template <typename... ForwardArgs>
  static auto create_const_0
      [[nodiscard]] (Compute_options const &options, ForwardArgs &&...args) {
    constexpr auto valid_options{~Compute_option::concurrent};
    util::check_bitset(valid_options,
                       u8"Unnecessary "s + util::type_name<Compute_option>(),
                       options);
    return create_const_1(options, std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const_0 [[nodiscard]] (ForwardArgs &&...args) {
    return create_const_1(util::Enum_bitset{} | Compute_option::none,
                          std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  static auto create_const_1 [[nodiscard]] (ForwardArgs &&...args) {
    return std::shared_ptr<Compute_value<return_type> const>{
        new Compute_value{std::forward<ForwardArgs>(args)...}};
  }

public:
  template <typename... ForwardArgs>
  static auto create [[nodiscard]] (ForwardArgs &&...args) {
    return std::shared_ptr<Compute_value<return_type>>{
        new Compute_value{std::forward<ForwardArgs>(args)...}};
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
  auto set(return_type const &value) {
    auto value_copy{value};
    auto const guard{mutex_ ? std::unique_lock{*mutex_}
                            : std::unique_lock<std::mutex>{}};
    using std::swap;
    swap(value_, value_copy);
    return value_copy;
  }

  auto operator() [[nodiscard]] () const -> return_type override {
    return get();
  }
  auto operator<<(return_type const &value) { return set(value); }

  ~Compute_value() noexcept override = default;
  Compute_value(Compute_value<R> const &) = delete;
  auto operator=(Compute_value<R> const &) = delete;
  Compute_value(Compute_value<R> &&) = delete;
  auto operator=(Compute_value<R> &&) = delete;
};

template <std::copyable R> class Compute_out : private Compute_io<R> {
public:
  using return_type = typename Compute_io<R>::return_type;

private:
  std::weak_ptr<Compute_io<return_type> const> in_{};
  return_type return_{};

public:
  constexpr Compute_out() noexcept = default;
  template <std::derived_from<Compute_io<return_type>> In>
  requires std::derived_from<In, std::enable_shared_from_this<In>>
  explicit Compute_out(In const &in)
      : in_{std::static_pointer_cast<Compute_io<return_type> const>(
            static_cast<std::enable_shared_from_this<In> const &>(in)
                .shared_from_this())},
        return_{in()} {}

  auto get [[nodiscard]] () const
      noexcept(noexcept(return_) &&
               std::is_nothrow_move_constructible_v<return_type>) {
    return return_;
  }
  auto extract() {
    if (auto const in{in_.lock()}) {
      return_ = (*in)();
    }
    return return_;
  }

  auto operator() [[nodiscard]] () const
      noexcept(noexcept(get()) &&
               std::is_nothrow_move_constructible_v<return_type>)
          -> return_type override {
    return get();
  }
  auto operator()([[maybe_unused]] Extract_tag /*unused*/) { return extract(); }
  auto operator>>(return_type &right) noexcept(
      noexcept(right = get()) &&
      std::is_nothrow_move_constructible_v<return_type>) {
    return right = get();
  }
  auto operator>>=(return_type &right) { return right = extract(); }

  ~Compute_out() noexcept override = default;
  void swap(Compute_out<return_type> &other) noexcept {
    using std::swap;
    swap(in_, other.in_);
    swap(return_, other.return_);
  }
  Compute_out(Compute_out<R> const &other) noexcept(
      std::is_nothrow_copy_constructible_v<decltype(in_)>
          &&std::is_nothrow_copy_constructible_v<decltype(return_)>)
      : in_{other.in_}, return_{other.return_} {}
  auto operator=(Compute_out<R> const &right) noexcept(noexcept(
      Compute_out{right}.swap(*this)) &&noexcept(*this)) -> Compute_out<R> & {
    Compute_out{right}.swap(*this);
    return *this;
  };
  Compute_out(Compute_out<R> &&other) noexcept
      : in_{std::move(other.in_)}, return_{std::move(other.return_)} {}
  auto operator=(Compute_out<R> &&right) noexcept -> Compute_out<R> & {
    Compute_out{std::move(right)}.swap(*this);
    return *this;
  };
};
template <std::copyable R>
explicit Compute_out(Compute_io<R> const &in) -> Compute_out<R>;
template <std::copyable R>
void swap(Compute_out<R> &left, Compute_out<R> &right) noexcept {
  left.swap(right);
}

auto operator<<([[maybe_unused]] Out_tag /*unused*/, auto const &right) {
  return Compute_out{right};
}
} // namespace artccel::core::compute

#endif
