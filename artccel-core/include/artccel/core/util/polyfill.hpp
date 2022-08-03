#pragma once
#ifndef GUARD_EBA0DB96_F1BC_4222_95D0_F363D493BBC1
#define GUARD_EBA0DB96_F1BC_4222_95D0_F363D493BBC1

#include <type_traits> // import std::is_enum_v, std::underlying_type_t

#pragma warning(push)
#pragma warning(disable : 4625 4626)
#include <ofats/invocable.h> // import ofats::any_invocable
#pragma warning(pop)

#include "concepts_extras.hpp"   // import Enum
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core::util {
template <typename Signature>
using Move_only_function =
    ofats::any_invocable<Signature>; // TODO: C++23: std::move_only_function

namespace f {
constexpr auto to_underlying [[nodiscard]] (Enum auto value) noexcept {
  // TODO: C++23: std::to_underlying
  return static_cast<std::underlying_type_t<decltype(value)>>(value);
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
} // namespace artccel::core::util

#endif
