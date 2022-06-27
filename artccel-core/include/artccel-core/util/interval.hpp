#ifndef ARTCCEL_CORE_UTIL_INTERVAL_HPP
#define ARTCCEL_CORE_UTIL_INTERVAL_HPP
#pragma once

#include "concepts_extras.hpp" // import Derived_from_but_not
#include "utility_extras.hpp"  // import Delegate
#include <cassert>             // import assert
#include <compare>             // import std::partial_ordering
#include <concepts>    // import std::move_constructible, std::totally_ordered
#include <type_traits> // import std::decay_t, std::is_nothrow_constructible_v, std::is_nothrow_move_constructible_v
#include <utility>     // import std::move

namespace artccel::core::util {
template <std::totally_ordered T> struct Bound;
template <std::totally_ordered T, T V> struct Open_bound;
template <std::totally_ordered T, T V> struct Closed_bound;
template <std::totally_ordered T> struct Unbounded;
template <typename L, Derived_from_but_not<Bound<typename L::type>> R>
requires Derived_from_but_not<L, Bound<typename L::type>> &&
    std::move_constructible<typename L::type>
class Interval;
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Dynamic_interval_t {
  explicit consteval Dynamic_interval_t() noexcept = default;
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

template <std::totally_ordered T, T Z = T{0}>
using Nonnegative_interval = LC_RU_interval<T, Z>; // [0,+∞)
template <std::totally_ordered T, T Z = T{0}>
using Nonpositive_interval = LU_RC_interval<T, Z>; // (-∞,0]
template <std::totally_ordered T, T Z = T{0}>
using Positive_interval = LO_RU_interval<T, Z>; // (0,+∞)
template <std::totally_ordered T, T Z = T{0}>
using Negative_interval = LU_RO_interval<T, Z>; // (-∞,0)

template <std::totally_ordered T>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Bound {
  using type = T;
  explicit constexpr Bound() noexcept = default;
  constexpr Bound(Bound const &) noexcept = default;
  constexpr auto operator=(Bound const &) noexcept -> Bound & = default;
  constexpr Bound(Bound &&) noexcept = default;
  constexpr auto operator=(Bound &&) noexcept -> Bound & = default;

protected:
  constexpr ~Bound() noexcept = default;
};

template <std::totally_ordered T, T V>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Open_bound : public Bound<T> {
  using type = typename Open_bound::type;
  constexpr static auto value_{V};
  explicit consteval Open_bound() noexcept = default;
  friend constexpr auto operator==(Open_bound const &left [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(T const &left [[maybe_unused]],
                                   Open_bound const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto
  operator<=(Open_bound const &left [[maybe_unused]], T const &right) noexcept(
      noexcept(V < right) &&
      std::is_nothrow_move_constructible_v<std::decay_t<decltype(V < right)>>) {
    return V < right;
  }
  friend constexpr auto
  operator<=(T const &left, Open_bound const &right [[maybe_unused]]) noexcept(
      noexcept(left < V) &&
      std::is_nothrow_move_constructible_v<std::decay_t<decltype(left < V)>>) {
    return left < V;
  }
  friend constexpr auto
  operator>=(Open_bound const &left [[maybe_unused]], T const &right) noexcept(
      noexcept(V > left) &&
      std::is_nothrow_move_constructible_v<std::decay_t<decltype(V > right)>>) {
    return V > right;
  }
  friend constexpr auto
  operator>=(T const &left, Open_bound const &right [[maybe_unused]]) noexcept(
      noexcept(left > V) &&
      std::is_nothrow_move_constructible_v<std::decay_t<decltype(left > V)>>) {
    return left > V;
  }
  friend constexpr auto operator<=>(
      Open_bound const &left [[maybe_unused]],
      T const &right) noexcept(noexcept(compare(V, right)) &&
                               std::is_nothrow_move_constructible_v<
                                   std::decay_t<decltype(compare(V, right))>>) {
    return compare(V, right);
  }
  friend constexpr auto
  operator<=>(T const &left, Open_bound const &right [[maybe_unused]]) noexcept(
      noexcept(compare(left, V)) &&
      std::is_nothrow_move_constructible_v<
          std::decay_t<decltype(compare(left, V))>>) {
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
struct Closed_bound : public Bound<T> {
  using type = typename Closed_bound::type;
  constexpr static auto value_{V};
  explicit consteval Closed_bound() noexcept = default;
  friend constexpr auto
  operator==(Closed_bound const &left [[maybe_unused]],
             T const &right) noexcept(noexcept(V == right) &&
                                      std::is_nothrow_move_constructible_v<
                                          decltype(V == right)>) {
    return V == right;
  }
  friend constexpr auto
  operator==(T const &left, Closed_bound const &right
             [[maybe_unused]]) noexcept(noexcept(left == V) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left == V)>) {
    return left == V;
  }
  friend constexpr auto
  operator<(Closed_bound const &left [[maybe_unused]],
            T const &right) noexcept(noexcept(V <= right) &&
                                     std::is_nothrow_move_constructible_v<
                                         std::decay_t<decltype(V <= right)>>) {
    return V <= right;
  }
  friend constexpr auto
  operator<(T const &left, Closed_bound const &right [[maybe_unused]]) noexcept(
      noexcept(left <= V) &&
      std::is_nothrow_move_constructible_v<std::decay_t<decltype(left <= V)>>) {
    return left <= V;
  }
  friend constexpr auto
  operator>(Closed_bound const &left [[maybe_unused]],
            T const &right) noexcept(noexcept(V >= right) &&
                                     std::is_nothrow_move_constructible_v<
                                         std::decay_t<decltype(V >= right)>>) {
    return V >= right;
  }
  friend constexpr auto
  operator>(T const &left, Closed_bound const &right [[maybe_unused]]) noexcept(
      noexcept(left >= V) &&
      std::is_nothrow_move_constructible_v<std::decay_t<decltype(left >= V)>>) {
    return left >= V;
  }
  friend constexpr auto
  operator<=>(Closed_bound const &left [[maybe_unused]],
              T const &right) noexcept(noexcept(compare(V, right)) &&
                                       std::is_nothrow_move_constructible_v<
                                           decltype(compare(V, right))>) {
    return compare(V, right);
  }
  friend constexpr auto
  operator<=>(T const &left, Closed_bound const &right
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
    return left == right  ? std::partial_ordering::equivalent
           : left < right ? std::partial_ordering::less
           : left > right ? std::partial_ordering::greater
                          : std::partial_ordering::unordered;
  }
};

// NOLINTNEXTLINE(altera-struct-pack-align)
template <std::totally_ordered T> struct Unbounded : public Bound<T> {
  using type = typename Unbounded::type;
  explicit consteval Unbounded() noexcept = default;
  friend constexpr auto operator==(Unbounded const &left [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(T const &left [[maybe_unused]],
                                   Unbounded const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator<(Unbounded const &left [[maybe_unused]],
                                  T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(T const &left [[maybe_unused]],
                                  Unbounded const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(Unbounded const &left [[maybe_unused]],
                                  T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(T const &left [[maybe_unused]],
                                  Unbounded const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(Unbounded const &left [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(T const &left [[maybe_unused]],
                                   Unbounded const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(Unbounded const &left [[maybe_unused]],
                                   T const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(T const &left [[maybe_unused]],
                                   Unbounded const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=>(
      Unbounded const &left [[maybe_unused]], T const &right
      [[maybe_unused]]) noexcept(noexcept(std::partial_ordering::unordered) &&
                                 std::is_nothrow_move_constructible_v<
                                     decltype(std::partial_ordering::
                                                  unordered)>) {
    return std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      T const &left [[maybe_unused]], Unbounded const &right
      [[maybe_unused]]) noexcept(noexcept(std::partial_ordering::unordered) &&
                                 std::is_nothrow_move_constructible_v<
                                     decltype(std::partial_ordering::
                                                  unordered)>) {
    return std::partial_ordering::unordered;
  }
};

template <typename L, Derived_from_but_not<Bound<typename L::type>> R>
requires Derived_from_but_not<L, Bound<typename L::type>> &&
    std::move_constructible<typename L::type>
class Interval : public Delegate<typename L::type, false> {
public:
  using left = L;
  using right = R;
  using type = typename Interval::type;
  /*
  usage
  `(constant expression)`
  checking (debug ONLY)
  - compile-time checking, causes (complicated) compile error @ assert
  */
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  consteval Interval(type value) noexcept(noexcept(Interval{
      std::move(value), Dynamic_interval_t{}}))
      : Interval{std::move(value), Dynamic_interval_t{}} {
    /* the parameter is passed-by-value to not bound to a temporary for using
     * this type as non-type template parameters */
  }
  /*
  usage
  `{(expression), Dynamic_interval_t{}}`
  checking (debug ONLY)
  - compile-time checking requires constexpr/consteval context, causes
  (complicated) compile error @ assert
  - runtime checking
  */
  constexpr Interval(type value, [[maybe_unused]] Dynamic_interval_t /*unused*/) noexcept(
      std::is_nothrow_constructible_v<typename Interval::Delegate,
                                      decltype(std::move(value))>
          &&noexcept(check(this->value_)))
      : Interval::Delegate{std::move(value)} {
    check(this->value_);
  }

private:
  constexpr static void
  check(type const &value) noexcept(noexcept(L{} < value && u8"L >(=) value",
                                             value < R{} && u8"value >(=) R")) {
    // clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(L{} < value && u8"L >(=) value");
    // clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(value < R{} && u8"value >(=) R");
  }
};
} // namespace artccel::core::util

#endif
