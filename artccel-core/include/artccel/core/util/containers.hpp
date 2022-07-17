#ifndef GUARD_F6ADCB05_F660_466C_A14C_7757725976FD
#define GUARD_F6ADCB05_F660_466C_A14C_7757725976FD
#pragma once

#include <array>            // import std::array, std::data, std::size
#include <concepts>         // import std::same_as
#include <cstddef>          // import std::size_t
#include <initializer_list> // import std::initializer_list
#include <span>             // import std::dynamic_extent, std::span
#include <type_traits> // import std::is_convertible_v, std::is_lvalue_reference_v, std::is_nothrow_constructible_v, std::remove_cv_t, std::remove_cvref_t
#include <utility> // import std::forward, std::index_sequence, std::in_place, std::in_place_t, std::make_index_sequence, std::move

#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core::util {
template <typename> struct Array_size;
template <typename Type> constexpr auto Array_size_v{Array_size<Type>::value};
template <typename Type, std::size_t Extent = std::dynamic_extent>
class Value_span;
struct ARTCCEL_CORE_EXPORT Move_span_t {
  explicit consteval Move_span_t() noexcept = default;
};

template <typename Type, std::size_t Extent>
class Value_span : public std::span<Type, Extent> {
  // cursed class
private:
  bool move_{false};

public:
  using element_type = typename Value_span::element_type;
  using reference = typename Value_span::reference;
  using Value_span::span::extent;

  constexpr Value_span() noexcept = default;
  constexpr Value_span(Value_span const &other) noexcept
      : Value_span::span{other} {}
  auto operator=(Value_span const &) = delete;
  constexpr Value_span(Value_span &&other) noexcept
      : Value_span::span{std::move(other)}, move_{true} {}
  auto operator=(Value_span &&) = delete;
  constexpr ~Value_span() noexcept = default;

  template <typename Arg>
  // TODO: explicit should use reflection
  explicit(
      !std::is_convertible_v<
          std::remove_cvref_t<Arg> &,
          typename Value_span::
              span>) constexpr Value_span(Move_span_t tag [[maybe_unused]],
                                          Arg &&
                                              arg) noexcept(std::
                                                                is_nothrow_constructible_v<
                                                                    typename Value_span::
                                                                        span,
                                                                    decltype((
                                                                        arg))>)
      : Value_span::span{arg}, move_{true} {}
  template <typename... Args>
  requires(sizeof...(Args) >= 2) explicit(extent !=
                                          std::dynamic_extent) constexpr
      // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
      Value_span(Move_span_t tag [[maybe_unused]], Args &&...args) noexcept(
          std::is_nothrow_constructible_v<typename Value_span::span,
                                          decltype((args))...>)
      : Value_span::span{args...}, move_{true} {}
  template <typename Arg>
  requires(!std::same_as<std::remove_cvref_t<Arg>, Value_span>)
      // TODO: explicit should use reflection
      explicit(!std::is_convertible_v<std::remove_cvref_t<Arg> &,
                                      typename Value_span::span>) constexpr
      // NOLINTNEXTLINE(bugprone-forwarding-reference-overload,google-explicit-constructor,hicpp-explicit-conversions)
      Value_span(Arg &&arg) noexcept(
          std::is_nothrow_constructible_v<typename Value_span::span,
                                          decltype((arg))>)
      : Value_span::span{arg}, move_{!std::is_lvalue_reference_v<Arg>} {}
  template <typename... Args>
  requires(sizeof...(Args) >= 2) explicit(extent !=
                                          std::dynamic_extent) constexpr
      // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
      Value_span(Args &&...args) noexcept(
          std::is_nothrow_constructible_v<
              typename Value_span::span, decltype(std::forward<Args>(args))...>)
      : Value_span::span{std::forward<Args>(args)...} {}

  constexpr auto forward [[nodiscard]] (reference element) const
      noexcept(noexcept(element_type{std::move(element)},
                        element_type{element})) -> element_type {
    // no discard, does actual moving unlike std::move
    if (move_) {
      return std::move(element);
    }
    return element;
  }
#pragma warning(suppress : 4623 4820)
};

namespace f {
template <typename Container>
constexpr auto atad [[nodiscard]] (Container &container) -> decltype(auto) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,clang-analyzer-cplusplus.InnerPointer)
  return std::data(container) + std::size(container);
}
template <typename Container>
constexpr auto atad [[nodiscard]] (const Container &container)
-> decltype(auto) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  return std::data(container) + std::size(container);
}
template <class Element, std::size_t Size>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr auto atad [[nodiscard]] (Element (&array)[Size]) noexcept {
  return array + Size;
}
template <class Element>
constexpr auto atad
    [[nodiscard]] (std::initializer_list<Element> init_list) noexcept {
  return init_list.end();
}

template <std::size_t Size, typename... Args>
requires(Size != std::dynamic_extent) constexpr auto static_span
    [[nodiscard]] (Args &&...args) noexcept(noexcept(std::span{
        std::forward<Args>(args)...})) {
  using span_type = decltype(std::span{std::forward<Args>(args)...});
  return std::span<typename span_type::element_type, Size>{
      std::forward<Args>(args)...};
}
template <std::size_t Size = std::dynamic_extent, typename... Args>
constexpr auto const_span
    [[nodiscard]] (Args &&...args) noexcept(noexcept(std::span{
        std::forward<Args>(args)...})) {
  using span_type = decltype(std::span{std::forward<Args>(args)...});
  return std::span < typename span_type::element_type const,
         Size == std::dynamic_extent ? span_type::extent
                                     : Size > {std::forward<Args>(args)...};
}

template <std::size_t Size = std::dynamic_extent, typename... Args>
constexpr auto const_array
    [[nodiscard]] (std::in_place_t tag [[maybe_unused]],
                   Args &&...args) noexcept(noexcept(std::array{
        std::forward<Args>(args)...})) {
  using array_type = decltype(std::array{std::forward<Args>(args)...});
  // mandatory copy/move elision
  return std::array < typename array_type::value_type const,
         Size == std::dynamic_extent ? Array_size_v<array_type>
                                     : Size > {std::forward<Args>(args)...};
}
template <std::size_t Size = std::dynamic_extent, typename Type,
          std::size_t InSize>
constexpr auto const_array
    [[nodiscard]] (std::array<Type, InSize> const &array) {
  return [&array]<std::size_t... Idxs>(std::index_sequence<Idxs...> idxs
                                       [[maybe_unused]]) {
    return std::array < Type const,
           Size == std::dynamic_extent ? InSize : Size > {{array[Idxs]...}};
  }
  (std::make_index_sequence<InSize>{});
}
template <std::size_t Size = std::dynamic_extent, typename Type,
          std::size_t InSize>
constexpr auto const_array [[nodiscard]] (std::array<Type, InSize> &&array) {
  return [&array]<std::size_t... Idxs>(std::index_sequence<Idxs...> idxs
                                       [[maybe_unused]]) {
    return std::array < Type const, Size == std::dynamic_extent
                                        ? InSize
                                        : Size > {{std::move(array[Idxs])...}};
  }
  (std::make_index_sequence<InSize>{});
}
template <std::size_t Size = std::dynamic_extent, typename Type,
          std::size_t InSize>
requires(InSize != std::dynamic_extent) constexpr auto const_array
    [[nodiscard]] (std::span<Type, InSize> span) {
  return [span]<std::size_t... Idxs>(std::index_sequence<Idxs...> idxs
                                     [[maybe_unused]]) {
    return std::array < Type const,
           Size == std::dynamic_extent ? InSize : Size > {{span[Idxs]...}};
  }
  (std::make_index_sequence<InSize>{});
}
template <std::size_t Size = std::dynamic_extent, typename Type,
          std::size_t InSize>
requires(InSize != std::dynamic_extent) constexpr auto const_array
    [[nodiscard]] (Move_span_t tag [[maybe_unused]],
                   std::span<Type, InSize> span) {
  return [span]<std::size_t... Idxs>(std::index_sequence<Idxs...> idxs
                                     [[maybe_unused]]) {
    return std::array < Type const, Size == std::dynamic_extent
                                        ? InSize
                                        : Size > {{std::move(span[Idxs])...}};
  }
  (std::make_index_sequence<InSize>{});
}
template <std::size_t Size = std::dynamic_extent, typename... Args>
requires(sizeof...(Args) != 1) constexpr auto const_array
    [[nodiscard]] (Args &&...args) noexcept(
        noexcept(f::const_array(std::in_place, std::forward<Args>(args)...))) {
  // mandatory copy/move elision
  return f::const_array<Size>(std::in_place, std::forward<Args>(args)...);
}

template <typename Type, std::size_t Size>
requires(Size != std::dynamic_extent) constexpr auto to_array(
    Value_span<Type, Size> const &span) {
  return [&span]<std::size_t... Idxs>(std::index_sequence<Idxs...> idxs
                                      [[maybe_unused]]) {
    return std::array<std::remove_cv_t<Type>, Size>{
        {span.forward(span[Idxs])...}};
  }
  (std::make_index_sequence<Size>{});
}

template <typename Type, std::size_t Size>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr auto to_array_cv(Type (&array)[Size]) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  return [&array]<std::size_t... Idxs>(std::index_sequence<Idxs...> idxs
                                       [[maybe_unused]]) {
    return std::array<Type, Size>{{array[Idxs]...}};
  }
  (std::make_index_sequence<Size>{});
}
template <typename Type, std::size_t Size>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr auto to_array_cv(Type (&&array)[Size]) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  return [&array]<std::size_t... Idxs>(std::index_sequence<Idxs...> idxs
                                       [[maybe_unused]]) {
    return std::array<Type, Size>{{std::move(array[Idxs])...}};
  }
  (std::make_index_sequence<Size>{});
}
template <typename Type, std::size_t Size>
requires(Size != std::dynamic_extent) constexpr auto to_array_cv(
    Value_span<Type, Size> const &span) {
  return [&span]<std::size_t... Idxs>(std::index_sequence<Idxs...> idxs
                                      [[maybe_unused]]) {
    return std::array<Type, Size>{{span.forward(span[Idxs])...}};
  }
  (std::make_index_sequence<Size>{});
}
} // namespace f

template <typename Type, std::size_t Size>
struct Array_size<std::array<Type, Size>> {
  constexpr static auto value{Size};
};
} // namespace artccel::core::util

#endif
