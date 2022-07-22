#ifndef GUARD_557BEE04_C882_4E8E_BAC1_6411844FEDE8
#define GUARD_557BEE04_C882_4E8E_BAC1_6411844FEDE8
#pragma once

#include <cassert>  // import assert
#include <compare>  // import std::compare_three_way_result_t, std::is_eq
#include <concepts> // import std::same_as, std::three_way_comparable, std::three_way_comparable_with
#include <type_traits> // import std::is_nothrow_constructible_v, std::remove_cv_t, std::remove_cvref_t
#include <utility> // import std::declval, std::forward, std::move

#include "concepts_extras.hpp" // import Brace_convertible_to, Derived_from_but_not, Differ_from, Guard_special_constructors
#include "contracts.hpp"       // import Validator_c

namespace artccel::core::util {
template <std::three_way_comparable Type, typename Derived> struct Bound;
template <typename Type>
concept Bound_c = Derived_from_but_not<
    std::remove_cv_t<Type>,
    Bound<typename std::remove_cv_t<Type>::type, std::remove_cv_t<Type>>>;
template <std::three_way_comparable Type> struct Open_bound;
template <std::three_way_comparable Type> struct Closed_bound;
template <std::three_way_comparable Type> struct Unbounded;
template <Bound_c BoundT, typename Derived> struct Directional_bound;
template <typename Type>
concept Directional_bound_c = Derived_from_but_not<
    std::remove_cv_t<Type>,
    Directional_bound<typename std::remove_cv_t<Type>::bound_type,
                      std::remove_cv_t<Type>>>;
template <Bound_c BoundT> struct Left_bound;
template <Bound_c BoundT> struct Right_bound;
template <Bound_c LeftBoundT, Bound_c RightBoundT>
requires std::same_as<typename LeftBoundT::type, typename RightBoundT::type>
class Interval;
template <typename Type>
concept Interval_c = std::same_as < std::remove_cv_t<Type>,
        Interval < typename std::remove_cv_t<Type>::left_bound_type,
typename std::remove_cv_t<Type>::right_bound_type >> ;

// mathematical classifications
template <std::three_way_comparable Type, Type Left, Type Right>
constexpr Interval open_interval{Open_bound{Left},
                                 Open_bound{Right}}; // (Left,Right)
template <std::three_way_comparable Type, Type Left, Type Right>
constexpr Interval closed_interval{Closed_bound{Left},
                                   Closed_bound{Right}}; // [Left,Right]
template <std::three_way_comparable Type, Type Left, Type Right>
constexpr Interval lc_ro_interval{Closed_bound{Left},
                                  Open_bound{Right}}; // [Left,Right)
template <std::three_way_comparable Type, Type Left, Type Right>
constexpr Interval lo_rc_interval{Open_bound{Left},
                                  Closed_bound{Right}}; // (Left,Right]
template <std::three_way_comparable Type, Type Left>
constexpr Interval lc_ru_interval{Closed_bound{Left},
                                  Unbounded<Type>{}}; // [Left,+∞)
template <std::three_way_comparable Type, Type Left>
constexpr Interval lo_ru_interval{Open_bound{Left},
                                  Unbounded<Type>{}}; // (Left,+∞)
template <std::three_way_comparable Type, Type Right>
constexpr Interval lu_rc_interval{Unbounded<Type>{},
                                  Closed_bound{Right}}; // (-∞,Right]
template <std::three_way_comparable Type, Type Right>
constexpr Interval lu_ro_interval{Unbounded<Type>{},
                                  Open_bound{Right}}; // (-∞,Right)
template <std::three_way_comparable Type>
constexpr Interval unbounded_interval{Unbounded<Type>{},
                                      Unbounded<Type>{}}; // (-∞,+∞)
template <std::three_way_comparable Type, Type Val = Type{}>
constexpr auto empty_interval{open_interval<Type, Val, Val>}; // (Val,Val) = {}
template <std::three_way_comparable Type, Type Val>
constexpr auto degenerate_interval{
    closed_interval<Type, Val, Val>}; // [Val,Val] = {Val}
// common uses
template <std::three_way_comparable Type, Type Zero = Type{0}>
constexpr auto nonnegative_interval{lc_ru_interval<Type, Zero>}; // [0,+∞)
template <std::three_way_comparable Type, Type Zero = Type{0}>
constexpr auto nonpositive_interval{lu_rc_interval<Type, Zero>}; // (-∞,0]
template <std::three_way_comparable Type, Type Zero = Type{0}>
constexpr auto positive_interval{lo_ru_interval<Type, Zero>}; // (0,+∞)
template <std::three_way_comparable Type, Type Zero = Type{0}>
constexpr auto negative_interval{lu_ro_interval<Type, Zero>}; // (-∞,0)

template <std::three_way_comparable Type, typename Derived> struct Bound {
  using type = Type;

  constexpr Bound() noexcept = default;
  constexpr Bound(Bound const &) noexcept = default;
  constexpr auto operator=(Bound const &) noexcept -> Bound & = default;
  constexpr Bound(Bound &&) noexcept = default;
  constexpr auto operator=(Bound &&) noexcept -> Bound & = default;

protected:
  constexpr ~Bound() noexcept = default;
};

template <std::three_way_comparable Type>
struct Open_bound : public Bound<Type, Open_bound<Type>> {
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  Type value_;
  explicit constexpr Open_bound(Type value) noexcept(noexcept(decltype(value_){
      std::move(value)}))
      : value_{std::move(value)} {}
  template <Guard_special_constructors<Open_bound> Other>
  requires std::same_as<
      std::remove_cvref_t<Other>,
      Open_bound<typename std::remove_cvref_t<Other>::type>> &&
      Brace_convertible_to<typename std::remove_cvref_t<Other>::type, Type>
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
  explicit constexpr Open_bound(Other &&other) noexcept(noexcept(Open_bound{
      Type{std::forward<Other>(other).value_}}))
      : Open_bound{Type{std::forward<Other>(other).value_}} {}
  friend constexpr auto operator<=>(Open_bound const &left,
                                    Open_bound const &right)
      -> std::compare_three_way_result_t<Type> = default;

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

template <std::three_way_comparable Type>
struct Closed_bound : public Bound<Type, Closed_bound<Type>> {
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  Type value_;
  explicit constexpr Closed_bound(Type value) noexcept(
      noexcept(decltype(value_){std::move(value)}))
      : value_{std::move(value)} {}
  template <Guard_special_constructors<Closed_bound> Other>
  requires std::same_as<
      std::remove_cvref_t<Other>,
      Closed_bound<typename std::remove_cvref_t<Other>::type>> &&
      Brace_convertible_to<typename std::remove_cvref_t<Other>::type, Type>
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
  explicit constexpr Closed_bound(Other &&other) noexcept(noexcept(Closed_bound{
      Type{std::forward<Other>(other).value_}}))
      : Closed_bound{Type{std::forward<Other>(other).value_}} {}
  friend constexpr auto operator<=>(Closed_bound const &left,
                                    Closed_bound const &right)
      -> std::compare_three_way_result_t<Type> = default;

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

template <std::three_way_comparable Type>
struct Unbounded : public Bound<Type, Unbounded<Type>> {
  explicit consteval Unbounded() noexcept = default;
  template <Guard_special_constructors<Unbounded> Other>
  requires std::same_as<std::remove_cvref_t<Other>,
                        Unbounded<typename std::remove_cvref_t<Other>::type>> &&
      Brace_convertible_to<typename std::remove_cvref_t<Other>::type, Type>
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
  explicit constexpr Unbounded(Other &&other
                               [[maybe_unused]]) noexcept(noexcept(Unbounded{}))
      : Unbounded{} {}
  friend constexpr auto operator<=>(Unbounded const &left,
                                    Unbounded const &right)
      -> std::compare_three_way_result_t<Type> = default;

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

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  BoundT bound_;
  explicit constexpr Left_bound(BoundT bound) noexcept(
      noexcept(decltype(bound_){std::move(bound)}))
      : bound_{std::move(bound)} {}
  template <Guard_special_constructors<Left_bound> Other>
  requires std::same_as<
      std::remove_cvref_t<Other>,
      Left_bound<typename std::remove_cvref_t<Other>::bound_type>> &&
      Brace_convertible_to<typename std::remove_cvref_t<Other>::bound_type,
                           BoundT>
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
  explicit constexpr Left_bound(Other &&other) noexcept(noexcept(Left_bound{
      BoundT{std::forward<Other>(other).bound_}}))
      : Left_bound{BoundT{std::forward<Other>(other).bound_}} {}

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
template <std::three_way_comparable LeftT,
          std::three_way_comparable_with<LeftT> RightT,
          template <typename> typename LeftBoundT,
          template <typename> typename RightBoundT>
constexpr auto
operator<=>(Left_bound<LeftBoundT<LeftT>> const &left,
            Left_bound<RightBoundT<RightT>> const
                &right) noexcept(noexcept(std::declval<LeftT>() <=>
                                          std::declval<RightT>())) {
  using ret_type = std::compare_three_way_result_t<LeftT, RightT>;
  using left_bound_type = LeftBoundT<LeftT>;
  using right_bound_type = RightBoundT<RightT>;
  if constexpr (std::same_as<left_bound_type, Unbounded<LeftT>> &&
                std::same_as<right_bound_type, Unbounded<RightT>>) {
    return ret_type::equivalent;
  } else if constexpr (std::same_as<left_bound_type, Unbounded<LeftT>>) {
    return ret_type::less;
  } else if constexpr (std::same_as<right_bound_type, Unbounded<RightT>>) {
    return ret_type::greater;
  } else {
    ret_type ret{left.bound_.value_ <=> right.bound_.value_};
    if constexpr (std::same_as<left_bound_type, right_bound_type>) {
      return ret;
    } else {
      if (std::is_eq(ret)) {
        if constexpr (std::same_as<left_bound_type, Open_bound<LeftT>> &&
                      std::same_as<right_bound_type, Closed_bound<RightT>>) {
          return ret_type::greater;
        } else {
          static_assert(std::same_as<left_bound_type, Closed_bound<LeftT>> &&
                            std::same_as<right_bound_type, Open_bound<RightT>>,
                        u8"Non-exhaustive");
          return ret_type::less;
        }
      }
      return ret;
    }
  }
}
template <std::three_way_comparable LeftT,
          std::three_way_comparable_with<LeftT> RightT,
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

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  BoundT bound_;
  explicit constexpr Right_bound(BoundT bound) noexcept(
      noexcept(decltype(bound_){std::move(bound)}))
      : bound_{std::move(bound)} {}
  template <Guard_special_constructors<Right_bound> Other>
  requires std::same_as<
      std::remove_cvref_t<Other>,
      Right_bound<typename std::remove_cvref_t<Other>::bound_type>> &&
      Brace_convertible_to<typename std::remove_cvref_t<Other>::bound_type,
                           BoundT>
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
  explicit constexpr Right_bound(Other &&other) noexcept(noexcept(Right_bound{
      BoundT{std::forward<Other>(other).bound_}}))
      : Right_bound{BoundT{std::forward<Other>(other).bound_}} {}

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
template <std::three_way_comparable LeftT,
          std::three_way_comparable_with<LeftT> RightT,
          template <typename> typename LeftBoundT,
          template <typename> typename RightBoundT>
constexpr auto
operator<=>(Right_bound<LeftBoundT<LeftT>> const &left,
            Right_bound<RightBoundT<RightT>> const
                &right) noexcept(noexcept(std::declval<LeftT>() <=>
                                          std::declval<RightT>())) {
  using ret_type = std::compare_three_way_result_t<LeftT, RightT>;
  using left_bound_type = LeftBoundT<LeftT>;
  using right_bound_type = RightBoundT<RightT>;
  if constexpr (std::same_as<left_bound_type, Unbounded<LeftT>> &&
                std::same_as<right_bound_type, Unbounded<RightT>>) {
    return ret_type::equivalent;
  } else if constexpr (std::same_as<left_bound_type, Unbounded<LeftT>>) {
    return ret_type::greater;
  } else if constexpr (std::same_as<right_bound_type, Unbounded<RightT>>) {
    return ret_type::less;
  } else {
    ret_type ret{left.bound_.value_ <=> right.bound_.value_};
    if constexpr (std::same_as<left_bound_type, right_bound_type>) {
      return ret;
    } else {
      if (std::is_eq(ret)) {
        if constexpr (std::same_as<left_bound_type, Open_bound<LeftT>> &&
                      std::same_as<right_bound_type, Closed_bound<RightT>>) {
          return ret_type::less;
        } else {
          static_assert(std::same_as<left_bound_type, Closed_bound<LeftT>> &&
                            std::same_as<right_bound_type, Open_bound<RightT>>,
                        u8"Non-exhaustive");
          return ret_type::greater;
        }
      }
      return ret;
    }
  }
}
template <std::three_way_comparable LeftT,
          std::three_way_comparable_with<LeftT> RightT,
          template <typename> typename LeftBoundT,
          template <typename> typename RightBoundT>
constexpr auto operator==(Right_bound<LeftBoundT<LeftT>> const &left,
                          Right_bound<RightBoundT<RightT>> const
                              &right) noexcept(noexcept(left <=> right)) {
  return std::is_eq(left <=> right);
}

template <Bound_c LeftBoundT, Bound_c RightBoundT>
requires std::same_as<typename LeftBoundT::type, typename RightBoundT::type>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
class Interval {
#pragma clang diagnostic pop
public:
  using left_bound_type = LeftBoundT;
  using right_bound_type = RightBoundT;
  using validate_type = typename LeftBoundT::type;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  LeftBoundT left_;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  RightBoundT right_;
  explicit constexpr Interval(LeftBoundT left, RightBoundT right) noexcept(
      noexcept(decltype(left_){std::move(left)}, void(),
               decltype(right_){std::move(right)}))
      : left_{std::move(left)}, right_{std::move(right)} {}
  template <Guard_special_constructors<Interval> Other>
  requires Interval_c<std::remove_cvref_t<Other>> &&
      Brace_convertible_to<typename std::remove_cvref_t<Other>::left_bound_type,
                           LeftBoundT> &&
      Brace_convertible_to<
          typename std::remove_cvref_t<Other>::right_bound_type, RightBoundT>
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
  explicit constexpr Interval(Other &&other) noexcept(noexcept(Interval{
      LeftBoundT{std::forward<Other>(other).left_},
      RightBoundT{std::forward<Other>(other).right_}}))
      : Interval{LeftBoundT{std::forward<Other>(other).left_},
                 RightBoundT{std::forward<Other>(other).right_}} {}

  constexpr void validate(validate_type const &value [[maybe_unused]]) const
      noexcept(noexcept(left_ < value && u8"value is outside the lower bound",
                        value < right_ &&
                            u8"value is outside the upper bound")) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(left_ < value && u8"value is outside the lower bound");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(value < right_ && u8"value is outside the upper bound");
  }
  template <typename Other>
  requires Interval_c<std::remove_cvref_t<Other>>
  constexpr auto subsumes [[nodiscard]] (
      Other &&other) const noexcept(noexcept(bool{
      Brace_convertible_to<typename std::remove_cvref_t<Other>::validate_type,
                           validate_type> &&
      Left_bound{left_} <= Left_bound{other.left_} &&
      Right_bound{other.right_} <= Right_bound{right_}})) -> bool {
    return Brace_convertible_to<
               typename std::remove_cvref_t<Other>::validate_type,
               validate_type> &&
           Left_bound{left_} <= Left_bound{other.left_} &&
           Right_bound{other.right_} <= Right_bound{right_};
  }
#pragma warning(suppress : 4820)
};
static_assert(
    Validator_c<decltype(Interval{Unbounded<bool>{}, Unbounded<bool>{}})>,
    u8"Implementation error");
} // namespace artccel::core::util

#endif
