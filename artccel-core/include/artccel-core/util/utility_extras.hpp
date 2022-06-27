#ifndef ARTCCEL_CORE_UTIL_UTILITY_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_UTILITY_EXTRAS_HPP
#pragma once

#include "polyfill.hpp" // import literals::operator""_UZ
#include <array>        // import std::array
#include <cassert>      // import assert
#include <concepts> // import std::copy_constructible, std::invocable, std::move_constructible
#include <cstddef>     // import std::size_t
#include <functional>  // import std::invoke
#include <memory>      // import std::addressof
#include <type_traits> // import std::decay_t, std::invoke_result_t, std::is_nothrow_invocable_v, std::is_nothrow_move_constructible_v, std::is_pointer_v, std::is_reference_v, std::remove_reference_t
#include <utility> // import std::forward, std::index_sequence, std::index_sequence_for, std::make_index_sequence, std::move

namespace artccel::core::util {
using literals::operator""_UZ;

template <typename T, bool Explicit = true> class Delegate;
template <typename CharT, std::size_t N> struct Template_string;

template <typename> constexpr inline auto dependent_false_v{false};

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
    // clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(value && u8"value == nullptr");
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
              I...> /*unused*/) mutable noexcept(std::
                                                     is_nothrow_invocable_v<
                                                         F, Args...> &&
                                                 std::
                                                     is_nothrow_move_constructible_v<
                                                         std::invoke_result_t<
                                                             F, Args...>>)
          ->decltype(auto) {
    return std::invoke(
        std::forward<F>(func),
        std::forward<Args>(std::get<I>(std::forward<TArgs>(t_args)))...);
  }
  (std::index_sequence_for<Args...>{});
}

template <typename T, std::size_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr auto to_array_cv(T (&array)[N]) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  return [&array]<std::size_t... I>(
      [[maybe_unused]] std::index_sequence<I...> /*unused*/) {
    return std::array<T, N>{{array[I]...}};
  }
  (std::make_index_sequence<N>{});
}
template <typename T, std::size_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr auto to_array_cv(T (&&array)[N]) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  return [&array]<std::size_t... I>(
      [[maybe_unused]] std::index_sequence<I...> /*unused*/) {
    return std::array<T, N>{{std::move(array[I])...}};
  }
  (std::make_index_sequence<N>{});
}
} // namespace f

template <typename T, bool Explicit> class Delegate {
public:
  using type = T;
  // public members to be a structual type
  T value_; // NOLINT(misc-non-private-member-variables-in-classes)

  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator auto &() &noexcept(noexcept(value_)) {
    return value_;
  }
  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator auto const &() const &noexcept(noexcept(value_)) {
    return value_;
  }
  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator T() &&noexcept(noexcept(std::move(value_)) &&
                          std::is_nothrow_move_constructible_v<T>) requires
      std::move_constructible<T> {
    return std::move(value_);
  }
  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator T() const &&noexcept(
      noexcept(value_) && std::is_nothrow_move_constructible_v<T>) requires
      std::copy_constructible<T> {
    return value_;
  }

protected:
  explicit constexpr Delegate(T value) noexcept(noexcept(decltype(value_){
      std::move(value)}))
      : value_{std::move(value)} {}
};

template <typename CharT, std::size_t N>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Template_string {
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::array<CharT const, N> data_;

  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions,cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  consteval Template_string(CharT const (&str)[N])
      : data_{f::to_array_cv(str)} {}
  explicit consteval Template_string(CharT chr) : data_{{chr}} {}
};
Template_string(auto chr) -> Template_string<decltype(chr), 1_UZ>;
} // namespace artccel::core::util

#endif
