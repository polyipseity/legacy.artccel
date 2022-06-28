#ifndef ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#pragma once

#include <array>   // import std::array
#include <cstddef> // import std::size_t
#include <span>    // import std::dynamic_extent, std::span
#include <utility> // import std::forward, std::index_sequence, std::make_index_sequence, std::move

namespace artccel::core::util {
namespace detail {
template <typename> struct array_size;
template <typename T, std::size_t N>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct array_size<std::array<T, N>> {
  constexpr static auto value{N};
};
} // namespace detail

template <typename T>
constexpr inline auto array_size_v{detail::array_size<T>::value};

namespace f {
template <typename... Args>
constexpr auto const_span
    [[nodiscard]] (Args &&...args) noexcept(noexcept(std::span{
        std::forward<Args>(args)...})) {
  using span_type = decltype(std::span{std::forward<Args>(args)...});
  return std::span<typename span_type::element_type const, span_type::extent>{
      std::forward<Args>(args)...};
}
template <typename T, std::size_t N>
constexpr auto const_array [[nodiscard]] (std::array<T, N> const &array) {
  return [&array]<std::size_t... I>(
      [[maybe_unused]] std::index_sequence<I...> /*unused*/) {
    return std::array<T const, N>{{array[I]...}};
  }
  (std::make_index_sequence<N>{});
}
template <typename T, std::size_t N>
constexpr auto const_array [[nodiscard]] (std::array<T, N> &&array) {
  return [&array]<std::size_t... I>(
      [[maybe_unused]] std::index_sequence<I...> /*unused*/) {
    return std::array<T const, N>{{std::move(array[I])...}};
  }
  (std::make_index_sequence<N>{});
}
template <typename... Args>
constexpr auto const_array
    [[nodiscard]] (Args &&...args) noexcept(noexcept(std::array{
        std::forward<Args>(args)...})) {
  using array_type = decltype(std::array{std::forward<Args>(args)...});
  return std::array<typename array_type::value_type const,
                    array_size_v<array_type>>{std::forward<Args>(args)...};
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
template <typename T, std::size_t N>
requires(N !=
         std::dynamic_extent) constexpr auto to_array_cv(std::span<T, N> span) {
  return [span]<std::size_t... I>(
      [[maybe_unused]] std::index_sequence<I...> /*unused*/) {
    return std::array<T, N>{{span[I]...}};
  }
  (std::make_index_sequence<N>{});
}
} // namespace f
} // namespace artccel::core::util

#endif
