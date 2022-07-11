#ifndef ARTCCEL_CORE_UTIL_CONVERSIONS_HPP
#define ARTCCEL_CORE_UTIL_CONVERSIONS_HPP
#pragma once

#include "concepts_extras.hpp" // import Regular_invocable_r
#include "error_handling.hpp"  // import Error_with_exception
#include <concepts>            // import std::integral
#include <functional>          // import std::invoke
#include <limits>              // import std::numeric_limits
#include <stdexcept>           // import std::overflow_error
#include <string>              // import std::to_string
#include <tl/expected.hpp>     // import tl::expected, tl::unexpect
#include <type_traits> // import std::conditional_t, std::is_nothrow_move_constructible_v, std::make_signed_t, std::make_unsigned_t, std::remove_cv_t

namespace artccel::core::util {
namespace detail {
template <std::integral Int, typename Ret, auto ExactFunc, auto PosOverflowFunc,
          auto NegOverflowFunc, std::integral InInt>
requires Regular_invocable_r<decltype(ExactFunc), Ret,
                             std::remove_cv_t<InInt>> &&
    Regular_invocable_r<decltype(PosOverflowFunc), Ret,
                        std::remove_cv_t<InInt>> &&
    Regular_invocable_r<decltype(NegOverflowFunc), Ret, std::remove_cv_t<InInt>>
constexpr auto int_cast(InInt value) noexcept(
    noexcept(Ret{std::invoke(ExactFunc, value)}, void(),
             Ret{std::invoke(PosOverflowFunc, value)}, void(),
             Ret{std::invoke(NegOverflowFunc, value)}) &&
    std::is_nothrow_move_constructible_v<Ret>) -> Ret {
  using in_limits = std::numeric_limits<InInt>;
  using out_limits = std::numeric_limits<Int>;
  if constexpr (in_limits::max() > out_limits::max()) {
#pragma warning(suppress : 4018)
    if (value > out_limits::max()) {
      return std::invoke(PosOverflowFunc, value);
    }
  }
  if constexpr (in_limits::is_signed) {
    if constexpr (!out_limits::is_signed) {
      static_assert(out_limits::lowest() == 0,
                    u8"Unsigned integral type with non-zero lowest value???");
      if (value < 0) {
        return std::invoke(NegOverflowFunc, value);
      }
    } else if constexpr (in_limits::lowest() < out_limits::lowest()) {
      if (value < out_limits::lowest()) {
        return std::invoke(NegOverflowFunc, value);
      }
    }
  }
  return std::invoke(ExactFunc, value);
}
} // namespace detail

namespace f {
template <std::integral Int>
constexpr auto int_modulo_cast(std::integral auto value) noexcept {
  using out_type = std::remove_cv_t<Int>;
  constexpr auto exact_and_overflow{[](std::integral auto val) noexcept {
    // value % 2^bits, non-implementation defined for signed since C++20
    return static_cast<out_type>(val);
  }};
  return detail::int_cast<Int, out_type, exact_and_overflow, exact_and_overflow,
                          exact_and_overflow>(value);
}
template <std::integral Int>
constexpr auto int_exact_cast(std::integral auto value) noexcept {
  using out_int_type = std::remove_cv_t<Int>;
  using out_type =
      tl::expected<out_int_type,
                   Error_with_exception<std::remove_cv_t<decltype(value)>>>;
  constexpr auto exact{[](std::integral auto val) noexcept {
    return out_type{static_cast<out_int_type>(val)};
  }};
  constexpr auto overflow{[](std::integral auto val) noexcept {
    return out_type{tl::unexpect,
                    typename out_type::error_type{
                        std::overflow_error{std::to_string(val)},
                        val}}; // TODO: C++23: tl::in_place is broken
  }};
  return detail::int_cast<Int, out_type, exact, overflow, overflow>(value);
}
template <std::integral Int>
constexpr auto int_clamp_cast(std::integral auto value) noexcept {
  using out_type = std::remove_cv_t<Int>;
  using out_limits = std::numeric_limits<out_type>;
  return detail::int_cast<Int, out_type,
                          [](std::integral auto val) noexcept {
                            return static_cast<out_type>(val);
                          },
                          [](std::integral auto val [[maybe_unused]]) noexcept {
                            return out_limits::max();
                          },
                          [](std::integral auto val [[maybe_unused]]) noexcept {
                            return out_limits::lowest();
                          }>(value);
}
template <bool Unsigned>
constexpr auto int_signedness_cast(std::integral auto value) noexcept {
  using out_type =
      std::conditional_t<Unsigned, std::make_unsigned_t<decltype(value)>,
                         std::make_signed_t<decltype(value)>>;
  return f::int_modulo_cast<out_type>(value);
}
constexpr auto int_unsigned_cast(std::integral auto value) noexcept {
  return f::int_signedness_cast<true>(value);
}
constexpr auto int_signed_cast(std::integral auto value) noexcept {
  return f::int_signedness_cast<false>(value);
}
template <bool Unsigned>
constexpr auto int_signedness_exact_cast(std::integral auto value) noexcept {
  using out_type =
      std::conditional_t<Unsigned, std::make_unsigned_t<decltype(value)>,
                         std::make_signed_t<decltype(value)>>;
  return f::int_exact_cast<out_type>(value);
}
constexpr auto int_unsigned_exact_cast(std::integral auto value) noexcept {
  return f::int_signedness_exact_cast<true>(value);
}
constexpr auto int_signed_exact_cast(std::integral auto value) noexcept {
  return f::int_signedness_exact_cast<false>(value);
}
template <bool Unsigned>
constexpr auto int_signedness_clamp_cast(std::integral auto value) noexcept {
  using out_type =
      std::conditional_t<Unsigned, std::make_unsigned_t<decltype(value)>,
                         std::make_signed_t<decltype(value)>>;
  return f::int_clamp_cast<out_type>(value);
}
constexpr auto int_unsigned_clamp_cast(std::integral auto value) noexcept {
  return f::int_signedness_clamp_cast<true>(value);
}
constexpr auto int_signed_clamp_cast(std::integral auto value) noexcept {
  return f::int_signedness_clamp_cast<false>(value);
}
} // namespace f
} // namespace artccel::core::util

#endif
