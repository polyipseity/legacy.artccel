#ifndef ARTCCEL_CORE_UTIL_ENCODING_HPP
#define ARTCCEL_CORE_UTIL_ENCODING_HPP
#pragma once

#include "containers_extras.hpp" // import f::const_array
#include "semantics.hpp"         // import null_terminator_size
#include "utility_extras.hpp"    // import Template_string
#include <algorithm>             // import std::ranges::transform
#include <array> // import std::array, std::begin, std::data, std::size
#include <string> // import std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::string_view, std::u16string_view, std::u32string_view, std::u8string_view
#include <utility>     // import std::as_const, std::move

namespace artccel::core::util {
namespace detail {
template <typename AsCharT, Template_string Str>
constexpr auto reinterpretation_storage{[] {
  std::array<AsCharT, std::size(Str.data_)> init{};
  std::ranges::transform(
      std::as_const(Str.data_), std::begin(init),
      [](auto chr) noexcept { return static_cast<AsCharT>(chr); });
  return f::const_array(std::move(init));
}()};
} // namespace detail

namespace f {
auto utf8_compat_as_utf8(std::string_view utf8_compat) -> std::u8string;
template <Template_string Str> consteval auto utf8_compat_as_utf8() {
  return std::u8string_view{
      std::data(detail::reinterpretation_storage<char8_t, Str>),
      std::size(detail::reinterpretation_storage<char8_t, Str>) -
          null_terminator_size};
}
constexpr auto utf8_compat_as_utf8(char utf8_compat) noexcept {
  return static_cast<char8_t>(utf8_compat);
}
auto utf8_as_utf8_compat(std::u8string_view utf8) -> std::string;
template <Template_string Str> consteval auto utf8_as_utf8_compat() {
  return std::string_view{
      std::data(detail::reinterpretation_storage<char, Str>),
      std::size(detail::reinterpretation_storage<char, Str>) -
          null_terminator_size};
}
constexpr auto utf8_as_utf8_compat(char8_t utf8) noexcept {
  return static_cast<char>(utf8);
}

auto utf8_to_utf16(std::u8string_view utf8) -> std::u16string;
auto utf8_to_utf16(char8_t utf8) -> std::u16string;
auto utf16_to_utf8(std::u16string_view utf16) -> std::u8string;
auto utf16_to_utf8(char16_t utf16) -> std::u8string;

auto loc_enc_to_utf8(std::string_view loc_enc) -> std::u8string;
auto loc_enc_to_utf8(char loc_enc) -> std::u8string;
auto loc_enc_to_utf16(std::string_view loc_enc) -> std::u16string;
auto loc_enc_to_utf16(char loc_enc) -> std::u16string;
auto loc_enc_to_utf32(std::string_view loc_enc) -> std::u32string;
auto loc_enc_to_utf32(char loc_enc) -> std::u32string;
auto utf8_to_loc_enc(std::u8string_view utf8) -> std::string;
auto utf8_to_loc_enc(char8_t utf8) -> std::string;
auto utf16_to_loc_enc(std::u16string_view utf16) -> std::string;
auto utf16_to_loc_enc(char16_t utf16) -> std::string;
auto utf32_to_loc_enc(std::u32string_view utf32) -> std::string;
auto utf32_to_loc_enc(char32_t utf32) -> std::string;
} // namespace f

namespace literals::encoding {
template <Template_string Str>
constexpr auto operator""_as_utf8 [[nodiscard]] () {
  return f::utf8_compat_as_utf8<Str>();
}
constexpr auto operator""_as_utf8 [[nodiscard]] (char utf8_compat) noexcept {
  return f::utf8_compat_as_utf8(utf8_compat);
}
template <Template_string Str>
constexpr auto operator""_as_utf8_compat [[nodiscard]] () {
  return f::utf8_as_utf8_compat<Str>();
}
constexpr auto operator""_as_utf8_compat [[nodiscard]] (char8_t utf8) noexcept {
  return f::utf8_as_utf8_compat(utf8);
}
} // namespace literals::encoding
} // namespace artccel::core::util

#endif
