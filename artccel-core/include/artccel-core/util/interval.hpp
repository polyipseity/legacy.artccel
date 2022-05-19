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
template <std::totally_ordered T>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct bound {
  using type = T;
  bound(bound<type> const &) = delete;
  auto operator=(bound<type> const &) = delete;
  bound(bound<type> &&) = delete;
  auto operator=(bound<type> &&) = delete;

protected:
  consteval bound() noexcept = default;
  constexpr ~bound() = default;
};

template <std::totally_ordered T, T V>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct open_bound : bound<T> {
  using type = typename open_bound::type;
  constexpr static auto value{V};
  consteval open_bound() noexcept = default;
  friend constexpr auto operator==(open_bound<type, value> const &left
                                   [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(type const &left [[maybe_unused]],
                                   open_bound<type, value> const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator!=(open_bound<type, value> const &left
                                   [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator!=(type const &left [[maybe_unused]],
                                   open_bound<type, value> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto
  operator<(open_bound<type, value> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value < right)) {
    return value < right;
  }
  friend constexpr auto
  operator<(type const &left, open_bound<type, value> const &right
            [[maybe_unused]]) noexcept(noexcept(left < value)) {
    return left < value;
  }
  friend constexpr auto
  operator>(open_bound<type, value> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value > right)) {
    return value > right;
  }
  friend constexpr auto
  operator>(type const &left, open_bound<type, value> const &right
            [[maybe_unused]]) noexcept(noexcept(left > value)) {
    return left > value;
  }
  friend constexpr auto
  operator<=(open_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value < right)) {
    return value < right;
  }
  friend constexpr auto
  operator<=(type const &left, open_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left < value)) {
    return left < value;
  }
  friend constexpr auto
  operator>=(open_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value > left)) {
    return value > right;
  }
  friend constexpr auto
  operator>=(type const &left, open_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left > value)) {
    return left > value;
  }
  friend constexpr auto operator<=>(
      open_bound<type, value> const &left [[maybe_unused]],
      type const &right) noexcept(noexcept(value < right) &&noexcept(value >
                                                                     right)) {
    return value < right   ? std::partial_ordering::less
           : value > right ? std::partial_ordering::greater
                           : std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      type const &left, open_bound<type, value> const &right
      [[maybe_unused]]) noexcept(noexcept(left < value) &&noexcept(left >
                                                                   value)) {
    return left < value   ? std::partial_ordering::less
           : left > value ? std::partial_ordering::greater
                          : std::partial_ordering::unordered;
  }
};

template <std::totally_ordered T, T V>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct closed_bound : bound<T> {
  using type = typename closed_bound::type;
  constexpr static auto value{V};
  consteval closed_bound() noexcept = default;
  friend constexpr auto
  operator==(closed_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value == right)) {
    return value == right;
  }
  friend constexpr auto
  operator==(type const &left, closed_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left == value)) {
    return left == value;
  }
  friend constexpr auto
  operator!=(closed_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value != right)) {
    return value != right;
  }
  friend constexpr auto
  operator!=(type const &left, closed_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left != value)) {
    return left != value;
  }
  friend constexpr auto
  operator<(closed_bound<type, value> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value <= right)) {
    return value <= right;
  }
  friend constexpr auto
  operator<(type const &left, closed_bound<type, value> const &right
            [[maybe_unused]]) noexcept(noexcept(left <= value)) {
    return left <= value;
  }
  friend constexpr auto
  operator>(closed_bound<type, value> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value >= right)) {
    return value >= right;
  }
  friend constexpr auto
  operator>(type const &left, closed_bound<type, value> const &right
            [[maybe_unused]]) noexcept(noexcept(left >= value)) {
    return left >= value;
  }
  friend constexpr auto
  operator<=(closed_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value <= right)) {
    return value <= right;
  }
  friend constexpr auto
  operator<=(type const &left, closed_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left <= value)) {
    return left <= value;
  }
  friend constexpr auto
  operator>=(closed_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value >= right)) {
    return value >= right;
  }
  friend constexpr auto
  operator>=(type const &left, closed_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left >= value)) {
    return left >= value;
  }
  friend constexpr auto operator<=>(
      closed_bound<type, value> const &left [[maybe_unused]],
      type const &right) noexcept(noexcept(value < right) &&noexcept(value >
                                                                     right)
                                      &&noexcept(value == right)) {
    return value < right    ? std::partial_ordering::less
           : value > right  ? std::partial_ordering::greater
           : value == right ? std::partial_ordering::equivalent
                            : std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      type const &left, closed_bound<type, value> const &right
      [[maybe_unused]]) noexcept(noexcept(left < value) &&noexcept(left > value)
                                     &&noexcept(left == value)) {
    return left < value    ? std::partial_ordering::less
           : left > value  ? std::partial_ordering::greater
           : left == value ? std::partial_ordering::equivalent
                           : std::partial_ordering::unordered;
  }
};

// NOLINTNEXTLINE(altera-struct-pack-align)
template <std::totally_ordered T> struct unbounded : bound<T> {
  using type = typename unbounded::type;
  consteval unbounded() noexcept = default;
  friend constexpr auto operator==(unbounded<type> const &left [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(type const &left [[maybe_unused]],
                                   unbounded<type> const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator!=(unbounded<type> const &left [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator!=(type const &left [[maybe_unused]],
                                   unbounded<type> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(unbounded<type> const &left [[maybe_unused]],
                                  type const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(type const &left [[maybe_unused]],
                                  unbounded<type> const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(unbounded<type> const &left [[maybe_unused]],
                                  type const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(type const &left [[maybe_unused]],
                                  unbounded<type> const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(unbounded<type> const &left [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(type const &left [[maybe_unused]],
                                   unbounded<type> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(unbounded<type> const &left [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(type const &left [[maybe_unused]],
                                   unbounded<type> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=>(unbounded<type> const &left
                                    [[maybe_unused]],
                                    type const &right
                                    [[maybe_unused]]) noexcept {
    return std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(type const &left [[maybe_unused]],
                                    unbounded<type> const &right
                                    [[maybe_unused]]) noexcept {
    return std::partial_ordering::unordered;
  }
};

template <typename L, typename R, std::totally_ordered T = typename L::type>
requires std::is_base_of_v<bound<T>, L> && std::is_base_of_v<bound<T>, R> &&
    (!std::same_as<L, bound<T>>)&&(!std::same_as<R, bound<T>>)struct interval {
  using left = L;
  using right = R;
  using type = T;
  /*
  usage
  `return (constant expression);`
  checking (debug ONLY)
  - compile-time checking, causes (complicated) compile error @ assert
  */
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval interval(type const &value) noexcept(noexcept(interval{
      value, nullptr})) requires std::copy_constructible<type>
      : interval{value, nullptr} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval interval(type &&value) noexcept(noexcept(interval{
      std::move(value), nullptr})) requires std::move_constructible<type>
      : interval{std::move(value), nullptr} {}
  /*
  usage
  `return {(expression), nullptr};`
  checking (debug ONLY)
  - compile-time checking requires constexpr/consteval context, causes
  (complicated) compile error @ assert
  - runtime checking
  */
  constexpr interval(
      type const &value,
      [[maybe_unused]] std::
          nullptr_t /*unused*/) noexcept(std::
                                             is_nothrow_copy_constructible_v<
                                                 type>
                                                 &&noexcept(check(
                                                     this->value))) requires
      std::copy_constructible<type> : value{value} {
    check(this->value);
  }
  constexpr interval(
      type &&value,
      [[maybe_unused]] std::
          nullptr_t /*unused*/) noexcept(std::
                                             is_nothrow_move_constructible_v<
                                                 type>
                                                 &&noexcept(check(
                                                     this->value))) requires
      std::move_constructible<type> : value{std::move(value)} {
    check(this->value);
  }
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr operator type() const
      noexcept(noexcept(value) && std::is_nothrow_move_constructible_v<type>) {
    return value;
  }

private:
  type value;
  static constexpr void check(type const &value) noexcept(
      noexcept(left{} < value) &&noexcept(value < right{})) {
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(left{} < value && u8"left >(=) value");
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(value < right{} && u8"value >(=) right");
  }
};

template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using nonnegative_interval = interval<closed_bound<T, T{0}>, unbounded<T>>;
template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using nonpositive_interval = interval<unbounded<T>, closed_bound<T, T{0}>>;
template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using positive_interval = interval<open_bound<T, T{0}>, unbounded<T>>;
template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using negative_interval = interval<unbounded<T>, open_bound<T, T{0}>>;
} // namespace artccel::core

#endif
