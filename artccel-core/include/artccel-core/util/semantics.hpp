#ifndef ARTCCEL_CORE_UTIL_SEMANTICS_HPP
#define ARTCCEL_CORE_UTIL_SEMANTICS_HPP
#pragma once

#include "polyfill.hpp" // import literals::operator""_UZ
#include <gsl/gsl>      // import gsl::basic_zstring, gsl::dynamic_extent

namespace artccel::core::util {
using literals::operator""_UZ;

using Cu8zstring = gsl::basic_zstring<char8_t const, gsl::dynamic_extent>;
using U8zstring = gsl::basic_zstring<char8_t, gsl::dynamic_extent>;

template <typename T> using Observer_ptr = T *;
template <typename T> using Observer_ref = T &;

constexpr inline auto null_terminator_size{1_UZ};
} // namespace artccel::core::util

#endif
