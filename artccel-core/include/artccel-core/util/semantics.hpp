#ifndef ARTCCEL_CORE_UTIL_SEMANTICS_HPP
#define ARTCCEL_CORE_UTIL_SEMANTICS_HPP
#pragma once

#include <gsl/gsl>     // import gsl::basic_zstring, gsl::dynamic_extent
#include <type_traits> // import std::is_reference_v

namespace artccel::core::util {
using Cu8zstring = gsl::basic_zstring<char8_t const, gsl::dynamic_extent>;
using U8zstring = gsl::basic_zstring<char8_t, gsl::dynamic_extent>;

template <typename T>
requires std::is_reference_v<T>
using Owner = T;

template <typename T> using Observer_ptr = T *;
template <typename T> using Observer_ref = T &;
} // namespace artccel::core::util

#endif
