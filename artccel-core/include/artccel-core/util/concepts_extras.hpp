#ifndef ARTCCEL_CORE_UTIL_CONCEPTS_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CONCEPTS_EXTRAS_HPP
#pragma once

#include <concepts> // import std::derived_from, std::same_as

namespace artccel::core::util {
template <typename Derived, typename Base>
concept Derived_from_but_not =
    std::derived_from<Derived, Base> && !std::same_as<Derived, Base>;
} // namespace artccel::core::util

#endif
