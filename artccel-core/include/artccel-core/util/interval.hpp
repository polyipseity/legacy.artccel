#ifndef ARTCCEL_CORE_UTIL_INTERVAL_HPP
#define ARTCCEL_CORE_UTIL_INTERVAL_HPP
#pragma once

#include <cassert>
#include <cinttypes>
#include <concepts>
#include <cstddef>
#include <utility>

namespace artccel::core {
template <std::totally_ordered Type>
requires std::default_initializable<Type>
constexpr Type unbounded{};

enum class bound : uint8_t {
  open,
  closed,
  unbounded,
};

constexpr auto bound_less_than(bound bound,
                               std::totally_ordered auto const &left,
                               std::totally_ordered auto const &right) requires
    std::totally_ordered_with<decltype(left), decltype(right)> {
  return (bound == bound::open && left < right) ||
         (bound == bound::closed && left <= right) || bound == bound::unbounded;
}

template <std::totally_ordered Type, bound LeftBound, Type Left, Type Right,
          bound RightBound>
struct interval {
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval interval(Type const &value) requires std::copy_constructible<Type>
      : interval{value, nullptr} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval interval(Type &&value) requires std::move_constructible<Type>
      : interval{std::move(value), nullptr} {}
  constexpr interval(Type const &value,
                     [[maybe_unused]] std::nullptr_t /*unused*/) requires
      std::copy_constructible<Type> : value{value} {
    check(this->value);
  }
  constexpr interval(Type &&value,
                     [[maybe_unused]] std::nullptr_t /*unused*/) requires
      std::move_constructible<Type> : value{std::move(value)} {
    check(this->value);
  }
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr operator Type() const { return value; }

private:
  Type value;
  static constexpr void check(Type const &value) {
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(bound_less_than(LeftBound, Left, value) &&
                                 u8"left >(=) value");
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(bound_less_than(RightBound, value, Right) &&
                                 u8"value >(=) right");
  }
};

template <std::totally_ordered Type>
requires std::constructible_from<Type, decltype(0)>
using nonnegative_interval =
    interval<Type, bound::closed, Type{0}, unbounded<Type>, bound::unbounded>;
template <std::totally_ordered Type>
requires std::constructible_from<Type, decltype(0)>
using nonpositive_interval =
    interval<Type, bound::unbounded, unbounded<Type>, Type{0}, bound::closed>;
template <std::totally_ordered Type>
requires std::constructible_from<Type, decltype(0)>
using positive_interval =
    interval<Type, bound::open, Type{0}, unbounded<Type>, bound::unbounded>;
template <std::totally_ordered Type>
requires std::constructible_from<Type, decltype(0)>
using negative_interval =
    interval<Type, bound::unbounded, unbounded<Type>, Type{0}, bound::open>;
} // namespace artccel::core

#endif
