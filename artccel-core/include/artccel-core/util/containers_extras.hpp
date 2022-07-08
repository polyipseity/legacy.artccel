#ifndef ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#pragma once

#include <array>            // import std::array, std::data, std::size
#include <cstddef>          // import std::size_t
#include <initializer_list> // import std::initializer_list
#include <span>             // import std::dynamic_extent, std::span
#include <utility> // import std::forward, std::index_sequence, std::make_index_sequence, std::move

namespace artccel::core::util {
template <typename> struct array_size;
template <typename Type> constexpr auto array_size_v{array_size<Type>::value};

namespace f {
template <typename Container>
constexpr auto atad [[nodiscard]] (Container &container) -> decltype(auto) {
  return std::data(container) + std::size(container);
}
template <typename Container>
constexpr auto atad [[nodiscard]] (const Container &container)
-> decltype(auto) {
  return std::data(container) + std::size(container);
}
template <class Element, std::size_t Size>
constexpr auto atad [[nodiscard]] (Element (&array)[Size]) noexcept {
  return array + Size;
}
template <class Element>
constexpr auto atad
    [[nodiscard]] (std::initializer_list<Element> init_list) noexcept {
  return init_list.end();
}

template <typename... Args>
constexpr auto const_span
    [[nodiscard]] (Args &&...args) noexcept(noexcept(std::span{
        std::forward<Args>(args)...})) {
  using span_type = decltype(std::span{std::forward<Args>(args)...});
  return std::span<typename span_type::element_type const, span_type::extent>{
      std::forward<Args>(args)...};
}
template <typename Type, std::size_t Size>
constexpr auto const_array [[nodiscard]] (std::array<Type, Size> const &array) {
  return [&array]<std::size_t... Idx>(
      [[maybe_unused]] std::index_sequence<Idx...> /*unused*/) {
    return std::array<Type const, Size>{{array[Idx]...}};
  }
  (std::make_index_sequence<Size>{});
}
template <typename Type, std::size_t Size>
constexpr auto const_array [[nodiscard]] (std::array<Type, Size> &&array) {
  return [&array]<std::size_t... Idx>(
      [[maybe_unused]] std::index_sequence<Idx...> /*unused*/) {
    return std::array<Type const, Size>{{std::move(array[Idx])...}};
  }
  (std::make_index_sequence<Size>{});
}
template <typename... Args>
constexpr auto const_array
    [[nodiscard]] (Args &&...args) noexcept(noexcept(std::array{
        std::forward<Args>(args)...})) {
  using array_type = decltype(std::array{std::forward<Args>(args)...});
  return std::array<typename array_type::value_type const,
                    array_size_v<array_type>>{std::forward<Args>(args)...};
}

template <typename Type, std::size_t Size>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr auto to_array_cv(Type (&array)[Size]) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  return [&array]<std::size_t... Idx>(
      [[maybe_unused]] std::index_sequence<Idx...> /*unused*/) {
    return std::array<Type, Size>{{array[Idx]...}};
  }
  (std::make_index_sequence<Size>{});
}
template <typename Type, std::size_t Size>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr auto to_array_cv(Type (&&array)[Size]) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  return [&array]<std::size_t... Idx>(
      [[maybe_unused]] std::index_sequence<Idx...> /*unused*/) {
    return std::array<Type, Size>{{std::move(array[Idx])...}};
  }
  (std::make_index_sequence<Size>{});
}
template <typename Type, std::size_t Size>
requires(Size != std::dynamic_extent) constexpr auto to_array_cv(
    std::span<Type, Size> span) {
  return [span]<std::size_t... Idx>(
      [[maybe_unused]] std::index_sequence<Idx...> /*unused*/) {
    return std::array<Type, Size>{{span[Idx]...}};
  }
  (std::make_index_sequence<Size>{});
}
} // namespace f

template <typename Type, std::size_t Size>
struct array_size<std::array<Type, Size>> {
  constexpr static auto value{Size};
};
} // namespace artccel::core::util

#endif
