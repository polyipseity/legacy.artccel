#ifndef ARTCCEL_CORE_POLYFILL_HPP
#define ARTCCEL_CORE_POLYFILL_HPP
#pragma once

#include <cstddef>     // import std::size_t
#include <type_traits> // import std::underlying_type_t

namespace artccel::core::util {
constexpr auto to_underlying [[nodiscard]] (auto e) noexcept {
  // TODO: C++23: std::to_underlying
  return static_cast<std::underlying_type_t<decltype(e)>>(e);
}

inline namespace literals {
// NOLINTNEXTLINE(google-runtime-int): specs requires 'unsigned long long'
consteval auto operator""_UZ [[nodiscard]](unsigned long long value) noexcept {
  return static_cast<std::size_t>(value); // TODO: C++23: UZ
}
} // namespace literals
} // namespace artccel::core::util

#endif
