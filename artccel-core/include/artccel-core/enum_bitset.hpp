#ifndef ARTCCEL_CORE_ENUM_BITSET_HPP
#define ARTCCEL_CORE_ENUM_BITSET_HPP
#pragma once

#include "interval.hpp" // import Closed_interval
#include "polyfill.hpp" // import to_underlying
#include <bitset>       // import std::bitset
#include <cinttypes>    // import std::uint8_t, std::uint64_t
#include <climits>      // import CHAR_BIT
#include <type_traits>  // import std::is_enum_v

namespace artccel::core::util {
template <typename E>
requires std::is_enum_v<E>
using bitset_of = std::bitset<CHAR_BIT * sizeof(E)>;

consteval auto bitset_value [[nodiscard]] (
    Closed_interval<std::uint8_t, 0U, CHAR_BIT * sizeof(std::uint64_t)>
        position) noexcept {
  return position == 0U ? std::uint64_t{0}
                        : std::uint64_t{0b1} << (position - 1U);
}

// NOLINTNEXTLINE(altera-struct-pack-align)
struct As_enum_bitset {
  constexpr auto operator<< [[nodiscard]](
      auto right) noexcept requires std::is_enum_v<decltype(right)> {
    return bitset_of<decltype(right)>{to_underlying(right)};
  }

  consteval As_enum_bitset() noexcept = default;
  constexpr ~As_enum_bitset() noexcept = default;
  constexpr As_enum_bitset(As_enum_bitset const &) noexcept = default;
  constexpr auto operator=(As_enum_bitset const &) noexcept
      -> As_enum_bitset & = default;
  constexpr As_enum_bitset(As_enum_bitset &&) noexcept = default;
  constexpr auto operator=(As_enum_bitset &&) noexcept
      -> As_enum_bitset & = default;
};

inline namespace bitset_operators {
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator& [[nodiscard]](E left, E right) noexcept {
  // NOLINTNEXTLINE(google-runtime-int): library argument type
  return bitset_of<E>{static_cast<unsigned long long>(to_underlying(left) &
                                                      to_underlying(right))};
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator| [[nodiscard]](E left, E right) noexcept {
  // NOLINTNEXTLINE(google-runtime-int): library argument type
  return bitset_of<E>{static_cast<unsigned long long>(to_underlying(left) |
                                                      to_underlying(right))};
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator^ [[nodiscard]](E left, E right) noexcept {
  // NOLINTNEXTLINE(google-runtime-int): library argument type
  return bitset_of<E>{static_cast<unsigned long long>(to_underlying(left) ^
                                                      to_underlying(right))};
}
constexpr auto operator~[[nodiscard]](
    auto operand) noexcept requires std::is_enum_v<decltype(operand)> {
  return bitset_of<decltype(operand)>{
      // NOLINTNEXTLINE(google-runtime-int): library argument type
      static_cast<unsigned long long>(~to_underlying(operand))};
}

template <typename E>
requires std::is_enum_v<E>
auto operator& [[nodiscard]](bitset_of<E> const &left, E right) noexcept {
  return left & bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
auto operator& [[nodiscard]](E left, bitset_of<E> const &right) noexcept {
  return bitset_of<E>{to_underlying(left)} & right;
}
template <typename E>
requires std::is_enum_v<E>
auto operator| [[nodiscard]](bitset_of<E> const &left, E right) noexcept {
  return left | bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
auto operator| [[nodiscard]](E left, bitset_of<E> const &right) noexcept {
  return bitset_of<E>{to_underlying(left)} | right;
}
template <typename E>
requires std::is_enum_v<E>
auto operator^ [[nodiscard]](bitset_of<E> const &left, E right) noexcept {
  return left ^ bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
auto operator^ [[nodiscard]](E left, bitset_of<E> const &right) noexcept {
  return bitset_of<E>{to_underlying(left)} ^ right;
}

template <typename E>
requires std::is_enum_v<E>
auto operator&= [[nodiscard]](bitset_of<E> &left, E right) noexcept {
  return left &= bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
auto operator|= [[nodiscard]](bitset_of<E> &left, E right) noexcept {
  return left |= bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
auto operator^= [[nodiscard]](bitset_of<E> &left, E right) noexcept {
  return left ^= bitset_of<E>{to_underlying(right)};
}
} // namespace bitset_operators
} // namespace artccel::core::util

#endif
