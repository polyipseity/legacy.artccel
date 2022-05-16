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
requires std::movable<Type>
struct interval {
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval interval(Type const &value) requires std::copyable<Type>
      : interval{Type{value}} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval interval(Type &&value) : interval{std::move(value), nullptr} {}
  constexpr interval(
      Type const &value,
      [[maybe_unused]] std::nullptr_t /*unused*/) requires std::copyable<Type>
      : interval{Type{value}, nullptr} {}
  constexpr interval(Type &&value, [[maybe_unused]] std::nullptr_t /*unused*/)
      : value{value} {
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(
        bound_less_than(LeftBound, Left, this->value) && u8"left >(=) value");
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(
        bound_less_than(RightBound, this->value, Right) &&
        u8"value >(=) right");
  }
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr operator Type() const { return value; }

private:
  Type value;
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
