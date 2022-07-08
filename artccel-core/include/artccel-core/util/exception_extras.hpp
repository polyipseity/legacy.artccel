#ifndef ARTCCEL_CORE_UTIL_EXCEPTION_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_EXCEPTION_EXTRAS_HPP
#pragma once

#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT
#include <concepts>              // import std::derived_from
#include <cstddef>               // import std::nullptr_t
#include <exception> // import std::current_exception, std::exception, std::exception_ptr, std::throw_with_nested
#include <type_traits> // import std::remove_reference_t
#include <utility>     // import std::forward

namespace artccel::core::util {
class ARTCCEL_CORE_EXPORT Rethrow_on_lexical_scope_exit;

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

class Rethrow_on_lexical_scope_exit {
private:
#pragma warning(push)
#pragma warning(disable : 4251)
  std::exception_ptr exc_;
#pragma warning(pop)

public:
  // functions are lvalue-only as this class only makes sense as a lvalue
  explicit Rethrow_on_lexical_scope_exit() noexcept;
  explicit Rethrow_on_lexical_scope_exit(
      [[maybe_unused]] std::nullptr_t /*unused*/) noexcept;
  explicit Rethrow_on_lexical_scope_exit(std::exception_ptr exc) noexcept;
  ~Rethrow_on_lexical_scope_exit() noexcept(false);

  // named 'write' to make 'rolse = std::move(rolse2.write())' look weird
  auto write [[nodiscard]] () &noexcept -> std::exception_ptr &;
  auto read [[nodiscard]] () const &noexcept -> std::exception_ptr const &;
  // "proper" (try rewriting your code instead) way: 'rolse = rolse2.release()'
  auto release [[nodiscard]] () & -> std::exception_ptr;

  auto operator=(std::nullptr_t right) &noexcept
      -> Rethrow_on_lexical_scope_exit &;
  auto operator=(std::exception_ptr const &right) &noexcept
      -> Rethrow_on_lexical_scope_exit &;
  auto operator=(std::exception_ptr &&right) &noexcept
      -> Rethrow_on_lexical_scope_exit &;

  // delete all to avoid double rethrowing
  Rethrow_on_lexical_scope_exit(Rethrow_on_lexical_scope_exit const &) = delete;
  auto operator=(Rethrow_on_lexical_scope_exit const &) = delete;
  Rethrow_on_lexical_scope_exit(Rethrow_on_lexical_scope_exit &&) = delete;
  auto operator=(Rethrow_on_lexical_scope_exit &&) = delete;
};
} // namespace artccel::core::util

#endif
