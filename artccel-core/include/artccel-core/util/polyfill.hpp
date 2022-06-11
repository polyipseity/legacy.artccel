#ifndef ARTCCEL_CORE_UTIL_POLYFILL_HPP
#define ARTCCEL_CORE_UTIL_POLYFILL_HPP
#pragma once

#include <cstddef>     // import std::size_t
#include <type_traits> // import std::is_enum_v, std::underlying_type_t

namespace artccel::core::util {
constexpr auto to_underlying [[nodiscard]] (
    auto enum_) noexcept requires std::is_enum_v<decltype(enum_)> {
  // TODO: C++23: std::to_underlying
  return static_cast<std::underlying_type_t<decltype(enum_)>>(enum_);
}

namespace literals {
consteval auto operator""_UZ
    // NOLINTNEXTLINE(google-runtime-int): specs requires 'unsigned long long'
    [[nodiscard]] (unsigned long long value) noexcept {
  return std::size_t{value}; // TODO: C++23: UZ
}
} // namespace literals
} // namespace artccel::core::util

#endif
