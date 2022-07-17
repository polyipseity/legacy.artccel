#ifndef GUARD_54788333_8D66_4378_A8B0_92306495AE04
#define GUARD_54788333_8D66_4378_A8B0_92306495AE04
#pragma once

#include <cstddef> // import std::size_t

#pragma warning(push)
#pragma warning(disable : 4626 4820)
#include <gsl/gsl> // import gsl::basic_zstring, gsl::dynamic_extent
#pragma warning(pop)

namespace artccel::core::util {
constexpr inline std::size_t null_terminator_size{1}; // TODO: C++23: UZ

using Cu8zstring = gsl::basic_zstring<char8_t const, gsl::dynamic_extent>;
using U8zstring = gsl::basic_zstring<char8_t, gsl::dynamic_extent>;

template <typename Type> using Observer_ptr = Type *;
template <typename Type> using Observer_ref = Type &;
} // namespace artccel::core::util

#endif
