#pragma once
#ifndef GUARD_B4659632_6C53_4BDB_82D4_2A563CFA703E
#define GUARD_B4659632_6C53_4BDB_82D4_2A563CFA703E

#include <bitset>  // import std::bitset
#include <cassert> // immport assert
#include <cstddef> // import std::size_t
#include <utility> // import std::move

#include "concepts_extras.hpp" // import Enum
#include "enum_bitset.hpp"     // import Bitset_of, Enum_bitset
#include "utility_extras.hpp"  // import Initialize_t

namespace artccel::core::util {
template <typename> struct Bitset_size;
template <typename Type> constexpr auto Bitset_size_v{Bitset_size<Type>::value};
template <std::size_t Size> struct Check_bitset;

template <std::size_t Size> struct Check_bitset {
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::bitset<Size> value_;
  explicit consteval Check_bitset(std::bitset<Size> value) noexcept
      : Check_bitset{Initialize_t{}, std::move(value)} {}
  explicit consteval Check_bitset(Enum auto value) noexcept
      : Check_bitset{Initialize_t{}, Enum_bitset{} | value} {}
  constexpr void operator()(std::bitset<Size> const &value
                            [[maybe_unused]]) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert((value_ & value) == value && u8"value has invalid bits set to 1");
  }

protected:
  explicit consteval Check_bitset(Initialize_t tag [[maybe_unused]],
                                  std::bitset<Size> &&value) noexcept
      : value_{std::move(value)} {}
};
template <Enum Enum>
Check_bitset(Enum value) -> Check_bitset<Bitset_size_v<Bitset_of<Enum>>>;

template <std::size_t Size> struct Bitset_size<std::bitset<Size>> {
  constexpr static auto value{Size};
};
} // namespace artccel::core::util

#endif
