#ifndef ARTCCEL_CORE_POLYFILL_HPP
#define ARTCCEL_CORE_POLYFILL_HPP
#pragma once

#include <cstddef>

namespace artccel::core::util {
inline namespace literals {
// NOLINTNEXTLINE(google-runtime-int)
consteval auto operator""_UZ(unsigned long long value) {
  return static_cast<std::size_t>(value);
}
} // namespace literals
} // namespace artccel::core::util

#endif
