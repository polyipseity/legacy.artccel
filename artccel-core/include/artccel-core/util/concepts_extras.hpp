#ifndef ARTCCEL_CORE_UTIL_CONCEPTS_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CONCEPTS_EXTRAS_HPP
#pragma once

#include <concepts>    // import std::derived_from, std::same_as
#include <type_traits> // import std::is_invocable_r_v

namespace artccel::core::util {
template <typename Derived, typename Base>
concept Derived_from_but_not =
    std::derived_from<Derived, Base> && !std::same_as<Derived, Base>;

template <typename Func, typename Ret, typename... ArgTypes>
concept Invocable_r = std::is_invocable_r_v<Ret, Func, ArgTypes...>;
template <typename Func, typename Ret, typename... ArgTypes>
concept Regular_invocable_r = Invocable_r<Func, Ret, ArgTypes...>;
} // namespace artccel::core::util

#endif
