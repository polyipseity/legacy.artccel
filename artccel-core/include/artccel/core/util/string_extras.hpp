#pragma once
#ifndef GUARD_D0C3CB97_52B0_4DA4_8BCE_22BAFB070268
#define GUARD_D0C3CB97_52B0_4DA4_8BCE_22BAFB070268

#include <concepts>    // import std::derived_from, std::same_as
#include <sstream>     // import std::basic_stringbuf, std::basic_stringstream
#include <string>      // import std::basjc_string
#include <type_traits> // import std::remove_cv_t

#include "meta.hpp" // import Replace_all_t

namespace artccel::core::util {
template <typename Type>
concept Char_traits_c = requires {
  // not the actual requirements, but close enough
  // https://cppreference.com/w/cpp/io/basic_stringbuf
  typename std::basic_string<typename std::remove_cv_t<Type>::char_type, Type>;
  typename std::basic_stringstream<typename std::remove_cv_t<Type>::char_type,
                                   Type>;
  typename std::basic_stringbuf<typename std::remove_cv_t<Type>::char_type,
                                Type>;
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
