#ifndef ARTCCEL_CORE_UTIL_ENUM_BITSET_HPP
#define ARTCCEL_CORE_UTIL_ENUM_BITSET_HPP
#pragma once

#include "encoding.hpp" // import c8srtombs
#include "polyfill.hpp" // import to_underlying
#include <bitset>       // import std::bitset
#include <cassert>      // import assert
#include <cinttypes>    // import std::uint64_t
#include <climits>      // import CHAR_BIT
#include <cstddef>      // import std::size_t
#include <iostream>     // import std::cerr
#include <string_view>  // import std::u8string_view
#include <type_traits>  // import std::is_enum_v

namespace artccel::core::util {
template <typename E>
requires std::is_enum_v<E>
using Bitset_of = std::bitset<CHAR_BIT * sizeof(E)>;

constexpr inline std::uint64_t empty_bitmask{0};
consteval auto next_bitmask [[nodiscard]] (std::uint64_t bitmask) {
  return bitmask == empty_bitmask ? std::uint64_t{1} : bitmask << 1U;
}
template <std::size_t N>
constexpr void check_bitset(std::bitset<N> const &valid,
                            std::u8string_view msg_prefix,
                            std::bitset<N> const &value) noexcept {
#ifndef NDEBUG
  if (auto const valid_value{valid & value};
      // clang-format off
      // NOLINTNEXTLINE(google-readability-braces-around-statements, hicpp-braces-around-statements, readability-braces-around-statements)
      /* clang-format on */ valid_value != value) [[unlikely]] {
    std::cerr << c8srtombs(msg_prefix) << c8srtombs(u8": ")
              << (value ^ valid_value) << c8srtombs(u8'\n');
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(false);
  }
#endif
}

// NOLINTNEXTLINE(altera-struct-pack-align)
struct Enum_bitset {
  consteval Enum_bitset() noexcept = default;
  constexpr auto operator| [[nodiscard]] (
      auto right) noexcept requires std::is_enum_v<decltype(right)> {
    return Bitset_of<decltype(right)>{to_underlying(right)};
  }
};

namespace enum_bitset_operators {
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator& [[nodiscard]] (E left, E right) noexcept {
  // NOLINTNEXTLINE(google-runtime-int): library argument type
  return Bitset_of<E>{static_cast<unsigned long long>(to_underlying(left) &
                                                      to_underlying(right))};
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator| [[nodiscard]] (E left, E right) noexcept {
  // NOLINTNEXTLINE(google-runtime-int): library argument type
  return Bitset_of<E>{static_cast<unsigned long long>(to_underlying(left) |
                                                      to_underlying(right))};
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator^ [[nodiscard]] (E left, E right) noexcept {
  // NOLINTNEXTLINE(google-runtime-int): library argument type
  return Bitset_of<E>{static_cast<unsigned long long>(to_underlying(left) ^
                                                      to_underlying(right))};
}
constexpr auto operator~[[nodiscard]] (
    auto operand) noexcept requires std::is_enum_v<decltype(operand)> {
  return Bitset_of<decltype(operand)>{
      // NOLINTNEXTLINE(google-runtime-int): library argument type
      static_cast<unsigned long long>(~to_underlying(operand))};
}

template <typename E>
requires std::is_enum_v<E>
constexpr auto operator&
    [[nodiscard]] (Bitset_of<E> const &left, E right) noexcept {
  return left & Bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator&
    [[nodiscard]] (E left, Bitset_of<E> const &right) noexcept {
  return Bitset_of<E>{to_underlying(left)} & right;
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator|
    [[nodiscard]] (Bitset_of<E> const &left, E right) noexcept {
  return left | Bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator|
    [[nodiscard]] (E left, Bitset_of<E> const &right) noexcept {
  return Bitset_of<E>{to_underlying(left)} | right;
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator^
    [[nodiscard]] (Bitset_of<E> const &left, E right) noexcept {
  return left ^ Bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator^
    [[nodiscard]] (E left, Bitset_of<E> const &right) noexcept {
  return Bitset_of<E>{to_underlying(left)} ^ right;
}

template <typename E>
requires std::is_enum_v<E>
constexpr auto operator&=(Bitset_of<E> &left, E right) noexcept {
  return left &= Bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator|=(Bitset_of<E> &left, E right) noexcept {
  return left |= Bitset_of<E>{to_underlying(right)};
}
template <typename E>
requires std::is_enum_v<E>
constexpr auto operator^=(Bitset_of<E> &left, E right) noexcept {
  return left ^= Bitset_of<E>{to_underlying(right)};
}
} // namespace enum_bitset_operators
} // namespace artccel::core::util

#endif
