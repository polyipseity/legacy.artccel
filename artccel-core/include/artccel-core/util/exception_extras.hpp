#ifndef ARTCCEL_CORE_UTIL_EXCEPTION_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_EXCEPTION_EXTRAS_HPP
#pragma once

#include "polyfill.hpp"          // import literals::operator""_UZ
#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT
#include <concepts>              // import std::derived_from
#include <cstddef>               // import std::nullptr_t
#include <exception> // import std::current_exception, std::exception, std::exception_ptr, std::throw_with_nested
#include <type_traits> // import std::remove_reference_t
#include <utility>     // import std::forward

namespace artccel::core::util {
class ARTCCEL_CORE_EXPORT Rethrow_on_destruct;

namespace detail {
using literals::operator""_UZ;

template <bool Rethrow, typename Top, typename... Nested>
requires(std::derived_from<std::remove_reference_t<Top>, std::exception> &&...
             &&std::derived_from<std::remove_reference_t<Nested>,
                                 std::exception>) auto throw_multiple_as_nested
    [[noreturn]] (Top &&top, Nested &&...nested)
    -> std::remove_reference_t<Top> {
  if constexpr (sizeof...(Nested) == 0_UZ) {
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

class Rethrow_on_destruct {
private:
  std::exception_ptr exc_;

public:
  explicit Rethrow_on_destruct() noexcept;
  explicit Rethrow_on_destruct(
      [[maybe_unused]] std::nullptr_t /*unused*/) noexcept;
  explicit Rethrow_on_destruct(std::exception_ptr exc) noexcept;

  auto ptr [[nodiscard]] () &noexcept -> std::exception_ptr &;
  auto ptr [[nodiscard]] () const &noexcept -> std::exception_ptr const &;
  auto ptr [[nodiscard]] () &&noexcept -> std::exception_ptr;
  auto ptr [[nodiscard]] () const && -> std::exception_ptr;

  Rethrow_on_destruct(Rethrow_on_destruct const &) = delete;
  auto operator=(Rethrow_on_destruct const &) = delete;
  Rethrow_on_destruct(Rethrow_on_destruct &&other) noexcept;
  auto operator=(Rethrow_on_destruct &&right) noexcept -> Rethrow_on_destruct &;
  ~Rethrow_on_destruct() noexcept(false);
};
} // namespace artccel::core::util

#endif
