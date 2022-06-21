#ifndef ARTCCEL_CORE_UTIL_UTILITY_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_UTILITY_EXTRAS_HPP
#pragma once

#include <cassert>     // import assert
#include <concepts>    // import std::invocable
#include <cstddef>     // import std::size_t
#include <functional>  // import std::invoke
#include <type_traits> // import std::invoke_result_t, std::is_nothrow_invocable_v, std::is_nothrow_move_constructible_v, std::is_pointer_v, std::is_reference_v, std::remove_reference_t
#include <utility> // import std::forward, std::index_sequence, std::index_sequence_for

namespace artccel::core::util {
template <typename> constexpr inline auto dependent_false_v{false};

template <typename T> constexpr auto unify_ref_to_ptr(T &&value) noexcept {
  // (callsite) -> (return)
  if constexpr (std::is_reference_v<T>) {
    // t& -> t*, t*& -> t**
    return &value;
  } else {
    // t -> t, t&& -> t, t* -> t*, t*&& -> t*
    return std::forward<T>(value);
  }
}
template <typename T>
constexpr auto unify_ptr_to_ref(T &&value) noexcept -> decltype(auto) {
  // (callsite) -> (return)
  if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) {
    // t* -> t&, t*& -> t&, t*&& -> t&
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(value && u8"value == nullptr");
    return *value; // *v is t&
  } else {
    // t -> t, t& -> t&, t&& -> t
    return T{std::forward<T>(value)};
  }
}

template <typename F, template <typename...> typename Tuple, typename... Args>
requires std::invocable<F, Args...>
constexpr auto forward_apply(F &&func, Tuple<Args...> &&t_args) noexcept(
    std::is_nothrow_invocable_v<F, Args...> &&
        std::is_nothrow_move_constructible_v<std::invoke_result_t<F, Args...>>)
    -> decltype(auto) {
  using TArgs = Tuple<Args...>;
  return
      [&func, &t_args ]<std::size_t... I>(
          [[maybe_unused]] std::index_sequence<
              I...> /*unused*/) mutable noexcept(std::
                                                     is_nothrow_invocable_v<
                                                         F, Args...> &&
                                                 std::
                                                     is_nothrow_move_constructible_v<
                                                         std::invoke_result_t<
                                                             F, Args...>>)
          ->decltype(auto) {
    return std::invoke(
        std::forward<F>(func),
        std::forward<Args>(std::get<I>(std::forward<TArgs>(t_args)))...);
  }
  (std::index_sequence_for<Args...>{});
}
} // namespace artccel::core::util

#endif
