#ifndef ARTCCEL_CORE_UTIL_ENUM_BITSET_HPP
#define ARTCCEL_CORE_UTIL_ENUM_BITSET_HPP
#pragma once

#include <bitset>      // import std::bitset
#include <cassert>     // import assert
#include <climits>     // import CHAR_BIT
#include <cstddef>     // import std::size_t
#include <cstdint>     // import std::uintmax_t
#include <iostream>    // import std::cerr
#include <string_view> // import std::u8string_view
#include <type_traits> // import std::is_enum_v

#include "conversions.hpp" // import f::int_unsigned_cast
#include "encoding.hpp" // import literals::encoding::operator""_as_utf8_compat, operators::utf8_compat::ostream::operator<<
#include "polyfill.hpp" // import f::to_underlying
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core::util {
struct ARTCCEL_CORE_EXPORT Enum_bitset;

constexpr inline std::uintmax_t empty_bitmask{0};
template <typename Enum>
requires std::is_enum_v<Enum>
using Bitset_of = std::bitset<CHAR_BIT * sizeof(Enum)>;

namespace f {
using literals::encoding::operator""_as_utf8_compat;
using operators::utf8_compat::ostream::operator<<;

ARTCCEL_CORE_EXPORT consteval auto next_bitmask
    [[nodiscard]] (std::uintmax_t bitmask) {
  return bitmask == empty_bitmask ? 1 : bitmask << 1U;
}
template <std::size_t Size>
constexpr void check_bitset(std::bitset<Size> const &valid [[maybe_unused]],
                            std::u8string_view msg_prefix [[maybe_unused]],
                            std::bitset<Size> const &value [[maybe_unused]]) {
#ifndef NDEBUG
  // NOLINTNEXTLINE(google-readability-braces-around-statements,hicpp-braces-around-statements,readability-braces-around-statements)
  if (auto const valid_value{valid & value}; valid_value != value)
      [[unlikely]] {
    std::cerr << msg_prefix << u8": "_as_utf8_compat << (value ^ valid_value)
              << u8'\n'_as_utf8_compat;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(false && u8"value has invalid bits set to 1");
  }
#endif
}
} // namespace f

struct Enum_bitset {
  explicit consteval Enum_bitset() noexcept = default;
  friend constexpr auto operator| [[nodiscard]] (
      Enum_bitset const &left [[maybe_unused]],
      auto right) noexcept requires std::is_enum_v<decltype(right)> {
    return Bitset_of<decltype(right)>{f::to_underlying(right)};
  }
};

namespace operators::enum_bitset {
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator& [[nodiscard]] (Enum left, Enum right) noexcept {
  return Bitset_of<Enum>{
      f::int_unsigned_cast(f::to_underlying(left) & f::to_underlying(right))};
}
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator| [[nodiscard]] (Enum left, Enum right) noexcept {
  return Bitset_of<Enum>{
      f::int_unsigned_cast(f::to_underlying(left) | f::to_underlying(right))};
}
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator^ [[nodiscard]] (Enum left, Enum right) noexcept {
  return Bitset_of<Enum>{
      f::int_unsigned_cast(f::to_underlying(left) ^ f::to_underlying(right))};
}
constexpr auto operator~[[nodiscard]] (
    auto operand) noexcept requires std::is_enum_v<decltype(operand)> {
  return Bitset_of<decltype(operand)>{
      f::int_unsigned_cast(~f::to_underlying(operand))};
}

template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator&
    [[nodiscard]] (Bitset_of<Enum> const &left, Enum right) noexcept {
  return left & Bitset_of<Enum>{f::to_underlying(right)};
}
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator&
    [[nodiscard]] (Enum left, Bitset_of<Enum> const &right) noexcept {
  return Bitset_of<Enum>{f::to_underlying(left)} & right;
}
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator|
    [[nodiscard]] (Bitset_of<Enum> const &left, Enum right) noexcept {
  return left | Bitset_of<Enum>{f::to_underlying(right)};
}
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator|
    [[nodiscard]] (Enum left, Bitset_of<Enum> const &right) noexcept {
  return Bitset_of<Enum>{f::to_underlying(left)} | right;
}
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator^
    [[nodiscard]] (Bitset_of<Enum> const &left, Enum right) noexcept {
  return left ^ Bitset_of<Enum>{f::to_underlying(right)};
}
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator^
    [[nodiscard]] (Enum left, Bitset_of<Enum> const &right) noexcept {
  return Bitset_of<Enum>{f::to_underlying(left)} ^ right;
}

template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator&=(Bitset_of<Enum> &left, Enum right) noexcept {
  return left &= Bitset_of<Enum>{f::to_underlying(right)};
}
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator|=(Bitset_of<Enum> &left, Enum right) noexcept {
  return left |= Bitset_of<Enum>{f::to_underlying(right)};
}
template <typename Enum>
requires std::is_enum_v<Enum>
constexpr auto operator^=(Bitset_of<Enum> &left, Enum right) noexcept {
  return left ^= Bitset_of<Enum>{f::to_underlying(right)};
}
} // namespace operators::enum_bitset
} // namespace artccel::core::util

#endif
