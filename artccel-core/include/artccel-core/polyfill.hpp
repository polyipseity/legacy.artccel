#ifndef ARTCCEL_CORE_POLYFILL_HPP
#define ARTCCEL_CORE_POLYFILL_HPP
#pragma once

#include <cstddef> // import std::size_t

namespace artccel::core::util {
inline namespace literals {
// NOLINTNEXTLINE(google-runtime-int): specs requires 'unsigned long long'
consteval auto operator""_UZ(unsigned long long value) noexcept {
  return static_cast<std::size_t>(value);
}
} // namespace literals
} // namespace artccel::core::util

#endif
