#ifndef GUARD_557BEE04_C882_4E8E_BAC1_6411844FEDE8
#define GUARD_557BEE04_C882_4E8E_BAC1_6411844FEDE8
#pragma once

#include <cassert>     // import assert
#include <concepts>    // import std::same_as, std::totally_ordered
#include <type_traits> // import std::is_nothrow_constructible_v, std::remove_cv_t
#include <utility>     // import std::move

#include "concepts_extras.hpp" // import Derived_from_but_not
#include "utility_extras.hpp"  // import Consteval_t, Delegate

namespace artccel::core::util {
template <std::totally_ordered Type, typename Derived> struct Bound;
template <typename Type>
concept Bound_c = requires(std::remove_cv_t<Type> type_norm) {
  Derived_from_but_not<
      decltype(type_norm),
      Bound<typename decltype(type_norm)::type, decltype(type_norm)>>;
};
template <std::totally_ordered Type> struct Open_bound;
template <std::totally_ordered Type> struct Closed_bound;
template <std::totally_ordered Type> struct Unbounded;
template <Bound_c auto Left, Bound_c auto Right>
requires std::same_as < typename std::remove_cv_t<decltype(Left)>::type,
typename std::remove_cv_t<decltype(Right)>::type > class Interval;

// mathematical classifications
template <std::totally_ordered Type, Type Left, Type Right>
using Open_interval =
    Interval<Open_bound{Left}, Open_bound{Right}>; // (Left,Right)
template <std::totally_ordered Type, Type Left, Type Right>
using Closed_interval =
    Interval<Closed_bound{Left}, Closed_bound{Right}>; // [Left,Right]
template <std::totally_ordered Type, Type Left, Type Right>
using LC_RO_interval =
    Interval<Closed_bound{Left}, Open_bound{Right}>; // [Left,Right)
template <std::totally_ordered Type, Type Left, Type Right>
using LO_RC_interval =
    Interval<Open_bound{Left}, Closed_bound{Right}>; // (Left,Right]
template <std::totally_ordered Type, Type Left>
using LC_RU_interval =
    Interval<Closed_bound{Left}, Unbounded<Type>{}>; // [Left,+∞)
template <std::totally_ordered Type, Type Left>
using LO_RU_interval =
    Interval<Open_bound{Left}, Unbounded<Type>{}>; // (Left,+∞)
template <std::totally_ordered Type, Type Right>
using LU_RC_interval =
    Interval<Unbounded<Type>{}, Closed_bound{Right}>; // (-∞,Right]
template <std::totally_ordered Type, Type Right>
using LU_RO_interval =
    Interval<Unbounded<Type>{}, Open_bound{Right}>; // (-∞,Right)
template <std::totally_ordered Type>
using Unbounded_interval =
    Interval<Unbounded<Type>{}, Unbounded<Type>{}>; // (-∞,+∞)
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

template <std::totally_ordered Type, typename Derived> struct Bound {
  using type = Type;
  constexpr Bound() noexcept = default;
  constexpr Bound(Bound const &) noexcept = default;
  constexpr auto operator=(Bound const &) noexcept -> Bound & = default;
  constexpr Bound(Bound &&) noexcept = default;
  constexpr auto operator=(Bound &&) noexcept -> Bound & = default;

  friend constexpr auto
  operator<(Bound const &left, Type const &right) noexcept(
      noexcept(static_cast<Derived const &>(left) < right)) -> decltype(auto) {
    return static_cast<Derived const &>(left) < right;
  }
  friend constexpr auto
  operator<(Type const &left, Bound const &right) noexcept(
      noexcept(left < static_cast<Derived const &>(right))) -> decltype(auto) {
    return left < static_cast<Derived const &>(right);
  }
  friend constexpr auto
  operator>(Bound const &left, Type const &right) noexcept(
      noexcept(static_cast<Derived const &>(left) > left)) -> decltype(auto) {
    return static_cast<Derived const &>(left) > right;
  }
  friend constexpr auto
  operator>(Type const &left, Bound const &right) noexcept(
      noexcept(left > static_cast<Derived const &>(right))) -> decltype(auto) {
    return left > static_cast<Derived const &>(right);
  }

protected:
  constexpr ~Bound() noexcept = default;
};

template <std::totally_ordered Type>
struct Open_bound : public Bound<Type, Open_bound<Type>> {
  Type value_;
  explicit constexpr Open_bound(Type value) noexcept(noexcept(decltype(value_){
      std::move(value)}))
      : value_{std::move(value)} {};
  friend constexpr auto
  operator<(Open_bound const &left,
            Type const &right) noexcept(noexcept(left.value_ < right))
      -> decltype(auto) {
    return left.value_ < right;
  }
  friend constexpr auto
  operator<(Type const &left,
            Open_bound const &right) noexcept(noexcept(left < right.value_))
      -> decltype(auto) {
    return left < right.value_;
  }
  friend constexpr auto
  operator>(Open_bound const &left,
            Type const &right) noexcept(noexcept(left.value_ > left))
      -> decltype(auto) {
    return left.value_ > right;
  }
  friend constexpr auto
  operator>(Type const &left,
            Open_bound const &right) noexcept(noexcept(left > right.value_))
      -> decltype(auto) {
    return left > right.value_;
  }
};

template <std::totally_ordered Type>
struct Closed_bound : public Bound<Type, Closed_bound<Type>> {
  Type value_;
  explicit constexpr Closed_bound(Type value) noexcept(
      noexcept(decltype(value_){std::move(value)}))
      : value_{std::move(value)} {};
  friend constexpr auto
  operator<(Closed_bound const &left,
            Type const &right) noexcept(noexcept(left.value_ <= right))
      -> decltype(auto) {
    return left.value_ <= right;
  }
  friend constexpr auto
  operator<(Type const &left,
            Closed_bound const &right) noexcept(noexcept(left <= right.value_))
      -> decltype(auto) {
    return left <= right.value_;
  }
  friend constexpr auto
  operator>(Closed_bound const &left,
            Type const &right) noexcept(noexcept(left.value_ >= right))
      -> decltype(auto) {
    return left.value_ >= right;
  }
  friend constexpr auto
  operator>(Type const &left,
            Closed_bound const &right) noexcept(noexcept(left >= right.value_))
      -> decltype(auto) {
    return left >= right.value_;
  }
};

template <std::totally_ordered Type>
struct Unbounded : public Bound<Type, Unbounded<Type>> {
  explicit consteval Unbounded() noexcept = default;
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
};

template <Bound_c auto Left, Bound_c auto Right>
requires std::same_as < typename std::remove_cv_t<decltype(Left)>::type,
typename std::remove_cv_t<decltype(Right)>::type > class Interval
    : public Delegate<typename decltype(Left)::type, false> {
public:
  using type = typename Interval::type;
  constexpr static auto left_{Left};
  constexpr static auto right_{Right};
  /*
  usage
  `{Consteval_t{}, (constant expression)}`
  checking (debug ONLY)
  - compile-time checking, causes (complicated) compile error @ assert
  */
  consteval Interval(Consteval_t tag [[maybe_unused]],
                     type value) noexcept(noexcept(Interval{std::move(value)}))
      : Interval{std::move(value)} {
    /* the parameter is passed-by-value to not bound to a temporary for using
     * this type as non-type template parameters */
  }
  /*
  usage
  `(expression)`
  checking (debug ONLY)
  - compile-time checking requires constexpr/consteval context, causes
  (complicated) compile error @ assert
  - runtime checking
  */
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  constexpr Interval(type value) noexcept(
      std::is_nothrow_constructible_v<typename Interval::Delegate,
                                      decltype(std::move(value))>
          &&noexcept(check(this->value_)))
      : Interval::Delegate{std::move(value)} {
    check(this->value_);
  }

private:
  constexpr static void check(type const &value) noexcept(
      noexcept(Left < value && u8"value is outside the lower bound",
               value < Right && u8"value is outside the upper bound")) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(Left < value && u8"value is outside the lower bound");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(value < Right && u8"value is outside the upper bound");
  }
};
} // namespace artccel::core::util

#endif
