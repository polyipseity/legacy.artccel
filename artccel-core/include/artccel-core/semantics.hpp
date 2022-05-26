#ifndef ARTCCEL_CORE_SEMANTICS_HPP
#define ARTCCEL_CORE_SEMANTICS_HPP
#pragma once

#include <gsl/gsl> // import gsl::basic_zstring, gsl::dynamic_extent

namespace artccel::core::util {
using Cu8zstring = gsl::basic_zstring<char8_t const, gsl::dynamic_extent>;
using U8zstring = gsl::basic_zstring<char8_t, gsl::dynamic_extent>;

template <typename T> using Observer_ptr = T *;
template <typename T> using Observer_ref = T &;
} // namespace artccel::core::util

#endif
