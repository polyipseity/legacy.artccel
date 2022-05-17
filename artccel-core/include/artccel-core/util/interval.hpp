#ifndef ARTCCEL_CORE_UTIL_INTERVAL_HPP
#define ARTCCEL_CORE_UTIL_INTERVAL_HPP
#pragma once

#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace artccel::core {
template <std::totally_ordered Type>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct bound {
public:
  bound(bound<Type> const &) = delete;
  auto operator=(bound<Type> const &) = delete;
  bound(bound<Type> &&) = delete;
  auto operator=(bound<Type> &&) = delete;

protected:
  consteval bound() noexcept = default;
  constexpr ~bound() = default;
};

template <std::totally_ordered Type, Type Value>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct open_bound : bound<Type> {
  consteval open_bound() noexcept = default;
  friend constexpr auto operator==(const open_bound<Type, Value> &left
                                   [[maybe_unused]],
                                   const Type &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(const Type &left [[maybe_unused]],
                                   const open_bound<Type, Value> &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator!=(const open_bound<Type, Value> &left
                                   [[maybe_unused]],
                                   const Type &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator!=(const Type &left [[maybe_unused]],
                                   const open_bound<Type, Value> &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto
  operator<(const open_bound<Type, Value> &left [[maybe_unused]],
            const Type &right) noexcept(noexcept(Value < right)) {
    return Value < right;
  }
  friend constexpr auto
  operator<(const Type &left, const open_bound<Type, Value> &right
            [[maybe_unused]]) noexcept(noexcept(left < Value)) {
    return left < Value;
  }
  friend constexpr auto
  operator>(const open_bound<Type, Value> &left [[maybe_unused]],
            const Type &right) noexcept(noexcept(Value > right)) {
    return Value > right;
  }
  friend constexpr auto
  operator>(const Type &left, const open_bound<Type, Value> &right
            [[maybe_unused]]) noexcept(noexcept(left > Value)) {
    return left > Value;
  }
  friend constexpr auto
  operator<=(const open_bound<Type, Value> &left [[maybe_unused]],
             const Type &right) noexcept(noexcept(Value < right)) {
    return Value < right;
  }
  friend constexpr auto
  operator<=(const Type &left, const open_bound<Type, Value> &right
             [[maybe_unused]]) noexcept(noexcept(left < Value)) {
    return left < Value;
  }
  friend constexpr auto
  operator>=(const open_bound<Type, Value> &left [[maybe_unused]],
             const Type &right) noexcept(noexcept(Value > left)) {
    return Value > right;
  }
  friend constexpr auto
  operator>=(const Type &left, const open_bound<Type, Value> &right
             [[maybe_unused]]) noexcept(noexcept(left > Value)) {
    return left > Value;
  }
  friend constexpr auto operator<=>(
      const open_bound<Type, Value> &left [[maybe_unused]],
      const Type &right) noexcept(noexcept(Value < right) &&noexcept(Value >
                                                                     right)) {
    return Value < right   ? std::partial_ordering::less
           : Value > right ? std::partial_ordering::greater
                           : std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      const Type &left, const open_bound<Type, Value> &right
      [[maybe_unused]]) noexcept(noexcept(left < Value) &&noexcept(left >
                                                                   Value)) {
    return left < Value   ? std::partial_ordering::less
           : left > Value ? std::partial_ordering::greater
                          : std::partial_ordering::unordered;
  }
};

template <std::totally_ordered Type, Type Value>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct closed_bound : bound<Type> {
  consteval closed_bound() noexcept = default;
  friend constexpr auto
  operator==(const closed_bound<Type, Value> &left [[maybe_unused]],
             const Type &right) noexcept(noexcept(Value == right)) {
    return Value == right;
  }
  friend constexpr auto
  operator==(const Type &left, const closed_bound<Type, Value> &right
             [[maybe_unused]]) noexcept(noexcept(left == Value)) {
    return left == Value;
  }
  friend constexpr auto
  operator!=(const closed_bound<Type, Value> &left [[maybe_unused]],
             const Type &right) noexcept(noexcept(Value != right)) {
    return Value != right;
  }
  friend constexpr auto
  operator!=(const Type &left, const closed_bound<Type, Value> &right
             [[maybe_unused]]) noexcept(noexcept(left != Value)) {
    return left != Value;
  }
  friend constexpr auto
  operator<(const closed_bound<Type, Value> &left [[maybe_unused]],
            const Type &right) noexcept(noexcept(Value <= right)) {
    return Value <= right;
  }
  friend constexpr auto
  operator<(const Type &left, const closed_bound<Type, Value> &right
            [[maybe_unused]]) noexcept(noexcept(left <= Value)) {
    return left <= Value;
  }
  friend constexpr auto
  operator>(const closed_bound<Type, Value> &left [[maybe_unused]],
            const Type &right) noexcept(noexcept(Value >= right)) {
    return Value >= right;
  }
  friend constexpr auto
  operator>(const Type &left, const closed_bound<Type, Value> &right
            [[maybe_unused]]) noexcept(noexcept(left >= Value)) {
    return left >= Value;
  }
  friend constexpr auto
  operator<=(const closed_bound<Type, Value> &left [[maybe_unused]],
             const Type &right) noexcept(noexcept(Value <= right)) {
    return Value <= right;
  }
  friend constexpr auto
  operator<=(const Type &left, const closed_bound<Type, Value> &right
             [[maybe_unused]]) noexcept(noexcept(left <= Value)) {
    return left <= Value;
  }
  friend constexpr auto
  operator>=(const closed_bound<Type, Value> &left [[maybe_unused]],
             const Type &right) noexcept(noexcept(Value >= left)) {
    return Value >= right;
  }
  friend constexpr auto
  operator>=(const Type &left, const closed_bound<Type, Value> &right
             [[maybe_unused]]) noexcept(noexcept(left >= Value)) {
    return left >= Value;
  }
  friend constexpr auto operator<=>(
      const closed_bound<Type, Value> &left [[maybe_unused]],
      const Type &right) noexcept(noexcept(Value < right) &&noexcept(Value >
                                                                     right)
                                      &&noexcept(Value == right)) {
    return Value < right    ? std::partial_ordering::less
           : Value > right  ? std::partial_ordering::greater
           : Value == right ? std::partial_ordering::equivalent
                            : std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      const Type &left, const closed_bound<Type, Value> &right
      [[maybe_unused]]) noexcept(noexcept(left < Value) &&noexcept(left > Value)
                                     &&noexcept(left == Value)) {
    return left < Value    ? std::partial_ordering::less
           : left > Value  ? std::partial_ordering::greater
           : left == Value ? std::partial_ordering::equivalent
                           : std::partial_ordering::unordered;
  }
};

// NOLINTNEXTLINE(altera-struct-pack-align)
template <std::totally_ordered Type> struct unbounded : bound<Type> {
  consteval unbounded() noexcept = default;
  friend constexpr auto operator==(const unbounded<Type> &left [[maybe_unused]],
                                   const Type &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(const Type &left [[maybe_unused]],
                                   const unbounded<Type> &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator!=(const unbounded<Type> &left [[maybe_unused]],
                                   const Type &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator!=(const Type &left [[maybe_unused]],
                                   const unbounded<Type> &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(const unbounded<Type> &left [[maybe_unused]],
                                  const Type &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(const Type &left [[maybe_unused]],
                                  const unbounded<Type> &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(const unbounded<Type> &left [[maybe_unused]],
                                  const Type &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(const Type &left [[maybe_unused]],
                                  const unbounded<Type> &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(const unbounded<Type> &left [[maybe_unused]],
                                   const Type &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(const Type &left [[maybe_unused]],
                                   const unbounded<Type> &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(const unbounded<Type> &left [[maybe_unused]],
                                   const Type &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(const Type &left [[maybe_unused]],
                                   const unbounded<Type> &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=>(const unbounded<Type> &left
                                    [[maybe_unused]],
                                    const Type &right
                                    [[maybe_unused]]) noexcept {
    return std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(const Type &left [[maybe_unused]],
                                    const unbounded<Type> &right
                                    [[maybe_unused]]) noexcept {
    return std::partial_ordering::unordered;
  }
};

template <std::totally_ordered Type, typename Left, typename Right>
requires std::is_base_of_v<bound<Type>, Left> &&
    std::is_base_of_v<bound<Type>, Right> &&
    (!std::same_as<Left, bound<Type>>)&&(
        !std::same_as<Right, bound<Type>>)struct interval {
  /*
  usage
  `return (constant expression);`
  checking (debug ONLY)
  - compile-time checking
  */
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval interval(Type const &value) noexcept(noexcept(interval{
      value, nullptr})) requires std::copy_constructible<Type>
      : interval{value, nullptr} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval interval(Type &&value) noexcept(noexcept(interval{
      std::move(value), nullptr})) requires std::move_constructible<Type>
      : interval{std::move(value), nullptr} {}
  /*
  usage
  `return {(expression), nullptr};`
  checking (debug ONLY)
  - compile-time checking requires constexpr/consteval context
  - runtime checking
  */
  constexpr interval(
      Type const &value,
      [[maybe_unused]] std::
          nullptr_t /*unused*/) noexcept(std::
                                             is_nothrow_copy_constructible_v<
                                                 Type>
                                                 &&noexcept(check(
                                                     this->value))) requires
      std::copy_constructible<Type> : value{value} {
    check(this->value);
  }
  constexpr interval(
      Type &&value,
      [[maybe_unused]] std::
          nullptr_t /*unused*/) noexcept(std::
                                             is_nothrow_move_constructible_v<
                                                 Type>
                                                 &&noexcept(check(
                                                     this->value))) requires
      std::move_constructible<Type> : value{std::move(value)} {
    check(this->value);
  }
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr operator Type() const
      noexcept(noexcept(value) && std::is_nothrow_move_constructible_v<Type>) {
    return value;
  }

private:
  Type value;
  static constexpr void check(Type const &value) noexcept(
      noexcept(Left{} < value) &&noexcept(value < Right{})) {
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(Left{} < value && u8"Left >(=) value");
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(value < Right{} && u8"value >(=) Right");
  }
};

template <std::totally_ordered Type>
requires std::constructible_from<Type, decltype(0)>
using nonnegative_interval =
    interval<Type, closed_bound<Type, Type{0}>, unbounded<Type>>;
template <std::totally_ordered Type>
requires std::constructible_from<Type, decltype(0)>
using nonpositive_interval =
    interval<Type, unbounded<Type>, closed_bound<Type, Type{0}>>;
template <std::totally_ordered Type>
requires std::constructible_from<Type, decltype(0)>
using positive_interval =
    interval<Type, open_bound<Type, Type{0}>, unbounded<Type>>;
template <std::totally_ordered Type>
requires std::constructible_from<Type, decltype(0)>
using negative_interval =
    interval<Type, unbounded<Type>, open_bound<Type, Type{0}>>;
} // namespace artccel::core

#endif
