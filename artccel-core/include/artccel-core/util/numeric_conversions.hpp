#ifndef ARTCCEL_CORE_UTIL_NUMERIC_CONVERSIONS_HPP
#define ARTCCEL_CORE_UTIL_NUMERIC_CONVERSIONS_HPP
#pragma once

#include <concepts>    // import std::integral
#include <limits>      // import std::numeric_limits
#include <type_traits> // import std::remove_cv_t

namespace artccel::core::util::f {
template <std::integral T>
constexpr auto int_modulo_cast(std::integral auto value) noexcept {
  // value % 2^bits, non-implementation defined for signed since C++20
  return static_cast<std::remove_cv_t<T>>(value);
}
template <std::integral T>
constexpr auto int_clamp_cast(std::integral auto value) noexcept {
  using out_type = std::remove_cv_t<T>;
  using in_limits = std::numeric_limits<decltype(value)>;
  using out_limits = std::numeric_limits<out_type>;
  if constexpr (in_limits::max() > out_limits::max()) {
    if (value > out_limits::max()) {
      return out_limits::max();
    }
  }
  if constexpr (in_limits::is_signed) {
    if constexpr (!out_limits::is_signed) {
      static_assert(out_limits::lowest() == 0,
                    u8"Unsigned integral type with non-zero lowest value???");
      if (value < 0) {
        return out_limits::lowest();
      }
    } else if constexpr (in_limits::lowest() < out_limits::lowest()) {
      if (value < out_limits::lowest()) {
        return out_limits::lowest();
      }
    }
  }
  return static_cast<out_type>(value);
}
} // namespace artccel::core::util::f

#endif
