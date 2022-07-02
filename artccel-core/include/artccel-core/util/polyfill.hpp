#ifndef ARTCCEL_CORE_UTIL_POLYFILL_HPP
#define ARTCCEL_CORE_UTIL_POLYFILL_HPP
#pragma once

#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT
#include <cstddef>               // import std::size_t
#include <type_traits> // import std::is_enum_v, std::underlying_type_t

namespace artccel::core::util {
namespace f {
constexpr auto to_underlying [[nodiscard]] (
    auto enum_) noexcept requires std::is_enum_v<decltype(enum_)> {
  // TODO: C++23: std::to_underlying
  return static_cast<std::underlying_type_t<decltype(enum_)>>(enum_);
}
ARTCCEL_CORE_EXPORT inline void unreachable [[noreturn]] () noexcept {
  // TODO: C++23: std::unreachable
#ifdef __GNUC__ // GCC, Clang, ICC
  __builtin_unreachable();
#elif defined _MSC_VER // MSVC // TODO: C++23: #elifdef
  __assume(false);
#endif
  // with [[noreturn]], an empty body also invokes undefined behavior
}
} // namespace f

namespace literals {
ARTCCEL_CORE_EXPORT constexpr auto operator""_UZ
    // NOLINTNEXTLINE(google-runtime-int): specs requires 'unsigned long long'
    [[nodiscard]] (unsigned long long value) noexcept {
  return std::size_t{value}; // TODO: C++23: UZ
}
} // namespace literals
} // namespace artccel::core::util

#endif
