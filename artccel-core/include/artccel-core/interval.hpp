#ifndef ARTCCEL_CORE_INTERVAL_HPP
#define ARTCCEL_CORE_INTERVAL_HPP
#pragma once

#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace artccel::core::util {
template <std::totally_ordered T>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Bound {
  using type = T;
  Bound(Bound<type> const &) = delete;
  auto operator=(Bound<type> const &) = delete;
  Bound(Bound<type> &&) = delete;
  auto operator=(Bound<type> &&) = delete;

protected:
  consteval Bound() noexcept = default;
  constexpr ~Bound() noexcept = default;
};

template <std::totally_ordered T, T V>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Open_bound : Bound<T> {
  using type = typename Open_bound::type;
  constexpr static auto value{V};
  consteval Open_bound() noexcept = default;
  consteval Open_bound(Open_bound<T, V> const &) noexcept = default;
  consteval auto operator=(Open_bound<T, V> const &) noexcept
      -> Open_bound<T, V> & = default;
  consteval Open_bound(Open_bound<T, V> &&) noexcept = default;
  consteval auto operator=(Open_bound<T, V> &&) noexcept
      -> Open_bound<T, V> & = default;
  constexpr ~Open_bound() noexcept = default;
  friend constexpr auto operator==(Open_bound<type, value> const &left
                                   [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(type const &left [[maybe_unused]],
                                   Open_bound<type, value> const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator!=(Open_bound<type, value> const &left
                                   [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator!=(type const &left [[maybe_unused]],
                                   Open_bound<type, value> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto
  operator<(Open_bound<type, value> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value < right)) {
    return value < right;
  }
  friend constexpr auto
  operator<(type const &left, Open_bound<type, value> const &right
            [[maybe_unused]]) noexcept(noexcept(left < value)) {
    return left < value;
  }
  friend constexpr auto
  operator>(Open_bound<type, value> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value > right)) {
    return value > right;
  }
  friend constexpr auto
  operator>(type const &left, Open_bound<type, value> const &right
            [[maybe_unused]]) noexcept(noexcept(left > value)) {
    return left > value;
  }
  friend constexpr auto
  operator<=(Open_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value < right)) {
    return value < right;
  }
  friend constexpr auto
  operator<=(type const &left, Open_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left < value)) {
    return left < value;
  }
  friend constexpr auto
  operator>=(Open_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value > left)) {
    return value > right;
  }
  friend constexpr auto
  operator>=(type const &left, Open_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left > value)) {
    return left > value;
  }
  friend constexpr auto operator<=>(
      Open_bound<type, value> const &left [[maybe_unused]],
      type const &right) noexcept(noexcept(value < right) &&noexcept(value >
                                                                     right)) {
    return value < right   ? std::partial_ordering::less
           : value > right ? std::partial_ordering::greater
                           : std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      type const &left, Open_bound<type, value> const &right
      [[maybe_unused]]) noexcept(noexcept(left < value) &&noexcept(left >
                                                                   value)) {
    return left < value   ? std::partial_ordering::less
           : left > value ? std::partial_ordering::greater
                          : std::partial_ordering::unordered;
  }
};

template <std::totally_ordered T, T V>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Closed_bound : Bound<T> {
  using type = typename Closed_bound::type;
  constexpr static auto value{V};
  consteval Closed_bound() noexcept = default;
  consteval Closed_bound(Closed_bound<T, V> const &) noexcept = default;
  consteval auto operator=(Closed_bound<T, V> const &) noexcept
      -> Closed_bound<T, V> & = default;
  consteval Closed_bound(Closed_bound<T, V> &&) noexcept = default;
  consteval auto operator=(Closed_bound<T, V> &&) noexcept
      -> Closed_bound<T, V> & = default;
  constexpr ~Closed_bound() noexcept = default;
  friend constexpr auto
  operator==(Closed_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value == right)) {
    return value == right;
  }
  friend constexpr auto
  operator==(type const &left, Closed_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left == value)) {
    return left == value;
  }
  friend constexpr auto
  operator!=(Closed_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value != right)) {
    return value != right;
  }
  friend constexpr auto
  operator!=(type const &left, Closed_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left != value)) {
    return left != value;
  }
  friend constexpr auto
  operator<(Closed_bound<type, value> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value <= right)) {
    return value <= right;
  }
  friend constexpr auto
  operator<(type const &left, Closed_bound<type, value> const &right
            [[maybe_unused]]) noexcept(noexcept(left <= value)) {
    return left <= value;
  }
  friend constexpr auto
  operator>(Closed_bound<type, value> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value >= right)) {
    return value >= right;
  }
  friend constexpr auto
  operator>(type const &left, Closed_bound<type, value> const &right
            [[maybe_unused]]) noexcept(noexcept(left >= value)) {
    return left >= value;
  }
  friend constexpr auto
  operator<=(Closed_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value <= right)) {
    return value <= right;
  }
  friend constexpr auto
  operator<=(type const &left, Closed_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left <= value)) {
    return left <= value;
  }
  friend constexpr auto
  operator>=(Closed_bound<type, value> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value >= right)) {
    return value >= right;
  }
  friend constexpr auto
  operator>=(type const &left, Closed_bound<type, value> const &right
             [[maybe_unused]]) noexcept(noexcept(left >= value)) {
    return left >= value;
  }
  friend constexpr auto operator<=>(
      Closed_bound<type, value> const &left [[maybe_unused]],
      type const &right) noexcept(noexcept(value < right) &&noexcept(value >
                                                                     right)
                                      &&noexcept(value == right)) {
    return value < right    ? std::partial_ordering::less
           : value > right  ? std::partial_ordering::greater
           : value == right ? std::partial_ordering::equivalent
                            : std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      type const &left, Closed_bound<type, value> const &right
      [[maybe_unused]]) noexcept(noexcept(left < value) &&noexcept(left > value)
                                     &&noexcept(left == value)) {
    return left < value    ? std::partial_ordering::less
           : left > value  ? std::partial_ordering::greater
           : left == value ? std::partial_ordering::equivalent
                           : std::partial_ordering::unordered;
  }
};

// NOLINTNEXTLINE(altera-struct-pack-align)
template <std::totally_ordered T> struct Unbounded : Bound<T> {
  using type = typename Unbounded::type;
  consteval Unbounded() noexcept = default;
  consteval Unbounded(Unbounded<T> const &) noexcept = default;
  consteval auto operator=(Unbounded<T> const &) noexcept
      -> Unbounded<T> & = default;
  consteval Unbounded(Unbounded<T> &&) noexcept = default;
  consteval auto operator=(Unbounded<T> &&) noexcept
      -> Unbounded<T> & = default;
  constexpr ~Unbounded() noexcept = default;
  friend constexpr auto operator==(Unbounded<type> const &left [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(type const &left [[maybe_unused]],
                                   Unbounded<type> const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator!=(Unbounded<type> const &left [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator!=(type const &left [[maybe_unused]],
                                   Unbounded<type> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(Unbounded<type> const &left [[maybe_unused]],
                                  type const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(type const &left [[maybe_unused]],
                                  Unbounded<type> const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(Unbounded<type> const &left [[maybe_unused]],
                                  type const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(type const &left [[maybe_unused]],
                                  Unbounded<type> const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(Unbounded<type> const &left [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(type const &left [[maybe_unused]],
                                   Unbounded<type> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(Unbounded<type> const &left [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(type const &left [[maybe_unused]],
                                   Unbounded<type> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=>(Unbounded<type> const &left
                                    [[maybe_unused]],
                                    type const &right
                                    [[maybe_unused]]) noexcept {
    return std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(type const &left [[maybe_unused]],
                                    Unbounded<type> const &right
                                    [[maybe_unused]]) noexcept {
    return std::partial_ordering::unordered;
  }
};

template <typename L, typename R, std::totally_ordered T = typename L::type>
requires std::is_base_of_v<Bound<T>, L> && std::is_base_of_v<Bound<T>, R> &&
    (!std::same_as<L, Bound<T>>)&&(!std::same_as<R, Bound<T>>)struct Interval {
  using left = L;
  using right = R;
  using type = T;
  // public members to be a structual type
  type value; // NOLINT(misc-non-private-member-variables-in-classes)
  /*
  usage
  `return (constant expression);`
  checking (debug ONLY)
  - compile-time checking, causes (complicated) compile error @ assert
  */
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval Interval(type const &value) noexcept(noexcept(Interval{
      value, nullptr})) requires std::copy_constructible<type>
      : Interval{value, nullptr} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval Interval(type &&value) noexcept(noexcept(Interval{
      std::move(value), nullptr})) requires std::move_constructible<type>
      : Interval{std::move(value), nullptr} {}
  /*
  usage
  `return {(expression), nullptr};`
  checking (debug ONLY)
  - compile-time checking requires constexpr/consteval context, causes
  (complicated) compile error @ assert
  - runtime checking
  */
  constexpr Interval(
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
  constexpr Interval(
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
  [[nodiscard]] constexpr operator type() const
      noexcept(noexcept(value) && std::is_nothrow_move_constructible_v<type>) {
    return value;
  }
  constexpr Interval(Interval<L, R, T> const &) = default;
  constexpr auto operator=(Interval<L, R, T> const &)
      -> Interval<L, R, T> & = default;
  // NOLINTNEXTLINE(hicpp-noexcept-move, performance-noexcept-move-constructor)
  constexpr Interval(Interval<L, R, T> &&) =
      default; // automatic noexcept depending on 'type'
  // NOLINTNEXTLINE(hicpp-noexcept-move, performance-noexcept-move-constructor)
  constexpr auto operator=(Interval<L, R, T> &&) -> Interval<L, R, T> & =
      default; // automatic noexcept depending on 'type'
  constexpr ~Interval() = default;

private:
  constexpr static void check(type const &value) noexcept(
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
using Nonnegative_interval = Interval<Closed_bound<T, T{0}>, Unbounded<T>>;
template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using Nonpositive_interval = Interval<Unbounded<T>, Closed_bound<T, T{0}>>;
template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using Positive_interval = Interval<Open_bound<T, T{0}>, Unbounded<T>>;
template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using Negative_interval = Interval<Unbounded<T>, Open_bound<T, T{0}>>;
} // namespace artccel::core::util

#endif
