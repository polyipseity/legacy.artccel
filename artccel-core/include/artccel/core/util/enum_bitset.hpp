#ifndef GUARD_AAE270C5_7224_46AA_BB9A_DCAF891BA740
#define GUARD_AAE270C5_7224_46AA_BB9A_DCAF891BA740
#pragma once

#include <bitset>  // import std::bitset
#include <climits> // import CHAR_BIT
#include <cstdint> // import std::uintmax_t

#include "concepts_extras.hpp"   // import Enum
#include "conversions.hpp"       // import f::int_unsigned_cast
#include "polyfill.hpp"          // import f::to_underlying
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core::util {
enum struct Enum_bitset : bool {};

constexpr inline std::uintmax_t empty_bitmask{0};
template <Enum Enum> using Bitset_of = std::bitset<CHAR_BIT * sizeof(Enum)>;

namespace f {
ARTCCEL_CORE_EXPORT consteval auto next_bitmask
    [[nodiscard]] (std::uintmax_t bitmask) {
  return bitmask == empty_bitmask ? 1 : bitmask << 1U;
}
} // namespace f

constexpr auto operator|
    [[nodiscard]] (Enum_bitset tag [[maybe_unused]], Enum auto right) noexcept {
  return Bitset_of<decltype(right)>{f::to_underlying(right)};
}

namespace operators::enum_bitset {
template <Enum Enum>
constexpr auto operator& [[nodiscard]] (Enum left, Enum right) noexcept {
  return Bitset_of<Enum>{
      f::int_unsigned_cast(f::to_underlying(left) & f::to_underlying(right))};
}
template <Enum Enum>
constexpr auto operator| [[nodiscard]] (Enum left, Enum right) noexcept {
  return Bitset_of<Enum>{
      f::int_unsigned_cast(f::to_underlying(left) | f::to_underlying(right))};
}
template <Enum Enum>
constexpr auto operator^ [[nodiscard]] (Enum left, Enum right) noexcept {
  return Bitset_of<Enum>{
      f::int_unsigned_cast(f::to_underlying(left) ^ f::to_underlying(right))};
}
constexpr auto operator~[[nodiscard]] (Enum auto operand) noexcept {
  return Bitset_of<decltype(operand)>{
      f::int_unsigned_cast(~f::to_underlying(operand))};
}

template <Enum Enum>
constexpr auto operator&
    [[nodiscard]] (Bitset_of<Enum> const &left, Enum right) noexcept {
  return left & Bitset_of<Enum>{f::to_underlying(right)};
}
template <Enum Enum>
constexpr auto operator&
    [[nodiscard]] (Enum left, Bitset_of<Enum> const &right) noexcept {
  return Bitset_of<Enum>{f::to_underlying(left)} & right;
}
template <Enum Enum>
constexpr auto operator|
    [[nodiscard]] (Bitset_of<Enum> const &left, Enum right) noexcept {
  return left | Bitset_of<Enum>{f::to_underlying(right)};
}
template <Enum Enum>
constexpr auto operator|
    [[nodiscard]] (Enum left, Bitset_of<Enum> const &right) noexcept {
  return Bitset_of<Enum>{f::to_underlying(left)} | right;
}
template <Enum Enum>
constexpr auto operator^
    [[nodiscard]] (Bitset_of<Enum> const &left, Enum right) noexcept {
  return left ^ Bitset_of<Enum>{f::to_underlying(right)};
}
template <Enum Enum>
constexpr auto operator^
    [[nodiscard]] (Enum left, Bitset_of<Enum> const &right) noexcept {
  return Bitset_of<Enum>{f::to_underlying(left)} ^ right;
}

template <Enum Enum>
constexpr auto operator&=(Bitset_of<Enum> &left, Enum right) noexcept {
  return left &= Bitset_of<Enum>{f::to_underlying(right)};
}
template <Enum Enum>
constexpr auto operator|=(Bitset_of<Enum> &left, Enum right) noexcept {
  return left |= Bitset_of<Enum>{f::to_underlying(right)};
}
template <Enum Enum>
constexpr auto operator^=(Bitset_of<Enum> &left, Enum right) noexcept {
  return left ^= Bitset_of<Enum>{f::to_underlying(right)};
}
} // namespace operators::enum_bitset
} // namespace artccel::core::util

#endif
