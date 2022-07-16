#ifndef ARTCCEL_CORE_UTIL_EXCEPTION_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_EXCEPTION_EXTRAS_HPP
#pragma once

#include <concepts> // import std::derived_from, std::invocable
#include <exception> // import std::current_exception, std::exception, std::throw_with_nested
#include <functional>  // import std::invoke
#include <type_traits> // import std::remove_cvref_t
#include <utility>     // import std::forward

#pragma warning(push)
#pragma warning(disable : 4626 4820)
#include <gsl/gsl> // import gsl::strict_not_null
#pragma warning(pop)

namespace artccel::core::util {
namespace detail {
template <bool Rethrow, typename Top, typename... Nested>
requires(std::derived_from<std::remove_cvref_t<Top>, std::exception> &&...
             &&std::derived_from<std::remove_cvref_t<Nested>,
                                 std::exception>) auto throw_multiple_as_nested
    [[noreturn]] (Top &&top, Nested &&...nested) -> std::remove_cvref_t<Top> {
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
void ignore_all_exceptions(std::invocable<> auto &&func) noexcept {
  try {
    std::invoke(func);
  } catch (...) {
    // NOOP
  }
}

template <typename Top, typename... Nested>
requires(std::derived_from<std::remove_cvref_t<Top>, std::exception> &&...
             &&std::derived_from<std::remove_cvref_t<Nested>,
                                 std::exception>) void throw_multiple_as_nested
    [[noreturn]] (Top &&top, Nested &&...nested) {
  detail::throw_multiple_as_nested<false>(std::forward<Top>(top),
                                          std::forward<Nested>(nested)...);
}
template <typename Top, typename... Nested>
requires(std::derived_from<std::remove_cvref_t<Top>, std::exception> &&... &&
             std::derived_from<std::remove_cvref_t<Nested>,
                               std::exception>) void rethrow_multiple_as_nested
    [[noreturn]] (Top &&top, Nested &&...nested) {
  detail::throw_multiple_as_nested<true>(std::forward<Top>(top),
                                         std::forward<Nested>(nested)...);
}
template <typename Top, typename... Nested>
requires(
    std::derived_from<std::remove_cvref_t<Top>, std::exception> &&...
        &&std::derived_from<
            std::remove_cvref_t<Nested>,
            std::exception>) auto make_nested_exception(Top &&top,
                                                        Nested &&...nested) {
  try {
    throw_multiple_as_nested(std::forward<Top>(top),
                             std::forward<Nested>(nested)...);
  } catch (Top const &) {
    return gsl::strict_not_null{std::current_exception()};
  }
}
template <typename Top, typename... Nested>
requires(
    std::derived_from<std::remove_cvref_t<Top>, std::exception> &&...
        &&std::derived_from<
            std::remove_cvref_t<Nested>,
            std::exception>) auto remake_nested_exception(Top &&top,
                                                          Nested &&...nested) {
  try {
    rethrow_multiple_as_nested(std::forward<Top>(top),
                               std::forward<Nested>(nested)...);
  } catch (Top const &) {
    return gsl::strict_not_null{std::current_exception()};
  }
}
} // namespace f
} // namespace artccel::core::util

#endif
