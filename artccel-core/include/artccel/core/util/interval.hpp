#ifndef GUARD_557BEE04_C882_4E8E_BAC1_6411844FEDE8
#define GUARD_557BEE04_C882_4E8E_BAC1_6411844FEDE8
#pragma once

#include <cassert>  // import assert
#include <compare>  // import std::is_eq, std::strong_ordering
#include <concepts> // import std::same_as, std::totally_ordered
#include <type_traits> // import std::is_nothrow_constructible_v, std::remove_cv_t, std::remove_cvref_t
#include <utility> // import std::forward, std::move

#include "concepts_extras.hpp" // import Brace_convertible_to, Derived_from_but_not, Differ_from
#include "utility_extras.hpp" // import Consteval_t, Delegate, dependent_false_v

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
template <Bound_c BoundT, typename Derived> struct Directional_bound;
template <typename Type>
concept Directional_bound_c = requires(std::remove_cv_t<Type> type_norm) {
  Derived_from_but_not<
      decltype(type_norm),
      Directional_bound<typename decltype(type_norm)::bound_type,
                        decltype(type_norm)>>;
};
template <Bound_c BoundT> struct Left_bound;
template <Bound_c BoundT> struct Right_bound;
template <Bound_c auto Left, Bound_c auto Right>
requires std::same_as < typename std::remove_cv_t<decltype(Left)>::type,
typename std::remove_cv_t<decltype(Right)>::type > class Interval;
template <typename Type>
concept Interval_c = requires(std::remove_cv_t<Type> type_norm) {
  std::same_as<decltype(type_norm), Interval<decltype(type_norm)::left_,
                                             decltype(type_norm)::right_>>;
};

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

protected:
  constexpr ~Bound() noexcept = default;
};

template <std::totally_ordered Type>
struct Open_bound : public Bound<Type, Open_bound<Type>> {
  Type value_;
  explicit constexpr Open_bound(Type value) noexcept(noexcept(decltype(value_){
      std::move(value)}))
      : value_{std::move(value)} {};
  template <Bound_c Bound>
  requires(
      !std::same_as<Bound, Open_bound> &&
      std::same_as<Bound, Open_bound<typename Bound::type>> &&
      Brace_convertible_to<
          typename Bound::type,
          Type>) explicit constexpr Open_bound(Bound &&
                                                   other) noexcept(noexcept(Open_bound{
      Type{std::forward<Bound>(other).value_}}))
      : Open_bound{Type{std::forward<Bound>(other).value_}} {}

  friend constexpr auto operator<(Open_bound const &left,
                                  Type const &right) noexcept(noexcept(bool{
      left.value_ < right})) -> bool {
    return left.value_ < right;
  }
  friend constexpr auto
  operator<(Type const &left, Open_bound const &right) noexcept(noexcept(bool{
      left < right.value_})) -> bool {
    return left < right.value_;
  }
  friend constexpr auto operator>(Open_bound const &left,
                                  Type const &right) noexcept(noexcept(bool{
      left.value_ > right})) -> bool {
    return left.value_ > right;
  }
  friend constexpr auto
  operator>(Type const &left, Open_bound const &right) noexcept(noexcept(bool{
      left > right.value_})) -> bool {
    return left > right.value_;
  }
};

template <std::totally_ordered Type>
struct Closed_bound : public Bound<Type, Closed_bound<Type>> {
  Type value_;
  explicit constexpr Closed_bound(Type value) noexcept(
      noexcept(decltype(value_){std::move(value)}))
      : value_{std::move(value)} {};
  template <Bound_c Bound>
  requires(
      !std::same_as<Bound, Closed_bound> &&
      std::same_as<Bound, Closed_bound<typename Bound::type>> &&
      Brace_convertible_to<
          typename Bound::type,
          Type>) explicit constexpr Closed_bound(Bound &&
                                                     other) noexcept(noexcept(Closed_bound{
      Type{std::forward<Bound>(other).value_}}))
      : Closed_bound{Type{std::forward<Bound>(other).value_}} {}

  friend constexpr auto operator<(Closed_bound const &left,
                                  Type const &right) noexcept(noexcept(bool{
      left.value_ <= right})) -> bool {
    return left.value_ <= right;
  }
  friend constexpr auto
  operator<(Type const &left, Closed_bound const &right) noexcept(noexcept(bool{
      left <= right.value_})) -> bool {
    return left <= right.value_;
  }
  friend constexpr auto operator>(Closed_bound const &left,
                                  Type const &right) noexcept(noexcept(bool{
      left.value_ >= right})) -> bool {
    return left.value_ >= right;
  }
  friend constexpr auto
  operator>(Type const &left, Closed_bound const &right) noexcept(noexcept(bool{
      left >= right.value_})) -> bool {
    return left >= right.value_;
  }
};

template <std::totally_ordered Type>
struct Unbounded : public Bound<Type, Unbounded<Type>> {
  explicit consteval Unbounded() noexcept = default;
  template <Bound_c Bound>
  requires(
      !std::same_as<Bound, Unbounded> &&
      std::same_as<Bound, Unbounded<typename Bound::type>> &&
      Brace_convertible_to<
          typename Bound::type,
          Type>) explicit constexpr Unbounded(Bound &&other
                                              [[maybe_unused]]) noexcept(noexcept(Unbounded{}))
      : Unbounded{} {}

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

template <Bound_c BoundT, typename Derived> struct Directional_bound {
  using bound_type = BoundT;
  using type = typename BoundT::type;

  constexpr Directional_bound() noexcept = default;
  constexpr Directional_bound(Directional_bound const &) noexcept = default;
  constexpr auto operator=(Directional_bound const &) noexcept
      -> Directional_bound & = default;
  constexpr Directional_bound(Directional_bound &&) noexcept = default;
  constexpr auto operator=(Directional_bound &&) noexcept
      -> Directional_bound & = default;

protected:
  constexpr ~Directional_bound() noexcept = default;
};

template <Bound_c BoundT>
struct Left_bound : public Directional_bound<BoundT, Left_bound<BoundT>> {
  using type = typename Left_bound::type;

  BoundT bound_;
  explicit constexpr Left_bound(BoundT bound) noexcept(
      noexcept(decltype(bound_){std::move(bound)}))
      : bound_{std::move(bound)} {};
  template <Directional_bound_c DirBound>
  requires(
      !std::same_as<DirBound, Left_bound> &&
      std::same_as<DirBound, Left_bound<typename DirBound::bound_type>> &&
      Brace_convertible_to<
          typename DirBound::bound_type,
          BoundT>) explicit constexpr Left_bound(DirBound &&
                                                     other) noexcept(noexcept(Left_bound{
      BoundT{std::forward<DirBound>(other).bound_}}))
      : Left_bound{BoundT{std::forward<DirBound>(other).bound_}} {}

  friend constexpr auto operator<(Left_bound const &left,
                                  type const &right) noexcept(noexcept(bool{
      left.value_ < right})) -> bool {
    return left.bound_ < right;
  }
  friend constexpr auto
  operator<(type const &left,
            Left_bound const &right) noexcept(noexcept(bool{right > left}))
      -> bool {
    return right > left;
  }
  friend constexpr auto operator>(Left_bound const &left,
                                  type const &right) noexcept(noexcept(bool{
      !(left < right)})) -> bool {
    return !(left < right);
  }
  friend constexpr auto
  operator>(type const &left,
            Left_bound const &right) noexcept(noexcept(bool{right < left}))
      -> bool {
    return right < left;
  }
};
template <std::totally_ordered LeftT, std::totally_ordered_with<LeftT> RightT,
          template <typename> typename LeftBoundT,
          template <typename> typename RightBoundT>
constexpr auto operator<=>(Left_bound<LeftBoundT<LeftT>> const &left,
                           Left_bound<RightBoundT<RightT>> const
                               &right) noexcept(noexcept(left.bound_.value_ <=>
                                                         right.bound_.value_))
    -> std::strong_ordering {
  using left_bound_type = LeftBoundT<LeftT>;
  using right_bound_type = RightBoundT<RightT>;
  if constexpr (std::same_as<left_bound_type, Unbounded<LeftT>> &&
                std::same_as<right_bound_type, Unbounded<RightT>>) {
    return std::strong_ordering::equivalent;
  } else if constexpr (std::same_as<left_bound_type, Unbounded<LeftT>>) {
    return std::strong_ordering::less;
  } else if constexpr (std::same_as<right_bound_type, Unbounded<RightT>>) {
    return std::strong_ordering::greater;
  } else {
    auto ret{left.bound_.value_ <=> right.bound_.value_};
    if constexpr (std::same_as<left_bound_type, right_bound_type>) {
      return ret;
    } else {
      if (std::is_eq(ret)) {
        if constexpr (std::same_as<left_bound_type, Open_bound<LeftT>> &&
                      std::same_as<right_bound_type, Closed_bound<RightT>>) {
          return std::strong_ordering::greater;
        } else {
          static_assert(std::same_as<left_bound_type, Closed_bound<LeftT>> &&
                            std::same_as<right_bound_type, Open_bound<RightT>>,
                        u8"Non-exhaustive");
          return std::strong_ordering::less;
        }
      }
      return ret;
    }
  }
}
template <std::totally_ordered LeftT, std::totally_ordered_with<LeftT> RightT,
          template <typename> typename LeftBoundT,
          template <typename> typename RightBoundT>
constexpr auto operator==(Left_bound<LeftBoundT<LeftT>> const &left,
                          Left_bound<RightBoundT<RightT>> const
                              &right) noexcept(noexcept(left <=> right)) {
  return std::is_eq(left <=> right);
}

template <Bound_c BoundT>
struct Right_bound : public Directional_bound<BoundT, Right_bound<BoundT>> {
  using type = typename Right_bound::type;

  BoundT bound_;
  explicit constexpr Right_bound(BoundT bound) noexcept(
      noexcept(decltype(bound_){std::move(bound)}))
      : bound_{std::move(bound)} {};
  template <Directional_bound_c DirBound>
  requires(
      !std::same_as<DirBound, Right_bound> &&
      std::same_as<DirBound, Right_bound<typename DirBound::bound_type>> &&
      Brace_convertible_to<
          typename DirBound::bound_type,
          BoundT>) explicit constexpr Right_bound(DirBound &&
                                                      other) noexcept(noexcept(Right_bound{
      BoundT{std::forward<DirBound>(other).bound_}}))
      : Right_bound{BoundT{std::forward<DirBound>(other).bound_}} {}

  friend constexpr auto
  operator<(Right_bound const &left,
            type const &right) noexcept(noexcept(bool{right > left})) -> bool {
    return right > left;
  }
  friend constexpr auto
  operator<(type const &left, Right_bound const &right) noexcept(noexcept(bool{
      left < right.bound_})) -> bool {
    return left < right.bound_;
  }
  friend constexpr auto
  operator>(Right_bound const &left,
            type const &right) noexcept(noexcept(bool{right < left})) -> bool {
    return right < left;
  }
  friend constexpr auto
  operator>(type const &left,
            Right_bound const &right) noexcept(noexcept(bool{!(left < right)}))
      -> bool {
    return !(left < right);
  }
};
template <std::totally_ordered LeftT, std::totally_ordered_with<LeftT> RightT,
          template <typename> typename LeftBoundT,
          template <typename> typename RightBoundT>
constexpr auto operator<=>(Right_bound<LeftBoundT<LeftT>> const &left,
                           Right_bound<RightBoundT<RightT>> const
                               &right) noexcept(noexcept(left.bound_.value_ <=>
                                                         right.bound_.value_))
    -> std::strong_ordering {
  using left_bound_type = LeftBoundT<LeftT>;
  using right_bound_type = RightBoundT<RightT>;
  if constexpr (std::same_as<left_bound_type, Unbounded<LeftT>> &&
                std::same_as<right_bound_type, Unbounded<RightT>>) {
    return std::strong_ordering::equivalent;
  } else if constexpr (std::same_as<left_bound_type, Unbounded<LeftT>>) {
    return std::strong_ordering::greater;
  } else if constexpr (std::same_as<right_bound_type, Unbounded<RightT>>) {
    return std::strong_ordering::less;
  } else {
    auto ret{left.bound_.value_ <=> right.bound_.value_};
    if constexpr (std::same_as<left_bound_type, right_bound_type>) {
      return ret;
    } else {
      if (std::is_eq(ret)) {
        if constexpr (std::same_as<left_bound_type, Open_bound<LeftT>> &&
                      std::same_as<right_bound_type, Closed_bound<RightT>>) {
          return std::strong_ordering::less;
        } else {
          static_assert(std::same_as<left_bound_type, Closed_bound<LeftT>> &&
                            std::same_as<right_bound_type, Open_bound<RightT>>,
                        u8"Non-exhaustive");
          return std::strong_ordering::greater;
        }
      }
      return ret;
    }
  }
}
template <std::totally_ordered LeftT, std::totally_ordered_with<LeftT> RightT,
          template <typename> typename LeftBoundT,
          template <typename> typename RightBoundT>
constexpr auto operator==(Right_bound<LeftBoundT<LeftT>> const &left,
                          Right_bound<RightBoundT<RightT>> const
                              &right) noexcept(noexcept(left <=> right)) {
  return std::is_eq(left <=> right);
}

template <Bound_c auto Left, Bound_c auto Right>
requires std::same_as < typename std::remove_cv_t<decltype(Left)>::type,
typename std::remove_cv_t<decltype(Right)>::type > class Interval
    : public Delegate<typename decltype(Left)::type, false> {
public:
  using type = typename Interval::type;
  constexpr static auto left_{Left};
  constexpr static auto right_{Right};
  consteval Interval(Consteval_t tag [[maybe_unused]],
                     type value) noexcept(noexcept(Interval{std::move(value)}))
      : Interval{std::move(value)} {
    /* the parameter is passed-by-value to not bound to a temporary for using
     * this type as non-type template parameters */
  }
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  constexpr Interval(type value) noexcept(
      std::is_nothrow_constructible_v<typename Interval::Delegate,
                                      decltype(std::move(value))>
          &&noexcept(check(this->value_)))
      : Interval::Delegate{std::move(value)} {
    check(this->value_);
  }
  template <Interval_c Intl>
  requires Differ_from<std::remove_cvref_t<Intl>, Interval>
  constexpr Interval(Intl &&other) = delete;
  template <Interval_c Intl>
  requires(
      Differ_from<std::remove_cvref_t<Intl>, Interval> &&Brace_convertible_to<
          typename std::remove_cvref_t<Intl>::type, type> &&Left_bound{
          std::remove_cvref_t<Intl>::left_} >= Left_bound{Left} &&
      Right_bound{std::remove_cvref_t<Intl>::right_} <=
          Right_bound{
              Right}) constexpr Interval(Intl &&
                                             other) noexcept(std::
                                                                 is_nothrow_constructible_v<
                                                                     typename Intl::
                                                                         Delegate,
                                                                     decltype(std::forward<
                                                                                  Intl>(
                                                                                  other)
                                                                                  .value_)>)

      : Intl::Delegate{std::forward<Intl>(other).value_} {}

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
