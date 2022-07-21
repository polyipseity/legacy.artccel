#ifndef GUARD_7E8E1598_4DA2_472F_908C_B6F29BE0389F
#define GUARD_7E8E1598_4DA2_472F_908C_B6F29BE0389F
#pragma once

#include <concepts>    // import std::derived_from, std::same_as
#include <cstddef>     // import std::size_t
#include <span>        // import std::dynamic_extent
#include <type_traits> // import std::is_arithmetic_v, std::is_enum_v, std::is_invocable_r_v, std::remove_cvref_t
#include <utility>     // import std::forward

namespace artccel::core::util {
namespace detail {
template <typename Left, typename Right>
concept Differ_from_0 = !std::same_as<Left, Right>;
} // namespace detail

template <typename Left, typename Right>
concept Differ_from =
    detail::Differ_from_0<Left, Right> && detail::Differ_from_0<Right, Left>;
template <typename Derived, typename Base>
concept Derived_from_but_not =
    std::derived_from<Derived, Base> && Differ_from<Derived, Base>;
template <typename Derived, typename Base>
concept Not_derived_from = !std::derived_from<Derived, Base>;
template <typename... Args>
concept Nonempty_pack = sizeof...(Args) >= 1;

template <typename From, typename To>
concept Brace_convertible_to = requires(From &&from) {
  To{std::forward<From>(from)};
};
template <typename Left, typename Right>
concept Brace_convertible_with =
    Brace_convertible_to<Left, Right> && Brace_convertible_to<Right, Left>;

template <typename Type>
concept Arithmetic = std::is_arithmetic_v<Type>;
template <typename Type>
concept Enum = std::is_enum_v<Type>;
template <typename Func, typename Ret, typename... ArgTypes>
concept Invocable_r = std::is_invocable_r_v<Ret, Func, ArgTypes...>;
template <typename Func, typename Ret, typename... ArgTypes>
concept Regular_invocable_r = Invocable_r<Func, Ret, ArgTypes...>;

template <typename ForwardType, typename ThisType>
concept Guard_special_constructors =
    Differ_from<std::remove_cvref_t<ForwardType>, ThisType>;

template <std::size_t Extent>
concept Dynamic_extent = Extent == std::dynamic_extent;
template <std::size_t Extent>
concept Nondynamic_extent = Extent != std::dynamic_extent;
} // namespace artccel::core::util

#endif
