#ifndef ARTCCEL_CORE_UTIL_UTILITY_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_UTILITY_EXTRAS_HPP
#pragma once

#include "containers_extras.hpp" // import f::to_array_cv
#include "polyfill.hpp"          // import literals::operator""_UZ
#include "semantics.hpp"         // import null_terminator_size
#include <array>                 // import std::array
#include <cassert>               // import assert
#include <concepts> // import std::copy_constructible, std::invocable, std::move_constructible
#include <cstddef>     // import std::size_t
#include <functional>  // import std::invoke
#include <memory>      // import std::addressof
#include <span>        // import std::dynamic_extent, std::span
#include <type_traits> // import std::decay_t, std::invoke_result_t, std::is_nothrow_invocable_v, std::is_nothrow_move_constructible_v, std::is_pointer_v, std::is_reference_v, std::remove_reference_t
#include <utility> // import std::forward, std::index_sequence, std::index_sequence_for, std::move

namespace artccel::core::util {
using literals::operator""_UZ;

template <typename T, bool Explicit = true> struct Delegate;
template <typename CharT, std::size_t N> struct Template_string;

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
};

template <typename CharT, std::size_t N>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Template_string {
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::array<CharT const, N> data_;

  explicit consteval Template_string(std::array<CharT const, N> str)
      : data_{std::move(str)} {}
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions,cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  consteval Template_string(CharT const (&str)[N])
      : Template_string{f::to_array_cv(str)} {
    // implicit for use in string literal operator template
  }
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  explicit consteval Template_string(CharT const (&&str)[N])
      : Template_string{f::to_array_cv(std::move(str))} {}
  explicit consteval Template_string(std::span<CharT const, N> str) requires(
      N != std::dynamic_extent)
      : Template_string{f::to_array_cv(str)} {}
  explicit consteval Template_string(char chr) : Template_string{{{chr}}} {}
  explicit consteval Template_string(wchar_t chr) : Template_string{{{chr}}} {}
  explicit consteval Template_string(char8_t chr) : Template_string{{{chr}}} {}
  explicit consteval Template_string(char16_t chr) : Template_string{{{chr}}} {}
  explicit consteval Template_string(char32_t chr) : Template_string{{{chr}}} {}
};
Template_string(char chr)->Template_string<char, 1_UZ + null_terminator_size>;
Template_string(wchar_t chr)
    ->Template_string<wchar_t, 1_UZ + null_terminator_size>;
Template_string(char8_t chr)
    ->Template_string<char8_t, 1_UZ + null_terminator_size>;
Template_string(char16_t chr)
    ->Template_string<char16_t, 1_UZ + null_terminator_size>;
Template_string(char32_t chr)
    ->Template_string<char32_t, 1_UZ + null_terminator_size>;
} // namespace artccel::core::util

#endif
