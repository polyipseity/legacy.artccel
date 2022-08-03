#pragma once
#ifndef GUARD_678654BB_B008_4FDC_84E6_9F0BC12F324F
#define GUARD_678654BB_B008_4FDC_84E6_9F0BC12F324F

#include <concepts>   // import std::copyable, std::derived_from
#include <cstdint>    // import std::uint_fast8_t
#include <functional> // import std::function, std::invoke
#include <memory> // import std::enable_shared_from_this, std::make_shared, std::make_unique, std::unique_ptr, std::weak_ptr
#include <mutex>  // import std::lock_guard, std::scoped_lock
#include <optional>     // import std::nullopt, std::optional
#include <shared_mutex> // import std::shared_lock, std::shared_mutex
#include <tuple>        // import std::tuple
#include <type_traits>  // import std::remove_cv_t
#include <utility> // import std::exchange, std::forward, std::move, std::swap

#pragma warning(push)
#pragma warning(disable : 4626 4820)
#include <gsl/gsl> // import gsl::owner
#pragma warning(pop)

#include "../util/bitset_extras.hpp" // import util::Check_bitset
#include "../util/clone.hpp" // import util::Cloneable, util::Cloneable_bases, util::Cloneable_impl
#include "../util/concepts_extras.hpp" // import util::Invocable_r
#include "../util/concurrent.hpp" // import util::Nullable_lockable, util::Semiregular_once_flag
#include "../util/enum_bitset.hpp" // import util::Bitset_of, util::Enum_bitset, util::empty_bitmask, util::f::next_bitmask, util::operators::enum_bitset
#include "../util/polyfill.hpp"    // import util::f::unreachable
#include "../util/utility_extras.hpp" // import util::f::forward_apply

namespace artccel::core {
namespace compute {
enum struct Compute_option : std::uint_fast8_t {
  empty = util::empty_bitmask,
  concurrent = util::f::next_bitmask(empty),
  defer = util::f::next_bitmask(concurrent),
};
using Compute_options = util::Bitset_of<Compute_option>;
template <std::copyable Ret> class Compute_io;
template <typename Derived, std::copyable Ret> class Compute_in;
template <std::copyable Ret> class Compute_out;
template <std::copyable Ret, Ret Val> class Compute_constant;
template <std::copyable Ret, auto Func>
requires util::Invocable_r<decltype(Func), Ret>
class Compute_function_constant;
template <std::copyable Ret> class Compute_value;
template <typename Signature> class Compute_function;
enum struct Reset_t : bool {};
enum struct Extract_t : bool {};
enum struct Out_t : bool {};

template <typename Type, typename Ret>
concept Compute_in_c =
    std::derived_from<std::remove_cv_t<Type>,
                      Compute_in<std::remove_cv_t<Type>, Ret>>;
template <typename Type>
concept Compute_in_any_c =
    Compute_in_c<Type, typename std::remove_cv_t<Type>::return_type>;
} // namespace compute

namespace util {
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace compute;

template <typename Derived, std::copyable Ret>
struct Cloneable_bases<Compute_in<Derived, Ret>> {
  using type = std::tuple<>;
};
template <std::copyable Ret, Ret Val>
struct Cloneable_bases<Compute_constant<Ret, Val>> {
  using self_type = Compute_constant<Ret, Val>;
  using type = std::tuple<Compute_in<self_type, Ret>>;
  using impl_type = std::tuple<>;
};
template <std::copyable Ret, auto Func>
requires util::Invocable_r<decltype(Func), Ret>
struct Cloneable_bases<Compute_function_constant<Ret, Func>> {
  using self_type = Compute_function_constant<Ret, Func>;
  using type = std::tuple<Compute_in<self_type, Ret>>;
  using impl_type = std::tuple<>;
};
template <std::copyable Ret> struct Cloneable_bases<Compute_value<Ret>> {
  using self_type = Compute_value<Ret>;
  using type = std::tuple<Compute_in<self_type, Ret>>;
  using impl_type = std::tuple<>;
};
template <std::copyable Ret, std::copyable... TArgs>
struct Cloneable_bases<Compute_function<Ret(TArgs...)>> {
  using self_type = Compute_function<Ret(TArgs...)>;
  using type = std::tuple<Compute_in<self_type, Ret>>;
  using impl_type = std::tuple<>;
};
} // namespace util

namespace compute {
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace util::operators::enum_bitset;

template <std::copyable Ret> class Compute_io {
public:
  using return_type = Ret;
  virtual auto operator()() const -> Ret = 0;

  virtual ~Compute_io() noexcept = default;
  Compute_io(Compute_io const &) = delete;
  auto operator=(Compute_io const &) = delete;
  Compute_io(Compute_io &&) = delete;
  auto operator=(Compute_io &&) = delete;

protected:
  constexpr Compute_io() noexcept = default;
};

template <typename Derived, std::copyable Ret>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Compute_in : public virtual util::Cloneable<Compute_in<Derived, Ret>>,
                   public Compute_io<Ret>,
                   public std::enable_shared_from_this<Derived> {
public:
  using return_type = typename Compute_in::return_type;
  constexpr static util::Check_bitset clone_valid_options{
      Compute_option::concurrent | Compute_option::defer};
  using util::Cloneable<Compute_in>::clone;
  constexpr auto clone [[nodiscard]] (Compute_options const &options) const {
    return std::unique_ptr<Compute_in>{clone_impl_options(options)};
  }

protected:
  using Compute_in::Compute_io::Compute_io;

private:
  constexpr virtual auto clone_impl_options
      [[nodiscard]] (Compute_options const &options) const
      -> gsl::owner<Compute_in *> = 0;
#pragma warning(suppress : 4625 4626 5026 5027)
};

template <std::copyable Ret> class Compute_out : public Compute_io<Ret> {
public:
  using return_type = typename Compute_out::return_type;
  using in_type = Compute_io<Ret>;

private:
  std::weak_ptr<in_type const> c_in_{};
  Ret return_{};

public:
  constexpr Compute_out() noexcept = default;
  explicit Compute_out(Compute_in_c<Ret> auto const &c_in)
      : c_in_{c_in.weak_from_this()}, return_{c_in()} {}

  auto operator() [[nodiscard]] () const noexcept(noexcept(Ret{return_}))
      -> Ret override {
    return return_;
  }
  auto operator()(Extract_t tag [[maybe_unused]]) -> Ret {
    if (auto const c_in{c_in_.lock()}) {
      return_ = *c_in();
    }
    return return_;
  }
  friend auto operator>>(Compute_out const &left,
                         Ret &right) noexcept(noexcept(Ret{right = left()}))
      -> Ret {
    return right = left();
  }
  friend auto operator>>=(Compute_out &left, Ret &right) -> Ret {
    return right = left(Extract_t{});
  }

  ~Compute_out() noexcept override = default;
  constexpr void swap(Compute_out &other) noexcept {
    using std::swap;
    swap(c_in_, other.c_in_);
    swap(return_, other.return_);
  }
  friend constexpr void swap(Compute_out &left, Compute_out &right) noexcept {
    left.swap(right);
  }
  Compute_out(Compute_out const &other) noexcept(
      noexcept(decltype(c_in_){other.c_in_}, decltype(return_){other.return_}))
      : c_in_{other.c_in_}, return_{other.return_} {}
  auto operator=(Compute_out const &right) noexcept(
      noexcept(Compute_out{right}.swap(*this), *this)) -> Compute_out & {
    Compute_out{right}.swap(*this);
    return *this;
  }
  Compute_out(Compute_out &&other) noexcept
      : c_in_{std::move(other.c_in_)}, return_{std::move(other.return_)} {}
  auto operator=(Compute_out &&right) noexcept -> Compute_out & {
    Compute_out{std::move(right)}.swap(*this);
    return *this;
  }
};
template <std::copyable Ret>
Compute_out(Compute_io<Ret> const &) -> Compute_out<Ret>;
auto operator<<(Out_t tag [[maybe_unused]],
                Compute_in_any_c auto const &right) {
  return Compute_out{right};
}

template <std::copyable Ret, Ret Val>
class Compute_constant
    : public virtual util::Cloneable_impl<Compute_constant<Ret, Val>>,
      public Compute_in<Compute_constant<Ret, Val>, Ret> {
  friend util::Cloneable_impl<Compute_constant>;

private:
  enum struct Friend : bool {};

public:
  using return_type = typename Compute_constant::return_type;
  constexpr static auto value_{Val};
  template <typename... Args>
  explicit Compute_constant(
      Friend tag [[maybe_unused]],
      Args &&...args) noexcept(noexcept(Compute_constant(std::
                                                             forward<Args>(
                                                                 args)...)))
      : Compute_constant(std::forward<Args>(args)...) {}

  template <typename... Args>
  static auto create [[nodiscard]] (Args &&...args) {
    return std::make_shared<Compute_constant>(Friend{},
                                              std::forward<Args>(args)...);
  }
  template <typename... Args>
  static auto create_const [[nodiscard]] (Args &&...args) {
    return std::make_shared<Compute_constant const>(
        Friend{}, std::forward<Args>(args)...);
  }
  auto operator() [[nodiscard]] () const noexcept(noexcept(Ret{value_}))
      -> Ret override {
    return value_;
  }

  using Compute_constant::Cloneable_impl::clone;
  auto clone [[nodiscard]] (Compute_options const &options) const {
    return std::unique_ptr<Compute_constant>{clone_impl_options(options)};
  }
  ~Compute_constant() noexcept override = default;
  auto operator=(Compute_constant const &) = delete;
  Compute_constant(Compute_constant &&) = delete;
  auto operator=(Compute_constant &&) = delete;

protected:
  Compute_constant() noexcept = default;
  Compute_constant(Compute_constant const &other [[maybe_unused]]) noexcept {}

private:
  auto clone_impl_options [[nodiscard]] (Compute_options const &options) const
      -> gsl::owner<Compute_constant *> override {
    Compute_constant::clone_valid_options(options);
    return new Compute_constant{*this};
  }
};

template <std::copyable Ret, auto Func>
requires util::Invocable_r<decltype(Func), Ret>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Compute_function_constant
    : public virtual util::Cloneable_impl<Compute_function_constant<Ret, Func>>,
      public Compute_in<Compute_function_constant<Ret, Func>, Ret> {
  friend util::Cloneable_impl<Compute_function_constant>;

private:
  enum struct Friend : bool {};

public:
  using return_type = typename Compute_function_constant::return_type;
  constexpr static auto function_{Func};
  template <typename... Args>
  explicit Compute_function_constant(
      Friend tag [[maybe_unused]],
      Args
          &&...args) noexcept(noexcept(Compute_function_constant(std::
                                                                     forward<
                                                                         Args>(
                                                                         args)...)))
      : Compute_function_constant(std::forward<Args>(args)...) {}

  template <typename... Args>
  static auto create [[nodiscard]] (Args &&...args) {
    return std::make_shared<Compute_function_constant>(
        Friend{}, std::forward<Args>(args)...);
  }
  template <typename... Args>
  static auto create_const [[nodiscard]] (Args &&...args) {
    return std::make_shared<Compute_function_constant const>(
        Friend{}, std::forward<Args>(args)...);
  }
  auto operator() [[nodiscard]] () const
      noexcept(noexcept(Ret{std::invoke(Func)})) -> Ret override {
    return std::invoke(Func);
  }

  using util::Cloneable_impl<Compute_function_constant>::clone;
  auto clone [[nodiscard]] (Compute_options const &options) const {
    return std::unique_ptr<Compute_function_constant>{
        clone_impl_options(options)};
  }
  ~Compute_function_constant() noexcept override = default;
  auto operator=(Compute_function_constant const &) = delete;
  Compute_function_constant(Compute_function_constant &&) = delete;
  auto operator=(Compute_function_constant &&) = delete;

protected:
  Compute_function_constant() noexcept = default;
  Compute_function_constant(Compute_function_constant const &other
                            [[maybe_unused]]) noexcept {}

private:
  auto clone_impl_options [[nodiscard]] (Compute_options const &options) const
      -> gsl::owner<Compute_function_constant *> override {
    Compute_function_constant::clone_valid_options(options);
    return new Compute_function_constant{*this};
  }
#pragma warning(suppress : 4250)
};

template <std::copyable Ret>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Compute_value : public virtual util::Cloneable_impl<Compute_value<Ret>>,
                      public Compute_in<Compute_value<Ret>, Ret> {
  friend util::Cloneable_impl<Compute_value>;

private:
  enum struct Friend : bool {};

public:
  using return_type = typename Compute_value::return_type;
  template <typename... Args>
  explicit Compute_value(Friend tag [[maybe_unused]], Args &&...args) noexcept(
      noexcept(Compute_value(std::forward<Args>(args)...)))
      : Compute_value(std::forward<Args>(args)...) {}

private:
  util::Nullable_lockable</* mutable */ std::shared_mutex> const mutex_;
  Ret value_;

protected:
  explicit Compute_value(Ret value)
      : Compute_value{util::Enum_bitset{} | Compute_option::concurrent,
                      std::move(value)} {}
  explicit Compute_value(Compute_options const &options, Ret value)
      : mutex_{(options & Compute_option::concurrent).any()
                   ? std::make_unique<std::shared_mutex>()
                   : nullptr},
        value_{std::move(value)} {
    constexpr static util::Check_bitset valid_options{
        Compute_option::concurrent};
    valid_options(options);
  }

private:
  template <typename... Args>
  static auto create_const_0
      [[nodiscard]] (Compute_options const &options, Args &&...args) {
    constexpr static util::Check_bitset valid_options{
        ~Compute_option::concurrent};
    valid_options(options);
    return create_const_1(options, std::forward<Args>(args)...);
  }
  template <typename... Args>
  static auto create_const_0 [[nodiscard]] (Args &&...args) {
    return create_const_1(util::Enum_bitset{} | Compute_option::empty,
                          std::forward<Args>(args)...);
  }
  template <typename... Args>
  static auto create_const_1 [[nodiscard]] (Args &&...args) {
    return std::make_shared<Compute_value const>(Friend{},
                                                 std::forward<Args>(args)...);
  }

public:
  template <typename... Args>
  static auto create [[nodiscard]] (Args &&...args) {
    return std::make_shared<Compute_value>(Friend{},
                                           std::forward<Args>(args)...);
  }
  template <typename... Args>
  static auto create_const [[nodiscard]] (Args &&...args) {
    return create_const_0(std::forward<Args>(args)...);
  }

  auto operator() [[nodiscard]] () const -> Ret override {
    std::shared_lock const guard{mutex_};
    return value_;
  }
  friend auto operator<<(Compute_value &left, Ret const &value) -> Ret {
    return left << Ret{value};
  }
  friend auto operator<<(Compute_value &left, Ret &&value) -> Ret {
    std::lock_guard const guard{left.mutex_};
    return std::exchange(left.value_, std::move(value));
  }
  friend auto operator<<=(Compute_value &left, Ret const &value) -> Ret {
    return left <<= Ret{value};
  }
  friend auto operator<<=(Compute_value &left, Ret &&value) -> Ret {
    std::lock_guard const guard{left.mutex_};
    return left.value_ = std::move(value);
  }

  using util::Cloneable_impl<Compute_value>::clone;
  auto clone [[nodiscard]] (Compute_options const &options) const {
    return std::unique_ptr<Compute_value>{clone_impl_options(options)};
  }
  ~Compute_value() noexcept override = default;

protected:
  void swap(Compute_value &other) noexcept {
    std::scoped_lock const guard{mutex_, other.mutex_};
    using std::swap;
    swap(value_, other.value_);
  }
  Compute_value(Compute_value const &other) noexcept(noexcept(Compute_value{
      other,
      other.mutex_.value_ ? std::make_unique<std::shared_mutex>() : nullptr}))
      : Compute_value(other, other.mutex_.value_
                                 ? std::make_unique<std::shared_mutex>()
                                 : nullptr) {}
  auto operator=(Compute_value const &right) noexcept(
      noexcept(Compute_value{right}.swap(*this), *this)) -> Compute_value & {
    Compute_value{right}.swap(*this);
    return *this;
  }
  Compute_value(Compute_value &&other) noexcept
      : mutex_{other.mutex_.value_ ? std::make_unique<std::shared_mutex>()
                                   : nullptr},
        value_{std::move(other.value_)} {}
  auto operator=(Compute_value &&right) noexcept -> Compute_value & {
    Compute_value{std::move(right)}.swap(*this);
    return *this;
  }

  explicit Compute_value(
      Compute_value const &other,
      std::remove_cv_t<decltype(mutex_)>
          &&mutex) noexcept(noexcept(decltype(mutex_){std::move(mutex)}, void(),
                                     decltype(value_){other.value_}))
      : mutex_{std::move(mutex)}, value_{other.value_} {}

private:
  auto clone_impl_options [[nodiscard]] (Compute_options const &options) const
      -> gsl::owner<Compute_value *> override {
    Compute_value::clone_valid_options(options);
    return new Compute_value{*this, (options | Compute_option::concurrent).any()
                                        ? std::make_unique<std::shared_mutex>()
                                        : nullptr};
  }
#pragma warning(suppress : 4250)
};

template <std::copyable Ret, std::copyable... TArgs>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Compute_function<Ret(TArgs...)>
    : public virtual util::Cloneable_impl<Compute_function<Ret(TArgs...)>>,
      public Compute_in<Compute_function<Ret(TArgs...)>, Ret> {
  friend util::Cloneable_impl<Compute_function>;

private:
  enum struct Friend : bool {};

public:
  using return_type = typename Compute_function::return_type;
  using signature_type = Ret(TArgs...);
  template <typename... Args>
  explicit Compute_function(
      Friend tag [[maybe_unused]],
      Args &&...args) noexcept(noexcept(Compute_function(std::
                                                             forward<Args>(
                                                                 args)...)))
      : Compute_function(std::forward<Args>(args)...) {}

protected:
  enum struct Bound_action : bool { compute, reset };

private:
  util::Nullable_lockable</* mutable */ std::shared_mutex> const mutex_;
  std::function<signature_type> function_;
  std::function<std::optional<Ret>(Bound_action)> bound_;

protected:
  template <typename... Args, util::Invocable_r<Ret, Args...> Func>
  explicit Compute_function(Func &&function, Args &&...args)
      : Compute_function(Compute_option::concurrent | Compute_option::defer,
                         std::forward<Func>(function),
                         std::forward<Args>(args)...) {}
  template <typename... Args, util::Invocable_r<Ret, Args...> Func>
  explicit Compute_function(Compute_options const &options, Func &&function,
                            Args &&...args)
      : mutex_{(options & Compute_option::concurrent).any()
                   ? std::make_unique<std::shared_mutex>()
                   : nullptr},
        function_{std::forward<Func>(function)},
        bound_{bind((options & Compute_option::defer).none(), function_,
                    std::forward<Args>(args)...)} {
    constexpr static util::Check_bitset valid_options{
        Compute_option::concurrent | Compute_option::defer};
    valid_options(options);
  }
  template <typename... Args>
  requires util::Invocable_r<decltype(function_), Ret, Args...>
  static auto bind(bool invoke, decltype(function_) const &function,
                   Args &&...args) {
    auto bound{[flag{util::Semiregular_once_flag{}}, ret{std::optional<Ret>{}},
                function, ... args{std::forward<Args>(args)}](
                   Bound_action action) mutable {
      switch (action) {
      case Bound_action::compute:
        flag.call_once([&ret, &function, &args...]() noexcept(noexcept(
                           ret = function(std::forward<Args>(args)...))) {
          ret = function(std::forward<Args>(args)...);
        });
        return ret;
      case Bound_action::reset:
        flag = {};
        return std::optional<Ret>{};
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
      default:
#pragma clang diagnostic pop
        util::f::unreachable();
      }
    }};
    if (invoke) {
      bound(Bound_action::compute);
    }
    return bound;
  }

private:
  template <typename... Args>
  static auto create_const_0
      [[nodiscard]] (Compute_options const &options, Args &&...args) {
    constexpr static util::Check_bitset valid_options{
        ~Compute_option::concurrent};
    valid_options(options);
    return create_const_1(options, std::forward<Args>(args)...);
  }
  template <typename... Args>
  static auto create_const_0 [[nodiscard]] (Args &&...args) {
    return create_const_1(util::Enum_bitset{} | Compute_option::defer,
                          std::forward<Args>(args)...);
  }
  template <typename... Args>
  static auto create_const_1 [[nodiscard]] (Args &&...args) {
    return std::make_shared<Compute_function const>(
        Friend{}, std::forward<Args>(args)...);
  }

public:
  template <typename... Args>
  static auto create [[nodiscard]] (Args &&...args) {
    return std::make_shared<Compute_function>(Friend{},
                                              std::forward<Args>(args)...);
  }
  template <typename... Args>
  static auto create_const [[nodiscard]] (Args &&...args) {
    return create_const_0(std::forward<Args>(args)...);
  }

  template <typename... Args>
  requires util::Invocable_r<decltype(function_), Ret, Args...>
  auto bind(Compute_options const &options, Args &&...args)
      -> std::optional<Ret> {
    constexpr static util::Check_bitset valid_options{Compute_option::defer};
    valid_options(options);
    auto const invoke{(options & Compute_option::defer).none()};
    std::lock_guard const guard{mutex_};
    bound_ = bind(invoke, function_, std::forward<Args>(args)...);
    return invoke ? bound_(Bound_action::compute) : std::nullopt;
  }
  auto reset(Compute_options const &options) -> std::optional<Ret> {
    constexpr static util::Check_bitset valid_options{Compute_option::defer};
    valid_options(options);
    auto const invoke{(options & Compute_option::defer).none()};
    std::lock_guard const guard{mutex_};
    bound_(Bound_action::reset);
    return invoke ? bound_(Bound_action::compute) : std::nullopt;
  }

  auto operator()() const -> Ret override {
    std::shared_lock const guard{mutex_};
    return *bound_(Bound_action::compute);
  }
  template <template <typename...> typename Tuple, typename... Args>
  requires util::Invocable_r<decltype(function_), Ret, Args...>
  friend void operator<<(Compute_function &left, Tuple<Args &&...> &&t_args) {
    util::f::forward_apply(
        [&left](Args &&...args) {
          left.bind(util::Enum_bitset{} | Compute_option::defer,
                    std::forward<Args>(args)...);
        },
        std::forward<Tuple<Args &&...>>(t_args));
  }
  template <template <typename...> typename Tuple, typename... Args>
  requires util::Invocable_r<decltype(function_), Ret, Args...>
  friend auto operator<<=(Compute_function &left, Tuple<Args &&...> &&t_args)
      -> Ret {
    return util::f::forward_apply(
        [&left](Args &&...args) -> decltype(auto) {
          return *left.bind(util::Enum_bitset{} | Compute_option::empty,
                            std::forward<Args>(args)...);
        },
        std::forward<Tuple<Args &&...>>(t_args));
  }
  friend void operator<<(Compute_function &left, Reset_t tag [[maybe_unused]]) {
    left.reset(util::Enum_bitset{} | Compute_option::defer);
  }
  friend auto operator<<=(Compute_function &left, Reset_t tag [[maybe_unused]])
      -> Ret {
    return *left.reset(util::Enum_bitset{} | Compute_option::empty);
  }

  using util::Cloneable_impl<Compute_function>::clone;
  auto clone [[nodiscard]] (Compute_options const &options) const {
    return std::unique_ptr<Compute_function>{clone_impl_options(options)};
  }
  ~Compute_function() noexcept override = default;

protected:
  void swap(Compute_function &other) noexcept {
    std::scoped_lock const guard{mutex_, other.mutex_};
    using std::swap;
    swap(function_, other.function_);
    swap(bound_, other.bound_);
  }
  Compute_function(Compute_function const &other) noexcept(noexcept(
      Compute_function{other, other.mutex_.value_
                                  ? std::make_unique<std::shared_mutex>()
                                  : nullptr}))
      : Compute_function(other, other.mutex_.value_
                                    ? std::make_unique<std::shared_mutex>()
                                    : nullptr) {}
  auto operator=(Compute_function const &right) noexcept(
      noexcept(this == &right, swap(right), *this)) -> Compute_function & {
    Compute_function{right}.swap(*this);
    return *this;
  }
  Compute_function(Compute_function &&other) noexcept
      : mutex_{other.mutex_.value_ ? std::make_unique<std::shared_mutex>()
                                   : nullptr},
        function_{std::move(other.function_)}, bound_{std::move(other.bound_)} {
  }
  auto operator=(Compute_function &&right) noexcept -> Compute_function & {
    Compute_function{std::move(right)}.swap(*this);
    return *this;
  }

  explicit Compute_function(
      Compute_function const &other,
      std::remove_cv_t<decltype(mutex_)>
          &&mutex) noexcept(noexcept(decltype(mutex_){std::move(mutex)}, void(),
                                     decltype(function_){other.function_},
                                     void(), decltype(bound_){other.bound_}))
      : mutex_{std::move(mutex)}, function_{other.function_},
        bound_{other.bound_} {}

private:
  auto clone_impl_options [[nodiscard]] (Compute_options const &options) const
      -> gsl::owner<Compute_function *> override {
    Compute_function::clone_valid_options(options);
    return new Compute_function{*this,
                                (options | Compute_option::concurrent).any()
                                    ? std::make_unique<std::shared_mutex>()
                                    : nullptr};
  }
#pragma warning(suppress : 4250)
};
template <typename Func>
Compute_function(Func &&func, auto &&...) -> Compute_function<
    decltype(decltype(std::function{std::forward<Func>(func)})::operator())>;
template <typename Func>
Compute_function(Compute_options const &, Func &&func, auto &&...)
    -> Compute_function<decltype(decltype(std::function{
        std::forward<Func>(func)})::operator())>;
} // namespace compute
} // namespace artccel::core

#endif
