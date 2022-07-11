#ifndef ARTCCEL_CORE_UTIL_INTERVAL_HPP
#define ARTCCEL_CORE_UTIL_INTERVAL_HPP
#pragma once

#include "concepts_extras.hpp"   // import Derived_from_but_not
#include "utility_extras.hpp"    // import Delegate
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT
#include <cassert>               // import assert
#include <compare>               // import std::partial_ordering
#include <concepts>              // import std::totally_ordered
#include <type_traits> // import std::decay_t, std::is_nothrow_constructible_v, std::is_nothrow_move_constructible_v
#include <utility>     // import std::move

namespace artccel::core::util {
template <std::totally_ordered Type> struct Bound;
template <std::totally_ordered Type, Type Val> struct Open_bound;
template <std::totally_ordered Type, Type Val> struct Closed_bound;
template <std::totally_ordered Type> struct Unbounded;
template <typename Left, Derived_from_but_not<Bound<typename Left::type>> Right>
requires Derived_from_but_not<Left, Bound<typename Left::type>>
class Interval;
struct ARTCCEL_CORE_EXPORT Dynamic_interval_t {
  explicit consteval Dynamic_interval_t() noexcept = default;
};

// mathematical classifications

template <std::totally_ordered Type, Type Left, Type Right>
using Open_interval =
    Interval<Open_bound<Type, Left>, Open_bound<Type, Right>>; // (Left,Right)
template <std::totally_ordered Type, Type Left, Type Right>
using Closed_interval = Interval<Closed_bound<Type, Left>,
                                 Closed_bound<Type, Right>>; // [Left,Right]
template <std::totally_ordered Type, Type Left, Type Right>
using LC_RO_interval =
    Interval<Closed_bound<Type, Left>, Open_bound<Type, Right>>; // [Left,Right)
template <std::totally_ordered Type, Type Left, Type Right>
using LO_RC_interval =
    Interval<Open_bound<Type, Left>, Closed_bound<Type, Right>>; // (Left,Right]
template <std::totally_ordered Type, Type Left>
using LC_RU_interval =
    Interval<Closed_bound<Type, Left>, Unbounded<Type>>; // [Left,+∞)
template <std::totally_ordered Type, Type Left>
using LO_RU_interval =
    Interval<Open_bound<Type, Left>, Unbounded<Type>>; // (Left,+∞)
template <std::totally_ordered Type, Type Right>
using LU_RC_interval =
    Interval<Unbounded<Type>, Closed_bound<Type, Right>>; // (-∞,Right]
template <std::totally_ordered Type, Type Right>
using LU_RO_interval =
    Interval<Unbounded<Type>, Open_bound<Type, Right>>; // (-∞,Right)
template <std::totally_ordered Type>
using Unbounded_interval =
    Interval<Unbounded<Type>, Unbounded<Type>>; // (-∞,+∞)
template <std::totally_ordered Type, Type Val = Type{}>
using Empty_interval = Open_interval<Type, Val, Val>; // (Val,Val) = {}
template <std::totally_ordered Type, Type Val>
using Degenerate_interval =
    Closed_interval<Type, Val, Val>; // [Val,Val] = {Val}

// common uses

template <std::totally_ordered Type, Type Zero = Type{0}>
using Nonnegative_interval = LC_RU_interval<Type, Zero>; // [0,+∞)
template <std::totally_ordered Type, Type Zero = Type{0}>
using Nonpositive_interval = LU_RC_interval<Type, Zero>; // (-∞,0]
template <std::totally_ordered Type, Type Zero = Type{0}>
using Positive_interval = LO_RU_interval<Type, Zero>; // (0,+∞)
template <std::totally_ordered Type, Type Zero = Type{0}>
using Negative_interval = LU_RO_interval<Type, Zero>; // (-∞,0)

template <std::totally_ordered Type> struct Bound {
  using type = Type;
  constexpr Bound() noexcept = default;
  constexpr Bound(Bound const &) noexcept = default;
  constexpr auto operator=(Bound const &) noexcept -> Bound & = default;
  constexpr Bound(Bound &&) noexcept = default;
  constexpr auto operator=(Bound &&) noexcept -> Bound & = default;

protected:
  constexpr ~Bound() noexcept = default;
};

template <std::totally_ordered Type, Type Val>
struct Open_bound : public Bound<Type> {
  using type = typename Open_bound::type;
  constexpr static auto value_{Val};
  consteval Open_bound() noexcept = default;
  friend constexpr auto operator==(Open_bound const &left [[maybe_unused]],
                                   Type const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(Type const &left [[maybe_unused]],
                                   Open_bound const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator<=(
      Open_bound const &left [[maybe_unused]],
      Type const &right) noexcept(noexcept(Val < right) &&
                                  std::is_nothrow_move_constructible_v<
                                      std::decay_t<decltype(Val < right)>>) {
    return Val < right;
  }
  friend constexpr auto operator<=(
      Type const &left, Open_bound const &right
      [[maybe_unused]]) noexcept(noexcept(left < Val) &&
                                 std::is_nothrow_move_constructible_v<
                                     std::decay_t<decltype(left < Val)>>) {
    return left < Val;
  }
  friend constexpr auto operator>=(
      Open_bound const &left [[maybe_unused]],
      Type const &right) noexcept(noexcept(Val > left) &&
                                  std::is_nothrow_move_constructible_v<
                                      std::decay_t<decltype(Val > right)>>) {
    return Val > right;
  }
  friend constexpr auto operator>=(
      Type const &left, Open_bound const &right
      [[maybe_unused]]) noexcept(noexcept(left > Val) &&
                                 std::is_nothrow_move_constructible_v<
                                     std::decay_t<decltype(left > Val)>>) {
    return left > Val;
  }
  friend constexpr auto operator<=>(
      Open_bound const &left [[maybe_unused]],
      Type const
          &right) noexcept(noexcept(compare(Val, right)) &&
                           std::is_nothrow_move_constructible_v<
                               std::decay_t<decltype(compare(Val, right))>>) {
    return compare(Val, right);
  }
  friend constexpr auto operator<=>(
      Type const &left, Open_bound const &right
      [[maybe_unused]]) noexcept(noexcept(compare(left, Val)) &&
                                 std::is_nothrow_move_constructible_v<
                                     std::decay_t<decltype(compare(left,
                                                                   Val))>>) {
    return compare(left, Val);
  }

private:
  constexpr static auto compare(Type const &left, Type const &right) noexcept(
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

template <std::totally_ordered Type, Type Val>
struct Closed_bound : public Bound<Type> {
  using type = typename Closed_bound::type;
  constexpr static auto value_{Val};
  consteval Closed_bound() noexcept = default;
  friend constexpr auto
  operator==(Closed_bound const &left [[maybe_unused]],
             Type const &right) noexcept(noexcept(Val == right) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(Val == right)>) {
    return Val == right;
  }
  friend constexpr auto
  operator==(Type const &left, Closed_bound const &right
             [[maybe_unused]]) noexcept(noexcept(left == Val) &&
                                        std::is_nothrow_move_constructible_v<
                                            decltype(left == Val)>) {
    return left == Val;
  }
  friend constexpr auto operator<(
      Closed_bound const &left [[maybe_unused]],
      Type const &right) noexcept(noexcept(Val <= right) &&
                                  std::is_nothrow_move_constructible_v<
                                      std::decay_t<decltype(Val <= right)>>) {
    return Val <= right;
  }
  friend constexpr auto operator<(
      Type const &left, Closed_bound const &right
      [[maybe_unused]]) noexcept(noexcept(left <= Val) &&
                                 std::is_nothrow_move_constructible_v<
                                     std::decay_t<decltype(left <= Val)>>) {
    return left <= Val;
  }
  friend constexpr auto operator>(
      Closed_bound const &left [[maybe_unused]],
      Type const &right) noexcept(noexcept(Val >= right) &&
                                  std::is_nothrow_move_constructible_v<
                                      std::decay_t<decltype(Val >= right)>>) {
    return Val >= right;
  }
  friend constexpr auto operator>(
      Type const &left, Closed_bound const &right
      [[maybe_unused]]) noexcept(noexcept(left >= Val) &&
                                 std::is_nothrow_move_constructible_v<
                                     std::decay_t<decltype(left >= Val)>>) {
    return left >= Val;
  }
  friend constexpr auto
  operator<=>(Closed_bound const &left [[maybe_unused]],
              Type const &right) noexcept(noexcept(compare(Val, right)) &&
                                          std::is_nothrow_move_constructible_v<
                                              decltype(compare(Val, right))>) {
    return compare(Val, right);
  }
  friend constexpr auto
  operator<=>(Type const &left, Closed_bound const &right
              [[maybe_unused]]) noexcept(noexcept(compare(left, Val)) &&
                                         std::is_nothrow_move_constructible_v<
                                             decltype(compare(left, Val))>) {
    return compare(left, Val);
  }

private:
  constexpr static auto compare(Type const &left, Type const &right) noexcept(
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

template <std::totally_ordered Type> struct Unbounded : public Bound<Type> {
  using type = typename Unbounded::type;
  consteval Unbounded() noexcept = default;
  friend constexpr auto operator==(Unbounded const &left [[maybe_unused]],
                                   Type const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator==(Type const &left [[maybe_unused]],
                                   Unbounded const &right
                                   [[maybe_unused]]) noexcept {
    return false;
  }
  friend constexpr auto operator<(Unbounded const &left [[maybe_unused]],
                                  Type const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<(Type const &left [[maybe_unused]],
                                  Unbounded const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(Unbounded const &left [[maybe_unused]],
                                  Type const &right [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>(Type const &left [[maybe_unused]],
                                  Unbounded const &right
                                  [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(Unbounded const &left [[maybe_unused]],
                                   Type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=(Type const &left [[maybe_unused]],
                                   Unbounded const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(Unbounded const &left [[maybe_unused]],
                                   Type const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator>=(Type const &left [[maybe_unused]],
                                   Unbounded const &right
                                   [[maybe_unused]]) noexcept {
    return true;
  }
  friend constexpr auto operator<=>(
      Unbounded const &left [[maybe_unused]], Type const &right
      [[maybe_unused]]) noexcept(noexcept(std::partial_ordering::unordered) &&
                                 std::is_nothrow_move_constructible_v<
                                     decltype(std::partial_ordering::
                                                  unordered)>) {
    return std::partial_ordering::unordered;
  }
  friend constexpr auto operator<=>(
      Type const &left [[maybe_unused]], Unbounded const &right
      [[maybe_unused]]) noexcept(noexcept(std::partial_ordering::unordered) &&
                                 std::is_nothrow_move_constructible_v<
                                     decltype(std::partial_ordering::
                                                  unordered)>) {
    return std::partial_ordering::unordered;
  }
};

template <typename Left, Derived_from_but_not<Bound<typename Left::type>> Right>
requires Derived_from_but_not<Left, Bound<typename Left::type>>
class Interval : public Delegate<typename Left::type, false> {
public:
  using left = Left;
  using right = Right;
  using type = typename Interval::type;
  /*
  usage
  `(constant expression)`
  checking (debug ONLY)
  - compile-time checking, causes (complicated) compile error @ assert
  */
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
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
  constexpr static void check(type const &value) noexcept(
      noexcept(Left{} < value && u8"value is outside the lower bound",
               value < Right{} && u8"value is outside the upper bound")) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(Left{} < value && u8"value is outside the lower bound");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(value < Right{} && u8"value is outside the upper bound");
  }
};
} // namespace artccel::core::util

#endif
