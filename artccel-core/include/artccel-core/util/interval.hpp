#pragma once

#include <assert.h>
#include <concepts>
#include <inttypes.h>

namespace artccel::core {
template <typename Type> constexpr Type unbounded{};

enum class bound : uint8_t {
  OPEN,
  CLOSED,
  UNBOUNDED,
};

template <bound Bound, std::totally_ordered auto Left,
          std::totally_ordered auto Right>
consteval bool bound_less_than() {
  return (Bound == bound::OPEN && Left < Right) ||
         (Bound == bound::CLOSED && Left <= Right) || Bound == bound::UNBOUNDED;
}

inline bool bound_less_than(bound bound, std::totally_ordered auto const &left,
                            std::totally_ordered auto const &right) {
  return (bound == bound::OPEN && left < right) ||
         (bound == bound::CLOSED && left <= right) || bound == bound::UNBOUNDED;
}

template <std::totally_ordered Type, bound LeftBound, Type Left, Type Right,
          bound RightBound>
struct interval {
  interval(Type const &value) : interval(value) {}
  interval(Type &&value) : value(value) {
    assert(("left >(=) value", bound_less_than(LeftBound, Left, this->value)));
    assert(
        ("value >(=) right", bound_less_than(RightBound, this->value, Right)));
  }
  template <Type Value> static auto check() {
    static_assert(bound_less_than<LeftBound, Left, Value>(), "left >(=) value");
    static_assert(bound_less_than<RightBound, Value, Right>(),
                  "value >(=) right");
    return interval(Value, false);
  }
  inline operator Type() { return value; }
  inline operator Type const() const { return value; }

private:
  Type value;
  interval(Type &&value, bool) : value(value) {}
};

template <std::totally_ordered Type>
using nonnegative_interval =
    interval<Type, bound::CLOSED, Type{0}, unbounded<Type>, bound::UNBOUNDED>;
template <std::totally_ordered Type>
using nonpositive_interval =
    interval<Type, bound::UNBOUNDED, unbounded<Type>, Type{0}, bound::CLOSED>;
template <std::totally_ordered Type>
using positive_interval =
    interval<Type, bound::OPEN, Type{0}, unbounded<Type>, bound::UNBOUNDED>;
template <std::totally_ordered Type>
using negative_interval =
    interval<Type, bound::UNBOUNDED, unbounded<Type>, Type{0}, bound::OPEN>;
} // namespace artccel::core
