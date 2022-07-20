#ifndef GUARD_7E8E1598_4DA2_472F_908C_B6F29BE0389F
#define GUARD_7E8E1598_4DA2_472F_908C_B6F29BE0389F
#pragma once

#include <concepts>    // import std::derived_from, std::same_as
#include <type_traits> // import std::is_arithmetic_v, std::is_invocable_r_v
#include <utility>     // import std::forward

namespace artccel::core::util {
template <typename Type>
concept Arithmetic = std::is_arithmetic_v<Type>;
template <typename Derived, typename Base>
concept Derived_from_but_not =
    std::derived_from<Derived, Base> && !std::same_as<Derived, Base>;

template <typename Func, typename Ret, typename... ArgTypes>
concept Invocable_r = std::is_invocable_r_v<Ret, Func, ArgTypes...>;
template <typename Func, typename Ret, typename... ArgTypes>
concept Regular_invocable_r = Invocable_r<Func, Ret, ArgTypes...>;

template <typename From, typename To>
concept Brace_convertible_to = requires(From &&from) {
  To{std::forward<From>(from)};
};
template <typename Left, typename Right>
concept Brace_convertible_with =
    Brace_convertible_to<Left, Right> && Brace_convertible_to<Right, Left>;
} // namespace artccel::core::util

#endif
