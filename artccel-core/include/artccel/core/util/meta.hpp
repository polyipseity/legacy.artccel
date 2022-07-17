#ifndef GUARD_7B51F7E5_D339_44D7_85C1_7DE334308D98
#define GUARD_7B51F7E5_D339_44D7_85C1_7DE334308D98
#pragma once

#include <array>    // import std::array
#include <concepts> // import std::same_as
#include <cstddef>  // import std::size_t
#include <span>     // import std::dynamic_extent, std::span

#include "containers.hpp" // import f::to_array_cv
#include "semantics.hpp"  // import null_terminator_size

namespace artccel::core::util {
template <typename CharT, std::size_t Size> struct Template_string;
template <typename Type, typename Find, typename Replace> struct Replace_all;
enum struct Replace_target : bool { self = false, container = true };
template <typename Type, typename Find, typename Replace>
using Replace_all_t = typename Replace_all<Type, Find, Replace>::type;
template <typename Type, template <Replace_target> typename Find,
          typename Replace>
using Replace_all_t_t =
    Replace_all_t<Replace_all_t<Type, Find<Replace_target::container>, Replace>,
                  Find<Replace_target::self>, Replace>;

template <typename CharT, std::size_t Size> struct Template_string {
  using char_type = CharT;
  constexpr static auto size_{Size};
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::array<CharT const, Size> data_;

  explicit consteval Template_string(std::array<CharT const, Size> const &str)
      : data_{str} {}
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions,cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  consteval Template_string(CharT const (&str)[Size])
      : Template_string{f::to_array_cv(str)} {
    // implicit for use in string literal operator template
  }
  explicit consteval Template_string(std::span<CharT const, Size> str) requires(
      Size != std::dynamic_extent)
      : Template_string{f::to_array_cv(str)} {}
  explicit consteval Template_string(char chr) : Template_string{{{chr}}} {}
  explicit consteval Template_string(wchar_t chr) : Template_string{{{chr}}} {}
  explicit consteval Template_string(char8_t chr) : Template_string{{{chr}}} {}
  explicit consteval Template_string(char16_t chr) : Template_string{{{chr}}} {}
  explicit consteval Template_string(char32_t chr) : Template_string{{{chr}}} {}

  consteval Template_string(Template_string const &) = default;
  auto operator=(Template_string const &) = delete;
  Template_string(Template_string &&) = delete;
  auto operator=(Template_string &&) = delete;
  constexpr ~Template_string() noexcept = default;
};
Template_string(char chr)->Template_string<char, 1 + null_terminator_size>;
Template_string(wchar_t chr)
    ->Template_string<wchar_t, 1 + null_terminator_size>;
Template_string(char8_t chr)
    ->Template_string<char8_t, 1 + null_terminator_size>;
Template_string(char16_t chr)
    ->Template_string<char16_t, 1 + null_terminator_size>;
Template_string(char32_t chr)
    ->Template_string<char32_t, 1 + null_terminator_size>;
static_assert(Template_string{""}.data_ == f::to_array_cv(""),
              u8"Implementation error");
static_assert(Template_string{char{}}.data_ == f::to_array_cv("\0"),
              u8"Implementation error");
static_assert(Template_string{wchar_t{}}.data_ == f::to_array_cv(L"\0"),
              u8"Implementation error");
static_assert(Template_string{char8_t{}}.data_ == f::to_array_cv(u8"\0"),
              u8"Implementation error");
static_assert(Template_string{char16_t{}}.data_ == f::to_array_cv(u"\0"),
              u8"Implementation error");
static_assert(Template_string{char32_t{}}.data_ == f::to_array_cv(U"\0"),
              u8"Implementation error");

template <typename Found, typename Replace>
struct Replace_all<Found, Found, Replace> {
  using type = Replace;
};
template <template <Replace_target> typename Found, typename Replace>
struct Replace_all<Found<Replace_target::self>, Found<Replace_target::self>,
                   Replace> {
  using type = Replace;
};
template <template <typename> typename Container,
          template <Replace_target> typename Found, typename Replace>
struct Replace_all<Container<Found<Replace_target::container>>,
                   Found<Replace_target::container>, Replace> {
  using type = Replace;
};
template <template <typename...> typename Type, typename Find, typename Replace,
          typename... TArgs>
struct Replace_all<Type<TArgs...>, Find, Replace> {
  using type = Type<Replace_all_t<TArgs, Find, Replace>...>;
};
template <typename Type, typename Find, typename Replace>
struct Replace_all<Type *, Find, Replace> {
  using type = Replace_all_t<Type, Find, Replace> *;
};
template <typename Type, typename Find, typename Replace>
struct Replace_all<Type &, Find, Replace> {
  using type = Replace_all_t<Type, Find, Replace> &;
};
template <typename Type, typename Find, typename Replace>
struct Replace_all<Type &&, Find, Replace> {
  using type = Replace_all_t<Type, Find, Replace> &&;
};
template <typename NotFound, typename Find, typename Replace>
requires(
    !std::same_as<NotFound, Find>) struct Replace_all<NotFound, Find, Replace> {
  using type = NotFound;
};
} // namespace artccel::core::util

#endif
