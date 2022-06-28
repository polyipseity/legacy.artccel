#ifndef ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#pragma once

#include <array>   // import std::array
#include <cstddef> // import std::size_t
#include <span>    // import std::span
#include <utility> // import std::forward, std::index_sequence, std::make_index_sequence, std::move

namespace artccel::core::util::f {
template <typename... Args>
constexpr auto const_span
    [[nodiscard]] (Args &&...args) noexcept(noexcept(std::span{
        std::forward<Args>(args)...})) {
  using span_type = decltype(std::span{std::forward<Args>(args)...});
  return std::span<typename span_type::element_type const, span_type::extent>{
      std::forward<Args>(args)...};
}

template <typename T, std::size_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr auto to_array_cv(T (&array)[N]) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  return [&array]<std::size_t... I>(
      [[maybe_unused]] std::index_sequence<I...> /*unused*/) {
    return std::array<T, N>{{array[I]...}};
  }
  (std::make_index_sequence<N>{});
}
template <typename T, std::size_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr auto to_array_cv(T (&&array)[N]) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  return [&array]<std::size_t... I>(
      [[maybe_unused]] std::index_sequence<I...> /*unused*/) {
    return std::array<T, N>{{std::move(array[I])...}};
  }
  (std::make_index_sequence<N>{});
}
} // namespace artccel::core::util::f

#endif
