#ifndef ARTCCEL_CORE_UTIL_CLONE_HPP
#define ARTCCEL_CORE_UTIL_CLONE_HPP
#pragma once

#include "encoding.hpp" // import c8srtombs
#include "meta.hpp"     // import Remove_cvptr_t, Replace_ptr_value_type_t
#include <cassert>      // import assert
#include <concepts>     // import std::convertible_to, std::derived_from
#include <exception>    // import std::invalid_argument
#include <functional>   // import std::invoke
#include <memory> // import std::enable_shared_from_this, std::shared_ptr, std::unique_ptr
#include <type_traits> // import std::invoke_result_t, std::is_pointer_v, std::is_reference_v, std::remove_reference_t
#include <utility>     // import std::declval, std::forward

namespace artccel::core::util {
template <typename P, typename F>
concept cloneable = std::convertible_to < Remove_cvptr_t<P>
&, Remove_cvptr_t<std::invoke_result_t<F, Remove_cvptr_t<P> const &>> & > ;
template <typename P>
constexpr auto default_clone_function{[]() noexcept {
  constexpr struct {
    constexpr auto operator()
        [[deprecated, nodiscard]] (Remove_cvptr_t<P> const &ptr) const
        -> decltype(auto) {
      return ptr.clone();
    }
  } init{};
  return init;
}()};

namespace detail {
template <typename F, cloneable<F> P>
constexpr auto clone_raw [[deprecated, nodiscard]] (P &&ptr, F &&func)
-> auto & {
  using return_type = std::invoke_result_t<F, Remove_cvptr_t<P> const &>;
  if constexpr (std::is_reference_v<P>) {
    if constexpr (std::is_pointer_v<return_type>) {
      return *std::invoke(std::forward<F>(func), ptr);
    } else if constexpr (std::is_reference_v<return_type>) {
      return std::invoke(std::forward<F>(func), ptr);
    } else {
      return std::invoke(std::forward<F>(func), ptr).release();
    }
  } else {
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(ptr && u8"ptr == nullptr");
    if constexpr (std::is_pointer_v<return_type>) {
      return *std::invoke(std::forward<F>(func), *ptr);
    } else if constexpr (std::is_reference_v<return_type>) {
      return std::invoke(std::forward<F>(func), *ptr);
    } else {
      return std::invoke(std::forward<F>(func), *ptr).release();
    }
  }
}
} // namespace detail

template <
    typename F, cloneable<F> P, typename RP = P,
    typename R = Replace_ptr_value_type_t<
        RP,
        std::remove_reference_t<
            // NOLINTNEXTLINE(clang-diagnostic-deprecated-declarations)
            decltype(detail::clone_raw(std::declval<P>(), std::declval<F>()))>>>
requires(!std::derived_from<Remove_cvptr_t<P>,
                            std::enable_shared_from_this<Remove_cvptr_t<P>>> ||
         std::derived_from<Replace_ptr_value_type_t<RP, void>,
                           std::shared_ptr<void>>) constexpr auto clone
    [[deprecated(/*u8*/ "Unsafe"), nodiscard]] (P &&ptr, F &&func) -> R {
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
    return &detail::clone_raw(std::forward<P>(ptr), std::forward<F>(func));
  } else if constexpr (std::is_reference_v<R>) {
    return detail::clone_raw(std::forward<P>(ptr), std::forward<F>(func));
  } else {
    return R{&detail::clone_raw(std::forward<P>(ptr), std::forward<F>(func))};
  }
}
template <typename P>
requires cloneable<P, decltype(default_clone_function<P>)>
auto clone [[deprecated(/*u8*/ "Unsafe"), nodiscard]] (P &&ptr)
-> decltype(auto) {
  return clone(std::forward<P>(ptr), default_clone_function<P>);
}
template <typename F, cloneable<F> P>
auto clone_unique [[nodiscard]] (P &&ptr, F &&func) -> decltype(auto) {
  // NOLINTNEXTLINE(clang-diagnostic-deprecated-declarations)
  return clone<F, P, std::unique_ptr<void>>(std::forward<P>(ptr),
                                            std::forward<F>(func));
}
template <typename P>
requires cloneable<P, decltype(default_clone_function<P>)>
auto clone_unique [[nodiscard]] (P &&ptr) -> decltype(auto) {
  return clone_unique(std::forward<P>(ptr), default_clone_function<P>);
}
template <typename F, cloneable<F> P>
auto clone_shared [[nodiscard]] (P &&ptr, F &&func) -> decltype(auto) {
  // NOLINTNEXTLINE(clang-diagnostic-deprecated-declarations)
  return clone<F, P, std::shared_ptr<void>>(std::forward<P>(ptr),
                                            std::forward<F>(func));
}
template <typename P>
requires cloneable<P, decltype(default_clone_function<P>)>
auto clone_shared [[nodiscard]] (P &&ptr) -> decltype(auto) {
  return clone_shared(std::forward<P>(ptr), default_clone_function<P>);
}
} // namespace artccel::core::util

#endif
