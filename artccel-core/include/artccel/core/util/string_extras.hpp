#ifndef ARTCCEL_CORE_UTIL_STRING_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_STRING_EXTRAS_HPP
#pragma once

#include <concepts> // import std::derived_from, std::same_as
#include <sstream>  // import std::basic_stringbuf, std::basic_stringstream
#include <string>   // import std::basjc_string

#include "meta.hpp" // import Replace_all_t

namespace artccel::core::util {
template <typename Type>
concept Char_traits_c = requires {
  // not the actual requirements, but close enough
  // https://cppreference.com/w/cpp/io/basic_stringbuf
  typename std::basic_string<typename Type::char_type, Type>;
  typename std::basic_stringstream<typename Type::char_type, Type>;
  typename std::basic_stringbuf<typename Type::char_type, Type>;
};
template <Char_traits_c Type, typename Replace>
using Rebind_char_traits_t =
    Replace_all_t<Type, typename Type::char_type, Replace>;
template <typename Type, typename LikeTraits, typename CharT>
concept Compatible_char_traits =
    Char_traits_c<Type> && Char_traits_c<LikeTraits> &&
    std::same_as<Type, Rebind_char_traits_t<LikeTraits, CharT>>;
} // namespace artccel::core::util

#endif
