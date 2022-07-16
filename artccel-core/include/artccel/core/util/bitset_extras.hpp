#ifndef ARTCCEL_CORE_UTIL_BITSET_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_BITSET_EXTRAS_HPP
#pragma once

#include <bitset>  // import std::bitset
#include <cassert> // immport assert
#include <cstddef> // import std::size_t
#include <utility> // import std::move

#include "enum_bitset.hpp" // import Bitset_of, Enum_bitset

namespace artccel::core::util {
template <typename> struct Bitset_size;
template <typename Type> constexpr auto Bitset_size_v{Bitset_size<Type>::value};
template <std::size_t Size> struct Check_bitset;

template <std::size_t Size> struct Check_bitset {
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::bitset<Size> value_;
  explicit consteval Check_bitset(std::bitset<Size> value) noexcept
      : value_{std::move(value)} {}
  explicit consteval Check_bitset(
      auto value) noexcept requires std::is_enum_v<decltype(value)>
      : value_{Enum_bitset{} | value} {}
  constexpr void operator()(std::bitset<Size> const &value) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert((value_ & value) == value && u8"value has invalid bits set to 1");
  }
};
template <typename Enum>
requires std::is_enum_v<Enum> Check_bitset(Enum value)
->Check_bitset<Bitset_size_v<Bitset_of<Enum>>>;

template <std::size_t Size> struct Bitset_size<std::bitset<Size>> {
  constexpr static auto value{Size};
};
} // namespace artccel::core::util

#endif
