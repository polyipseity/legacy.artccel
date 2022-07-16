#ifndef ARTCCEL_CORE_UTIL_ENUM_BITSET_HPP
#define ARTCCEL_CORE_UTIL_ENUM_BITSET_HPP
#pragma once

#include <bitset>      // import std::bitset
#include <climits>     // import CHAR_BIT
#include <cstdint>     // import std::uintmax_t
#include <type_traits> // import std::is_enum_v

#include "conversions.hpp"       // import f::int_unsigned_cast
#include "polyfill.hpp"          // import f::to_underlying
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core::util {
struct ARTCCEL_CORE_EXPORT Enum_bitset;

constexpr inline std::uintmax_t empty_bitmask{0};
template <typename Enum>
requires std::is_enum_v<Enum>
using Bitset_of = std::bitset<CHAR_BIT * sizeof(Enum)>;

namespace f {
ARTCCEL_CORE_EXPORT consteval auto next_bitmask
    [[nodiscard]] (std::uintmax_t bitmask) {
  return bitmask == empty_bitmask ? 1 : bitmask << 1U;
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
