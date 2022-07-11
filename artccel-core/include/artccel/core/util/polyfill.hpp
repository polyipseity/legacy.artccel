#ifndef ARTCCEL_CORE_UTIL_POLYFILL_HPP
#define ARTCCEL_CORE_UTIL_POLYFILL_HPP
#pragma once

#include <type_traits> // import std::is_enum_v, std::underlying_type_t

#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core::util::f {
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
} // namespace artccel::core::util::f

#endif
