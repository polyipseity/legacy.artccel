#ifndef ARTCCEL_CORE_UTIL_EXCEPTION_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_EXCEPTION_EXTRAS_HPP
#pragma once

#include <concepts> // import std::derived_from
#include <exception> // import std::current_exception, std::exception, std::throw_with_nested
#include <type_traits> // import std::remove_reference_t
#include <utility>     // import std::forward

namespace artccel::core::util {
namespace detail {
template <bool Rethrow, typename Top, typename... Nested>
requires(std::derived_from<std::remove_reference_t<Top>, std::exception> &&...
             &&std::derived_from<std::remove_reference_t<Nested>,
                                 std::exception>) auto throw_multiple_as_nested
    [[noreturn]] (Top &&top, Nested &&...nested)
    -> std::remove_reference_t<Top> {
  if constexpr (sizeof...(Nested) == 0) {
    if constexpr (Rethrow) {
      if (std::current_exception()) {
        std::throw_with_nested(std::forward<Top>(top));
      }
    }
    throw std::forward<Top>(top);
  } else {
    using thrown_type = decltype(throw_multiple_as_nested<Rethrow>(
        std::forward<Nested>(nested)...));
    try {
      throw_multiple_as_nested<Rethrow>(std::forward<Nested>(nested)...);
    } catch (thrown_type const &) {
      std::throw_with_nested(std::forward<Top>(top));
    }
  }
}
} // namespace detail

namespace f {
template <typename Top, typename... Nested>
requires(std::derived_from<std::remove_reference_t<Top>, std::exception> &&...
             &&std::derived_from<std::remove_reference_t<Nested>,
                                 std::exception>) void throw_multiple_as_nested
    [[noreturn]] (Top &&top, Nested &&...nested) {
  detail::throw_multiple_as_nested<false>(std::forward<Top>(top),
                                          std::forward<Nested>(nested)...);
}
template <typename Top, typename... Nested>
requires(
    std::derived_from<std::remove_reference_t<Top>, std::exception> &&...
        &&std::derived_from<std::remove_reference_t<Nested>,
                            std::exception>) void rethrow_multiple_as_nested
    [[noreturn]] (Top &&top, Nested &&...nested) {
  detail::throw_multiple_as_nested<true>(std::forward<Top>(top),
                                         std::forward<Nested>(nested)...);
}
} // namespace f
} // namespace artccel::core::util

#endif
