#ifndef GUARD_4D02DFF6_D796_408D_96E0_25F301394495
#define GUARD_4D02DFF6_D796_408D_96E0_25F301394495
#pragma once

#include <algorithm>   // import std::clamp
#include <concepts>    // import std::integral
#include <cstdint>     // import std::intmax_t, std::uintmax_t
#include <functional>  // import std::invoke
#include <limits>      // import std::numeric_limits
#include <stdexcept>   // import std::overflow_error
#include <string>      // import std::to_string
#include <tuple>       // import std::tuple
#include <type_traits> // import std::conditional_t, std::make_signed_t, std::make_unsigned_t, std::remove_cvref_t

#pragma warning(push)
#pragma warning(disable : 4582 4583 4625 4626 4820 5026 5027)
#include <tl/expected.hpp> // import tl::expected, tl::unexpect
#pragma warning(pop)

#include "concepts_extras.hpp" // import Nonempty_pack, Regular_invocable_r
#include "error_handling.hpp"  // import Error_with_exception

namespace artccel::core::util {
namespace detail {
template <std::integral Int, typename Ret, auto ExactFunc, auto PosOverflowFunc,
          auto NegOverflowFunc, std::integral InInt>
requires Regular_invocable_r<decltype(ExactFunc), Ret,
                             std::remove_cvref_t<InInt>> &&
    Regular_invocable_r<decltype(PosOverflowFunc), Ret,
                        std::remove_cvref_t<InInt>> &&
    Regular_invocable_r<decltype(NegOverflowFunc), Ret,
                        std::remove_cvref_t<InInt>>
constexpr auto int_cast(InInt value) noexcept(
    noexcept(Ret{std::invoke(ExactFunc, value)}, void(),
             Ret{std::invoke(PosOverflowFunc, value)}, void(),
             Ret{std::invoke(NegOverflowFunc, value)})) -> Ret {
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
constexpr auto int_strict_equal(std::integral auto left,
                                std::integral auto right) noexcept {
  using left_limits = std::numeric_limits<decltype(left)>;
  using right_limits = std::numeric_limits<decltype(right)>;
  if constexpr (left_limits::is_signed != right_limits::is_signed) {
    if constexpr (left_limits::is_signed) {
      if (left < 0) {
        return false;
      }
    } else {
      if (right < 0) {
        return false;
      }
    }
    return left == right;
  }
}
template <std::integral Int>
constexpr auto int_modulo_cast(std::integral auto value) noexcept {
  using out_type = std::remove_cvref_t<Int>;
  constexpr auto exact_and_overflow{[](std::integral auto val) noexcept {
    // value % 2^bits, non-implementation defined for signed since C++20
    return static_cast<out_type>(val);
  }};
  return detail::int_cast<Int, out_type, exact_and_overflow, exact_and_overflow,
                          exact_and_overflow>(value);
}
template <std::integral Int>
constexpr auto int_exact_cast(std::integral auto value) noexcept {
  using out_int_type = std::remove_cvref_t<Int>;
  using out_type =
      tl::expected<out_int_type,
                   Error_with_exception<std::remove_cvref_t<decltype(value)>>>;
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
  using out_type = std::remove_cvref_t<Int>;
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

namespace detail {
template <std::integral Int, std::integral... Ints>
constexpr auto int_roundrobin_clamp_cast(std::integral auto value) noexcept {
  if constexpr (sizeof...(Ints) == 0) {
    return f::int_clamp_cast<Int>(value);
  } else {
    return f::int_clamp_cast<Int>(
        detail::int_roundrobin_clamp_cast<Ints...>(value));
  }
}
} // namespace detail

template <std::integral Int, std::integral... Ints>
constexpr auto ints_minimax{[]() noexcept {
  using first_limits = std::numeric_limits<Int>;
  using ret_type = std::conditional_t<(first_limits::is_signed && ... &&
                                       std::numeric_limits<Ints>::is_signed),
                                      std::intmax_t, std::uintmax_t>;
  return ret_type{
      detail::int_roundrobin_clamp_cast<Int, Ints...>(first_limits::max())};
}()};
template <std::integral Int, std::integral... Ints>
constexpr auto ints_maximin{[]() noexcept {
  using first_limits = std::numeric_limits<Int>;
  using ret_type = std::conditional_t<(first_limits::is_signed && ... &&
                                       std::numeric_limits<Ints>::is_signed),
                                      std::intmax_t, std::uintmax_t>;
  return ret_type{
      detail::int_roundrobin_clamp_cast<Int, Ints...>(first_limits::lowest())};
}()};

namespace f {
template <std::integral... Ints>
requires Nonempty_type_pack<Ints...>
constexpr auto int_clamp_casts(std::integral auto value) noexcept {
  using out_types = std::tuple<std::remove_cvref_t<Ints>...>;
  auto const ret{std::clamp<decltype(value)>(value, ints_maximin<Ints...>,
                                             ints_minimax<Ints...>)};
  return out_types{assert_success(f::int_exact_cast<Ints>(ret))...};
}
} // namespace f
} // namespace artccel::core::util

#endif
