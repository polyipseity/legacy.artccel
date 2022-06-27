#ifndef ARTCCEL_CORE_UTIL_ENCODING_HPP
#define ARTCCEL_CORE_UTIL_ENCODING_HPP
#pragma once

#include "semantics.hpp"      // import null_terminator_size
#include "utility_extras.hpp" // import Template_string
#include <algorithm>          // import std::transform
#include <array>              // import std::array
#include <string> // import std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::cbegin, std::cend, std::size, std::string_view, std::u16string_view, std::u32string_view, std::u8string_view

namespace artccel::core::util {
namespace literals::encoding {
namespace detail {
template <typename AsCharT, Template_string Str>
constexpr inline auto reinterpretation_storage{[] {
  std::array<AsCharT, std::size(Str.data_)> init{};
  std::transform(std::cbegin(Str.data_), std::cend(Str.data_), std::begin(init),
                 [](auto from) noexcept { return static_cast<AsCharT>(from); });
  return init;
}()};
} // namespace detail
template <Template_string Str>
constexpr auto operator""_as_utf8 [[nodiscard]] () {
  return std::u8string_view{
      std::cbegin(detail::reinterpretation_storage<char8_t, Str>),
      std::size(detail::reinterpretation_storage<char8_t, Str>) -
          null_terminator_size};
}
constexpr auto operator""_as_utf8 [[nodiscard]] (char utf8_compat) {
  return static_cast<char8_t>(utf8_compat);
}
template <Template_string Str>
constexpr auto operator""_as_utf8_compat [[nodiscard]] () {
  return std::string_view{
      std::cbegin(detail::reinterpretation_storage<char, Str>),
      std::size(detail::reinterpretation_storage<char, Str>) -
          null_terminator_size};
}
constexpr auto operator""_as_utf8_compat [[nodiscard]] (char8_t utf8) {
  return static_cast<char>(utf8);
}
} // namespace literals::encoding

namespace f {
auto utf8_compat_to_utf8(std::string_view utf8_compat) -> std::u8string;
auto utf8_to_utf8_compat(std::u8string_view utf8) -> std::string;

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
} // namespace artccel::core::util

#endif
