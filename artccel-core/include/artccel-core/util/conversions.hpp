#ifndef ARTCCEL_CORE_UTIL_CONVERSIONS_HPP
#define ARTCCEL_CORE_UTIL_CONVERSIONS_HPP
#pragma once

#include "concepts_extras.hpp" // import Regular_invocable_r
#include <concepts>            // import std::integral, std::same_as
#include <functional>          // import std::invoke
#include <limits>              // import std::numeric_limits
#include <stdexcept>           // import std::overflow_error
#include <string>              // import std::to_string
#include <type_traits> // import std::is_nothrow_move_constructible_v, std::remove_cv_t

namespace artccel::core::util {
namespace detail {
template <std::integral Int, auto PosOverflowFunc, auto NegOverflowFunc,
          std::integral InInt>
requires Regular_invocable_r<decltype(PosOverflowFunc), std::remove_cv_t<Int>,
                             std::remove_cv_t<InInt>> &&
    Regular_invocable_r<decltype(NegOverflowFunc), std::remove_cv_t<Int>,
                        std::remove_cv_t<InInt>>
constexpr auto int_cast(InInt value) noexcept(
    noexcept(std::remove_cv_t<Int>{std::invoke(PosOverflowFunc, value)},
             std::remove_cv_t<Int>{std::invoke(NegOverflowFunc, value)}) &&
    std::is_nothrow_move_constructible_v<std::remove_cv_t<Int>>) {
  using out_type = std::remove_cv_t<Int>;
  using in_limits = std::numeric_limits<decltype(value)>;
  using out_limits = std::numeric_limits<out_type>;
  if constexpr (in_limits::max() > out_limits::max()) {
#pragma warning(suppress : 4018)
    if (value > out_limits::max()) {
      return out_type{std::invoke(PosOverflowFunc, value)};
    }
  }
  if constexpr (in_limits::is_signed) {
    if constexpr (!out_limits::is_signed) {
      static_assert(out_limits::lowest() == 0,
                    u8"Unsigned integral type with non-zero lowest value???");
      if (value < 0) {
        return out_type{std::invoke(NegOverflowFunc, value)};
      }
    } else if constexpr (in_limits::lowest() < out_limits::lowest()) {
      if (value < out_limits::lowest()) {
        return out_type{std::invoke(NegOverflowFunc, value)};
      }
    }
  }
  return static_cast<out_type>(value);
}
} // namespace detail

namespace f {
template <std::integral Int>
constexpr auto int_modulo_cast(std::integral auto value) noexcept {
  constexpr auto overflow{[](std::integral auto value) noexcept {
    // value % 2^bits, non-implementation defined for signed since C++20
    return static_cast<std::remove_cv_t<Int>>(value);
  }};
  return detail::int_cast<Int, overflow, overflow>(value);
}
template <std::integral Int>
constexpr auto int_exact_cast(std::integral auto value) {
  constexpr auto overflow{[](std::integral auto value [[maybe_unused]]) {
    throw std::overflow_error{std::to_string(value)};
  }};
  return detail::int_cast<Int, overflow, overflow>(value);
}
template <std::integral Int>
constexpr auto int_clamp_cast(std::integral auto value) noexcept {
  using out_limits = std::numeric_limits<std::remove_cv_t<Int>>;
  return detail::int_cast<
      Int,
      [](std::integral auto value [[maybe_unused]]) noexcept {
        return out_limits::max();
      },
      [](std::integral auto value [[maybe_unused]]) noexcept {
        return out_limits::lowest();
      }>(value);
}
} // namespace f
} // namespace artccel::core::util

#endif
