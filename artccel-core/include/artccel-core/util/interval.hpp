#ifndef ARTCCEL_CORE_UTIL_INTERVAL_HPP
#define ARTCCEL_CORE_UTIL_INTERVAL_HPP
#pragma once

#include <cassert>  // import assert
#include <compare>  // import std::partial_ordering
#include <concepts> // import std::constructible_from, std::copy_constructible, std::move_constructible, std::same_as, std::totally_ordered
#include <cstddef>  // import std::nullptr_t
#include <type_traits> // import std::is_base_of_v, std::is_nothrow_move_constructible_v
#include <utility> // import std::move

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
  constexpr static auto value_{V};
  consteval Open_bound() noexcept = default;
  consteval Open_bound(Open_bound<T, V> const &) noexcept = default;
  consteval auto operator=(Open_bound<T, V> const &) noexcept
      -> Open_bound<T, V> & = default;
  consteval Open_bound(Open_bound<T, V> &&) noexcept = default;
  consteval auto operator=(Open_bound<T, V> &&) noexcept
      -> Open_bound<T, V> & = default;
  constexpr ~Open_bound() noexcept = default;
  friend constexpr auto operator==(Open_bound<type, value_> const &left
                                   [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(type const &left [[maybe_unused]],
                                   Open_bound<type, value_> const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator!=(Open_bound<type, value_> const &left
                                   [[maybe_unused]],
                                   type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator!=(type const &left [[maybe_unused]],
                                   Open_bound<type, value_> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto
  operator<(Open_bound<type, value_> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value_ < right) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(value_ < right)>) {
    return value_ < right;
  }
  friend constexpr auto
  operator<(type const &left, Open_bound<type, value_> const &right
            [[maybe_unused]]) noexcept(noexcept(left < value_) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(left < value_)>) {
    return left < value_;
  }
  friend constexpr auto
  operator>(Open_bound<type, value_> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value_ > right) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(value_ > right)>) {
    return value_ > right;
  }
  friend constexpr auto
  operator>(type const &left, Open_bound<type, value_> const &right
            [[maybe_unused]]) noexcept(noexcept(left > value_) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(left > value_)>) {
    return left > value_;
  }
  friend constexpr auto
  operator<=(Open_bound<type, value_> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value_ < right) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(value_ < right)>) {
    return value_ < right;
  }
  friend constexpr auto
  operator<=(type const &left, Open_bound<type, value_> const &right
             [[maybe_unused]]) noexcept(noexcept(left < value_) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left < value_)>) {
    return left < value_;
  }
  friend constexpr auto
  operator>=(Open_bound<type, value_> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value_ > left) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(value_ > right)>) {
    return value_ > right;
  }
  friend constexpr auto
  operator>=(type const &left, Open_bound<type, value_> const &right
             [[maybe_unused]]) noexcept(noexcept(left > value_) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left > value_)>) {
    return left > value_;
  }
  friend constexpr auto operator<=>(
      Open_bound<type, value_> const &left [[maybe_unused]],
      type const
          &right) noexcept(noexcept(value_<right ? std::partial_ordering::less
                                                 : value_>
                                            right
                                        ? std::partial_ordering::greater
                                        : std::partial_ordering::unordered) &&
                           std::is_nothrow_move_constructible_v<
                               decltype(value_ < right
                                            ? std::partial_ordering::less
                                        : value_ > right
                                            ? std::partial_ordering::greater
                                            : std::partial_ordering::
                                                  unordered)>) {
    return value_ < right   ? std::partial_ordering::less
           : value_ > right ? std::partial_ordering::greater
                            : std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      type const &left, Open_bound<type, value_> const &right
      [[maybe_unused]]) noexcept(noexcept(left<value_
                                                   ? std::partial_ordering::less
                                                   : left>
                                                  value_
                                              ? std::partial_ordering::greater
                                              : std::partial_ordering::
                                                    unordered) &&
                                 std::is_nothrow_move_constructible_v<
                                     decltype(left < value_
                                                  ? std::partial_ordering::less
                                              : left > value_
                                                  ? std::partial_ordering::
                                                        greater
                                                  : std::partial_ordering::
                                                        unordered)>) {
    return left < value_   ? std::partial_ordering::less
           : left > value_ ? std::partial_ordering::greater
                           : std::partial_ordering::unordered;
  }
};

template <std::totally_ordered T, T V>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Closed_bound : Bound<T> {
  using type = typename Closed_bound::type;
  constexpr static auto value_{V};
  consteval Closed_bound() noexcept = default;
  consteval Closed_bound(Closed_bound<T, V> const &) noexcept = default;
  consteval auto operator=(Closed_bound<T, V> const &) noexcept
      -> Closed_bound<T, V> & = default;
  consteval Closed_bound(Closed_bound<T, V> &&) noexcept = default;
  consteval auto operator=(Closed_bound<T, V> &&) noexcept
      -> Closed_bound<T, V> & = default;
  constexpr ~Closed_bound() noexcept = default;
  friend constexpr auto
  operator==(Closed_bound<type, value_> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value_ == right) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(value_ == right)>) {
    return value_ == right;
  }
  friend constexpr auto
  operator==(type const &left, Closed_bound<type, value_> const &right
             [[maybe_unused]]) noexcept(noexcept(left == value_) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left == value_)>) {
    return left == value_;
  }
  friend constexpr auto
  operator!=(Closed_bound<type, value_> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value_ != right) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(value_ != right)>) {
    return value_ != right;
  }
  friend constexpr auto
  operator!=(type const &left, Closed_bound<type, value_> const &right
             [[maybe_unused]]) noexcept(noexcept(left != value_) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left != value_)>) {
    return left != value_;
  }
  friend constexpr auto
  operator<(Closed_bound<type, value_> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value_ <= right) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(value_ <= right)>) {
    return value_ <= right;
  }
  friend constexpr auto
  operator<(type const &left, Closed_bound<type, value_> const &right
            [[maybe_unused]]) noexcept(noexcept(left <= value_) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(left <= value_)>) {
    return left <= value_;
  }
  friend constexpr auto
  operator>(Closed_bound<type, value_> const &left [[maybe_unused]],
            type const &right) noexcept(noexcept(value_ >= right) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(value_ >= right)>) {
    return value_ >= right;
  }
  friend constexpr auto
  operator>(type const &left, Closed_bound<type, value_> const &right
            [[maybe_unused]]) noexcept(noexcept(left >= value_) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(left >= value_)>) {
    return left >= value_;
  }
  friend constexpr auto
  operator<=(Closed_bound<type, value_> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value_ <= right) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(value_ <= right)>) {
    return value_ <= right;
  }
  friend constexpr auto
  operator<=(type const &left, Closed_bound<type, value_> const &right
             [[maybe_unused]]) noexcept(noexcept(left <= value_) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left <= value_)>) {
    return left <= value_;
  }
  friend constexpr auto
  operator>=(Closed_bound<type, value_> const &left [[maybe_unused]],
             type const &right) noexcept(noexcept(value_ >= right) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(value_ >= right)>) {
    return value_ >= right;
  }
  friend constexpr auto
  operator>=(type const &left, Closed_bound<type, value_> const &right
             [[maybe_unused]]) noexcept(noexcept(left >= value_) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left >= value_)>) {
    return left >= value_;
  }
  friend constexpr auto operator<=>(
      Closed_bound<type, value_> const &left [[maybe_unused]],
      type const
          &right) noexcept(noexcept(value_<right ? std::partial_ordering::less
                                                 : value_>
                                            right
                                        ? std::partial_ordering::greater
                                    : value_ == right
                                        ? std::partial_ordering::equivalent
                                        : std::partial_ordering::unordered) &&
                           std::is_nothrow_move_constructible_v<
                               decltype(value_ < right
                                            ? std::partial_ordering::less
                                        : value_ > right
                                            ? std::partial_ordering::greater
                                        : value_ == right
                                            ? std::partial_ordering::equivalent
                                            : std::partial_ordering::
                                                  unordered)>) {
    return value_ < right    ? std::partial_ordering::less
           : value_ > right  ? std::partial_ordering::greater
           : value_ == right ? std::partial_ordering::equivalent
                             : std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      type const &left, Closed_bound<type, value_> const &right
      [[maybe_unused]]) noexcept(noexcept(left<value_
                                                   ? std::partial_ordering::less
                                                   : left>
                                                  value_
                                              ? std::partial_ordering::greater
                                          : left == value_
                                              ? std::partial_ordering::
                                                    equivalent
                                              : std::partial_ordering::
                                                    unordered) &&
                                 std::is_nothrow_move_constructible_v<
                                     decltype(left < value_
                                                  ? std::partial_ordering::less
                                              : left > value_
                                                  ? std::partial_ordering::
                                                        greater
                                              : left == value_
                                                  ? std::partial_ordering::
                                                        equivalent
                                                  : std::partial_ordering::
                                                        unordered)>) {
    return left < value_    ? std::partial_ordering::less
           : left > value_  ? std::partial_ordering::greater
           : left == value_ ? std::partial_ordering::equivalent
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
  friend constexpr auto operator<=>(
      Unbounded<type> const &left [[maybe_unused]], type const &right
      [[maybe_unused]]) noexcept(noexcept(std::partial_ordering::unordered) &&
                                 std::is_nothrow_constructible_v<
                                     decltype(std::partial_ordering::
                                                  unordered)>) {
    return std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      type const &left [[maybe_unused]], Unbounded<type> const &right
      [[maybe_unused]]) noexcept(noexcept(std::partial_ordering::unordered) &&
                                 std::is_nothrow_constructible_v<
                                     decltype(std::partial_ordering::
                                                  unordered)>) {
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
  type value_; // NOLINT(misc-non-private-member-variables-in-classes)
  /*
  usage
  `return (constant expression);`
  checking (debug ONLY)
  - compile-time checking, causes (complicated) compile error @ assert
  */
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval Interval(type value) noexcept(noexcept(Interval{
      value, nullptr})) requires std::copy_constructible<type>
      : Interval{value, nullptr} {}
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
          nullptr_t /*unused*/) noexcept(noexcept(decltype(value_){
      value}) &&noexcept(check(value_))) requires std::copy_constructible<type>
      : value_{value} {
    check(value_);
  }
  constexpr Interval(
      type &&value,
      [[maybe_unused]] std::
          nullptr_t /*unused*/) noexcept(noexcept(decltype(value_){
      std::move(value)}) &&noexcept(check(value_))) requires
      std::move_constructible<type> : value_{std::move(value)} {
    check(value_);
  }
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  [[nodiscard]] constexpr operator type &() &noexcept(noexcept(value_)) {
    return value_;
  }
  [[nodiscard]] constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator type const &() const &noexcept(noexcept(value_)) {
    return value_;
  }
  [[nodiscard]] constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator type &&() &&noexcept(noexcept(std::move(value_))) {
    return std::move(value_);
  }
  [[nodiscard]] constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator type const &&() const &&noexcept(noexcept(std::move(value_))) {
    return std::move(value_);
  }

private:
  constexpr static void check(type const &value) noexcept(noexcept(
      left{} < value && u8"left >(=) value") &&noexcept(value < right{} &&
                                                        u8"value >(=) right")) {
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(left{} < value && u8"left >(=) value");
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(value < right{} && u8"value >(=) right");
  }
};

// mathematical classifications

template <std::totally_ordered T, T L, T R>
using Open_interval = Interval<Open_bound<T, L>, Open_bound<T, R>>; // (L,R)
template <std::totally_ordered T, T L, T R>
using Closed_interval =
    Interval<Closed_bound<T, L>, Closed_bound<T, R>>; // [L,R]
template <std::totally_ordered T, T L, T R>
using LC_RO_interval = Interval<Closed_bound<T, L>, Open_bound<T, R>>; // [L,R)
template <std::totally_ordered T, T L, T R>
using LO_RC_interval = Interval<Open_bound<T, L>, Closed_bound<T, R>>; // (L,R]
template <std::totally_ordered T, T L>
using LC_RU_interval = Interval<Closed_bound<T, L>, Unbounded<T>>; // [L,+∞)
template <std::totally_ordered T, T L>
using LO_RU_interval = Interval<Open_bound<T, L>, Unbounded<T>>; // (L,+∞)
template <std::totally_ordered T, T R>
using LU_RC_interval = Interval<Unbounded<T>, Closed_bound<T, R>>; // (-∞,R]
template <std::totally_ordered T, T R>
using LU_RO_interval = Interval<Unbounded<T>, Open_bound<T, R>>; // (-∞,R)
template <std::totally_ordered T>
using Unbounded_interval = Interval<Unbounded<T>, Unbounded<T>>; // (-∞,+∞)
template <std::totally_ordered T, T V = T{}>
using Empty_interval = Open_interval<T, V, V>; // (V,V) = {}
template <std::totally_ordered T, T V>
using Degenerate_interval = Closed_interval<T, V, V>; // [V,V] = {V}

// common uses

template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using Nonnegative_interval = LC_RU_interval<T, T{0}>; // [0,+∞)
template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using Nonpositive_interval = LU_RC_interval<T, T{0}>; // (-∞,0]
template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using Positive_interval = LO_RU_interval<T, T{0}>; // (0,+∞)
template <std::totally_ordered T>
requires std::constructible_from<T, decltype(0)>
using Negative_interval = LU_RO_interval<T, T{0}>; // (-∞,0)
} // namespace artccel::core::util

#endif
