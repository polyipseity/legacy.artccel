#ifndef ARTCCEL_CORE_UTIL_UTILITY_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_UTILITY_EXTRAS_HPP
#pragma once

#include <array>   // import std::array
#include <cassert> // import assert
#include <concepts> // import std::copy_constructible, std::invocable, std::move_constructible
#include <cstddef>     // import std::size_t
#include <functional>  // import std::invoke
#include <memory>      // import std::addressof
#include <type_traits> // import std::invoke_result_t, std::is_nothrow_invocable_v, std::is_nothrow_move_constructible_v, std::is_pointer_v, std::is_reference_v, std::remove_reference_t
#include <utility> // import std::forward, std::index_sequence, std::index_sequence_for, std::move

namespace artccel::core::util {
template <typename T, bool Explicit = true> struct Delegate;
template <typename... Ts> struct Overloader;

template <typename> constexpr auto dependent_false_v{false};

namespace f {
template <typename T> constexpr auto unify_ref_to_ptr(T &&value) noexcept {
  // (callsite) -> (return)
  if constexpr (std::is_reference_v<T>) {
    // t& -> t*, t*& -> t**
    return std::addressof(value);
  } else {
    // t -> t, t&& -> t, t* -> t*, t*&& -> t*
    return std::forward<T>(value);
  }
}
template <typename T>
constexpr auto unify_ptr_to_ref(T &&value) noexcept -> decltype(auto) {
  // (callsite) -> (return)
  if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) {
    // t* -> t&, t*& -> t&, t*&& -> t&
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(value && u8"value == nullptr");
    return *value; // *v is t&
  } else {
    // t -> t, t& -> t&, t&& -> t
    return T{std::forward<T>(value)};
  }
}

template <typename F, template <typename...> typename Tuple, typename... Args>
requires std::invocable<F, Args...>
constexpr auto forward_apply(F &&func, Tuple<Args...> &&t_args) noexcept(
    std::is_nothrow_invocable_v<F, Args...> &&
        std::is_nothrow_move_constructible_v<std::invoke_result_t<F, Args...>>)
    -> decltype(auto) {
  using TArgs = Tuple<Args...>;
  return
      [&func, &t_args ]<std::size_t... I>(
          [[maybe_unused]] std::index_sequence<
              I...> /*unused*/) noexcept(std::is_nothrow_invocable_v<F,
                                                                     Args...> &&
                                         std::is_nothrow_move_constructible_v<
                                             std::invoke_result_t<F, Args...>>)
          ->decltype(auto) {
    return std::invoke(
        std::forward<F>(func),
        std::forward<Args>(std::get<I>(std::forward<TArgs>(t_args)))...);
  }
  (std::index_sequence_for<Args...>{});
}
} // namespace f

template <typename T, bool Explicit> struct Delegate {
  using type = T;
  // public members to be a structual type
  T value_; // NOLINT(misc-non-private-member-variables-in-classes)

  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  operator auto &() &noexcept(noexcept(value_)) {
    return value_;
  }
  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  operator auto const &() const &noexcept(noexcept(value_)) {
    return value_;
  }
  template <typename = void>
  requires std::move_constructible<T>
  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  operator T() &&noexcept(noexcept(std::move(value_)) &&
                          std::is_nothrow_move_constructible_v<T>) {
    return std::move(value_);
  }
  template <typename = void>
  requires std::copy_constructible<T>
  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  operator T() const &&noexcept(noexcept(value_) &&
                                std::is_nothrow_move_constructible_v<T>) {
    return value_;
  }

protected:
  explicit constexpr Delegate(T value) noexcept(noexcept(decltype(value_){
      std::move(value)}))
      : value_{std::move(value)} {}
#pragma warning(suppress : 4625 4626)
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
template <typename... Ts> struct Overloader : Ts... {
  using Ts::operator()...;
};
template <typename... Ts>
Overloader(Ts &&...args) -> Overloader<std::remove_reference_t<Ts>...>;
} // namespace artccel::core::util

#endif
