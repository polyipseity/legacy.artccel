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
  constexpr Bound() noexcept = default;
  constexpr Bound(Bound<T> const &) noexcept = default;
  constexpr auto operator=(Bound<T> const &) noexcept -> Bound<T> & = default;
  constexpr Bound(Bound<T> &&) noexcept = default;
  constexpr auto operator=(Bound<T> &&) noexcept -> Bound<T> & = default;

protected:
  constexpr ~Bound() noexcept = default;
};

template <std::totally_ordered T, T V>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Open_bound : private Bound<T> {
  using type = typename Open_bound::type;
  constexpr static auto value_{V};
  consteval Open_bound() noexcept = default;
  friend constexpr auto operator==(Open_bound<T, V> const &left
                                   [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(T const &left [[maybe_unused]],
                                   Open_bound<T, V> const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator!=(Open_bound<T, V> const &left
                                   [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator!=(T const &left [[maybe_unused]],
                                   Open_bound<T, V> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto
  operator<(Open_bound<T, V> const &left [[maybe_unused]],
            T const &right) noexcept(noexcept(V < right) &&
                                     std::is_nothrow_move_constructible_v<
                                         decltype(V < right)>) {
    return V < right;
  }
  friend constexpr auto
  operator<(T const &left, Open_bound<T, V> const &right
            [[maybe_unused]]) noexcept(noexcept(left < V) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(left < V)>) {
    return left < V;
  }
  friend constexpr auto
  operator>(Open_bound<T, V> const &left [[maybe_unused]],
            T const &right) noexcept(noexcept(V > right) &&
                                     std::is_nothrow_move_constructible_v<
                                         decltype(V > right)>) {
    return V > right;
  }
  friend constexpr auto
  operator>(T const &left, Open_bound<T, V> const &right
            [[maybe_unused]]) noexcept(noexcept(left > V) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(left > V)>) {
    return left > V;
  }
  friend constexpr auto
  operator<=(Open_bound<T, V> const &left [[maybe_unused]],
             T const &right) noexcept(noexcept(V < right) &&
                                      std::is_nothrow_move_constructible_v<
                                          decltype(V < right)>) {
    return V < right;
  }
  friend constexpr auto
  operator<=(T const &left, Open_bound<T, V> const &right
             [[maybe_unused]]) noexcept(noexcept(left < V) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left < V)>) {
    return left < V;
  }
  friend constexpr auto
  operator>=(Open_bound<T, V> const &left [[maybe_unused]],
             T const &right) noexcept(noexcept(V > left) &&
                                      std::is_nothrow_move_constructible_v<
                                          decltype(V > right)>) {
    return V > right;
  }
  friend constexpr auto
  operator>=(T const &left, Open_bound<T, V> const &right
             [[maybe_unused]]) noexcept(noexcept(left > V) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left > V)>) {
    return left > V;
  }
  friend constexpr auto
  operator<=>(Open_bound<T, V> const &left [[maybe_unused]],
              T const &right) noexcept(noexcept(compare(V, right)) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(compare(V, right))>) {
    return compare(V, right);
  }
  friend constexpr auto
  operator<=>(T const &left, Open_bound<T, V> const &right
              [[maybe_unused]]) noexcept(noexcept(compare(left, V)) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(compare(left, V))>) {
    return compare(left, V);
  }

private:
  constexpr static auto compare(T const &left, T const &right) noexcept(
      noexcept(left<right ? std::partial_ordering::less : left> right
                   ? std::partial_ordering::greater
                   : std::partial_ordering::unordered) &&
      std::is_nothrow_move_constructible_v<std::partial_ordering>)
      -> std::partial_ordering {
    return left < right   ? std::partial_ordering::less
           : left > right ? std::partial_ordering::greater
                          : std::partial_ordering::unordered;
  }
};

template <std::totally_ordered T, T V>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Closed_bound : private Bound<T> {
  using type = typename Closed_bound::type;
  constexpr static auto value_{V};
  consteval Closed_bound() noexcept = default;
  friend constexpr auto
  operator==(Closed_bound<T, V> const &left [[maybe_unused]],
             T const &right) noexcept(noexcept(V == right) &&
                                      std::is_nothrow_move_constructible_v<
                                          decltype(V == right)>) {
    return V == right;
  }
  friend constexpr auto
  operator==(T const &left, Closed_bound<T, V> const &right
             [[maybe_unused]]) noexcept(noexcept(left == V) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left == V)>) {
    return left == V;
  }
  friend constexpr auto
  operator!=(Closed_bound<T, V> const &left [[maybe_unused]],
             T const &right) noexcept(noexcept(V != right) &&
                                      std::is_nothrow_move_constructible_v<
                                          decltype(V != right)>) {
    return V != right;
  }
  friend constexpr auto
  operator!=(T const &left, Closed_bound<T, V> const &right
             [[maybe_unused]]) noexcept(noexcept(left != V) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left != V)>) {
    return left != V;
  }
  friend constexpr auto
  operator<(Closed_bound<T, V> const &left [[maybe_unused]],
            T const &right) noexcept(noexcept(V <= right) &&
                                     std::is_nothrow_move_constructible_v<
                                         decltype(V <= right)>) {
    return V <= right;
  }
  friend constexpr auto
  operator<(T const &left, Closed_bound<T, V> const &right
            [[maybe_unused]]) noexcept(noexcept(left <= V) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(left <= V)>) {
    return left <= V;
  }
  friend constexpr auto
  operator>(Closed_bound<T, V> const &left [[maybe_unused]],
            T const &right) noexcept(noexcept(V >= right) &&
                                     std::is_nothrow_move_constructible_v<
                                         decltype(V >= right)>) {
    return V >= right;
  }
  friend constexpr auto
  operator>(T const &left, Closed_bound<T, V> const &right
            [[maybe_unused]]) noexcept(noexcept(left >= V) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(left >= V)>) {
    return left >= V;
  }
  friend constexpr auto
  operator<=(Closed_bound<T, V> const &left [[maybe_unused]],
             T const &right) noexcept(noexcept(V <= right) &&
                                      std::is_nothrow_move_constructible_v<
                                          decltype(V <= right)>) {
    return V <= right;
  }
  friend constexpr auto
  operator<=(T const &left, Closed_bound<T, V> const &right
             [[maybe_unused]]) noexcept(noexcept(left <= V) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left <= V)>) {
    return left <= V;
  }
  friend constexpr auto
  operator>=(Closed_bound<T, V> const &left [[maybe_unused]],
             T const &right) noexcept(noexcept(V >= right) &&
                                      std::is_nothrow_move_constructible_v<
                                          decltype(V >= right)>) {
    return V >= right;
  }
  friend constexpr auto
  operator>=(T const &left, Closed_bound<T, V> const &right
             [[maybe_unused]]) noexcept(noexcept(left >= V) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left >= V)>) {
    return left >= V;
  }
  friend constexpr auto
  operator<=>(Closed_bound<T, V> const &left [[maybe_unused]],
              T const &right) noexcept(noexcept(compare(V, right)) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(compare(V, right))>) {
    return compare(V, right);
  }
  friend constexpr auto
  operator<=>(T const &left, Closed_bound<T, V> const &right
              [[maybe_unused]]) noexcept(noexcept(compare(left, V)) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(compare(left, V))>) {
    return compare(left, V);
  }

private:
  constexpr static auto compare(T const &left, T const &right) noexcept(
      noexcept(left<right ? std::partial_ordering::less : left> right
                   ? std::partial_ordering::greater
               : left == right ? std::partial_ordering::equivalent
                               : std::partial_ordering::unordered) &&
      std::is_nothrow_move_constructible_v<std::partial_ordering>)
      -> std::partial_ordering {
    return left < right    ? std::partial_ordering::less
           : left > right  ? std::partial_ordering::greater
           : left == right ? std::partial_ordering::equivalent
                           : std::partial_ordering::unordered;
  }
};

// NOLINTNEXTLINE(altera-struct-pack-align)
template <std::totally_ordered T> struct Unbounded : private Bound<T> {
  using type = typename Unbounded::type;
  consteval Unbounded() noexcept = default;
  friend constexpr auto operator==(Unbounded<T> const &left [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(T const &left [[maybe_unused]],
                                   Unbounded<T> const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator!=(Unbounded<T> const &left [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator!=(T const &left [[maybe_unused]],
                                   Unbounded<T> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(Unbounded<T> const &left [[maybe_unused]],
                                  T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(T const &left [[maybe_unused]],
                                  Unbounded<T> const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(Unbounded<T> const &left [[maybe_unused]],
                                  T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(T const &left [[maybe_unused]],
                                  Unbounded<T> const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(Unbounded<T> const &left [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(T const &left [[maybe_unused]],
                                   Unbounded<T> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(Unbounded<T> const &left [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(T const &left [[maybe_unused]],
                                   Unbounded<T> const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=>(
      Unbounded<T> const &left [[maybe_unused]], T const &right
      [[maybe_unused]]) noexcept(noexcept(std::partial_ordering::unordered) &&
                                 std::is_nothrow_constructible_v<
                                     decltype(std::partial_ordering::
                                                  unordered)>) {
    return std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      T const &left [[maybe_unused]], Unbounded<T> const &right
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
  // public members to be a structual T
  T value_; // NOLINT(misc-non-private-member-variables-in-classes)
  /*
  usage
  `(constant expression)`
  checking (debug ONLY)
  - compile-time checking, causes (complicated) compile error @ assert
  */
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval Interval(T value) noexcept(noexcept(Interval{
      std::move(value), nullptr})) requires std::move_constructible<T>
      : Interval{std::move(value), nullptr} {
    /* the parameter is passed-by-value to not bound to a temporary for using
     * this type as non-type template parameters */
  }
  /*
  usage
  `{(expression), nullptr}`
  checking (debug ONLY)
  - compile-time checking requires constexpr/consteval context, causes
  (complicated) compile error @ assert
  - runtime checking
  */
  constexpr Interval(
      T const &value,
      [[maybe_unused]] std::
          nullptr_t /*unused*/) noexcept(noexcept(decltype(value_){
      value}) &&noexcept(check(value_))) requires std::copy_constructible<T>
      : value_{value} {
    check(value_);
  }
  constexpr Interval(
      T &&value, [[maybe_unused]] std::
                     nullptr_t /*unused*/) noexcept(noexcept(decltype(value_){
      std::move(value)}) &&noexcept(check(value_))) requires
      std::move_constructible<T> : value_{std::move(value)} {
    check(value_);
  }
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  [[nodiscard]] constexpr operator T &() &noexcept(noexcept(value_)) {
    return value_;
  }
  [[nodiscard]] constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator T const &() const &noexcept(noexcept(value_)) {
    return value_;
  }
  [[nodiscard]] constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator T &&() &&noexcept(noexcept(std::move(value_))) {
    return std::move(value_);
  }
  [[nodiscard]] constexpr
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  operator T const &&() const &&noexcept(noexcept(std::move(value_))) {
    return std::move(value_);
  }

private:
  constexpr static void check(T const &value) noexcept(
      noexcept(L{} < value && u8"L >(=) value") &&noexcept(value < R{} &&
                                                           u8"value >(=) R")) {
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(L{} < value && u8"L >(=) value");
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(value < R{} && u8"value >(=) R");
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
