#ifndef ARTCCEL_CORE_UTIL_CLONE_HPP
#define ARTCCEL_CORE_UTIL_CLONE_HPP
#pragma once

#include "encoding.hpp" // import c8srtombs
#include "meta.hpp"     // import Remove_cvptr_t, Replace_ptr_value_type_t
#include <cassert>      // import assert
#include <concepts>     // import std::convertible_to, std::derived_from
#include <exception>    // import std::invalid_argument
#include <memory> // import std::enable_shared_from_this, std::shared_ptr, std::unique_ptr
#include <type_traits> // import std::is_pointer_v, std::is_reference_v, std::remove_reference_t
#include <utility> // import std::declval, std::forward

namespace artccel::core::util {
template <typename P>
concept cloneable = requires(Remove_cvptr_t<P> const &ptr) {
  requires std::convertible_to < Remove_cvptr_t<P>
  &, Remove_cvptr_t<decltype(ptr.clone())> & > ;
};

namespace detail {
template <cloneable P>
constexpr auto clone_raw [[deprecated, nodiscard]] (P &&ptr) -> auto & {
  if constexpr (std::is_reference_v<P>) {
    using return_type = decltype(ptr.clone());
    if constexpr (std::is_pointer_v<return_type>) {
      return *(ptr.clone());
    } else if constexpr (std::is_reference_v<return_type>) {
      return ptr.clone();
    } else {
      return ptr.clone().release();
    }
  } else {
    // clang-format off
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(ptr && u8"ptr == nullptr");
    using return_type = decltype(ptr->clone());
    if constexpr (std::is_pointer_v<return_type>) {
      return *(ptr->clone());
    } else if constexpr (std::is_reference_v<return_type>) {
      return ptr->clone();
    } else {
      return ptr->clone().release();
    }
  }
}
} // namespace detail

template <
    cloneable P, typename RP = P,
    typename R = Replace_ptr_value_type_t<
        RP, std::remove_reference_t<
                // NOLINTNEXTLINE(clang-diagnostic-deprecated-declarations)
                decltype(detail::clone_raw(std::declval<P>()))>>>
requires(!std::derived_from<Remove_cvptr_t<P>,
                            std::enable_shared_from_this<Remove_cvptr_t<P>>> ||
         std::derived_from<Replace_ptr_value_type_t<RP, void>,
                           std::shared_ptr<void>>) constexpr auto clone
    [[deprecated(/*u8*/ "Unsafe"), nodiscard]] (P &&ptr) -> R {
  if constexpr (!std::is_reference_v<P>) {
    if (!ptr) {
      if constexpr (std::is_pointer_v<R>) {
        return nullptr;
      } else if constexpr (std::is_reference_v<R>) {
        throw std::invalid_argument{c8srtombs(u8"ptr == nullptr")};
      } else {
        return R{};
      }
    }
  }
  if constexpr (std::is_pointer_v<R>) {
    return &detail::clone_raw(std::forward<P>(ptr));
  } else if constexpr (std::is_reference_v<R>) {
    return detail::clone_raw(std::forward<P>(ptr));
  } else {
    return R{&detail::clone_raw(std::forward<P>(ptr))};
  }
}
template <cloneable P> auto clone_unique [[nodiscard]] (P &&ptr) {
  // NOLINTNEXTLINE(clang-diagnostic-deprecated-declarations)
  return clone<P, std::unique_ptr<void>>(std::forward<P>(ptr));
}
template <cloneable P> auto clone_shared [[nodiscard]] (P &&ptr) {
  // NOLINTNEXTLINE(clang-diagnostic-deprecated-declarations)
  return clone<P, std::shared_ptr<void>>(std::forward<P>(ptr));
}
} // namespace artccel::core::util

#endif
