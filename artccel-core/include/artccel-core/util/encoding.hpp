#ifndef ARTCCEL_CORE_UTIL_ENCODING_HPP
#define ARTCCEL_CORE_UTIL_ENCODING_HPP
#pragma once

#include "containers_extras.hpp" // import f::const_array
#include "semantics.hpp"         // import null_terminator_size
#include "utility_extras.hpp"    // import Template_string
#include <algorithm>             // import std::ranges::transform
#include <array> // import std::array, std::begin, std::data, std::size
#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT
#include <string> // import std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::string_view, std::u16string_view, std::u32string_view, std::u8string_view
#include <utility>     // import std::as_const, std::move

namespace artccel::core::util {
namespace detail {
template <typename AsCharT, Template_string Str>
constexpr auto reinterpretation_storage{[] {
  static_assert(sizeof(AsCharT) == sizeof(typename decltype(Str)::char_type),
                u8"sizeof the two character types mismatch");
  std::array<AsCharT, std::size(Str.data_)> init{};
  std::ranges::transform(
      std::as_const(Str.data_), std::begin(init),
      [](auto chr) noexcept { return static_cast<AsCharT>(chr); });
  return f::const_array(std::move(init));
}()};
} // namespace detail

namespace f {
ARTCCEL_CORE_EXPORT auto utf8_compat_as_utf8(std::string_view utf8_compat)
    -> std::u8string;
template <Template_string Str>
consteval auto utf8_compat_as_utf8_array [[nodiscard]] () noexcept -> auto & {
  return detail::reinterpretation_storage<char8_t, Str>;
}
template <Template_string Str>
consteval auto utf8_compat_as_utf8 [[nodiscard]] () {
  return std::u8string_view{std::data(f::utf8_compat_as_utf8_array<Str>()),
                            std::size(f::utf8_compat_as_utf8_array<Str>()) -
                                null_terminator_size};
}
ARTCCEL_CORE_EXPORT constexpr auto utf8_compat_as_utf8
    [[nodiscard]] (char utf8_compat) noexcept {
  return static_cast<char8_t>(utf8_compat);
}
ARTCCEL_CORE_EXPORT auto utf8_as_utf8_compat(std::u8string_view utf8)
    -> std::string;
template <Template_string Str>
consteval auto utf8_as_utf8_compat_array [[nodiscard]] () noexcept -> auto & {
  return detail::reinterpretation_storage<char, Str>;
}
template <Template_string Str>
consteval auto utf8_as_utf8_compat [[nodiscard]] () {
  return std::string_view{std::data(f::utf8_as_utf8_compat_array<Str>()),
                          std::size(f::utf8_as_utf8_compat_array<Str>()) -
                              null_terminator_size};
}
ARTCCEL_CORE_EXPORT constexpr auto utf8_as_utf8_compat
    [[nodiscard]] (char8_t utf8) noexcept {
  return static_cast<char>(utf8);
}

ARTCCEL_CORE_EXPORT auto utf8_to_utf16(std::u8string_view utf8)
    -> std::u16string;
ARTCCEL_CORE_EXPORT auto utf8_to_utf16(char8_t utf8) -> std::u16string;
ARTCCEL_CORE_EXPORT auto utf16_to_utf8(std::u16string_view utf16)
    -> std::u8string;
ARTCCEL_CORE_EXPORT auto utf16_to_utf8(char16_t utf16) -> std::u8string;

ARTCCEL_CORE_EXPORT auto loc_enc_to_utf8(std::string_view loc_enc)
    -> std::u8string;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf8(char loc_enc) -> std::u8string;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf16(std::string_view loc_enc)
    -> std::u16string;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf16(char loc_enc) -> std::u16string;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf32(std::string_view loc_enc)
    -> std::u32string;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf32(char loc_enc) -> std::u32string;
ARTCCEL_CORE_EXPORT auto utf8_to_loc_enc(std::u8string_view utf8)
    -> std::string;
ARTCCEL_CORE_EXPORT auto utf8_to_loc_enc(char8_t utf8) -> std::string;
ARTCCEL_CORE_EXPORT auto utf16_to_loc_enc(std::u16string_view utf16)
    -> std::string;
ARTCCEL_CORE_EXPORT auto utf16_to_loc_enc(char16_t utf16) -> std::string;
ARTCCEL_CORE_EXPORT auto utf32_to_loc_enc(std::u32string_view utf32)
    -> std::string;
ARTCCEL_CORE_EXPORT auto utf32_to_loc_enc(char32_t utf32) -> std::string;
} // namespace f

namespace literals::encoding {
template <Template_string Str>
constexpr auto operator""_as_utf8_array [[nodiscard]] () noexcept
    -> decltype(auto) {
  return f::utf8_compat_as_utf8_array<Str>();
}
template <Template_string Str>
constexpr auto operator""_as_utf8 [[nodiscard]] () -> decltype(auto) {
  return f::utf8_compat_as_utf8<Str>();
}
ARTCCEL_CORE_EXPORT constexpr auto operator""_as_utf8
    [[nodiscard]] (char utf8_compat) noexcept -> decltype(auto) {
  return f::utf8_compat_as_utf8(utf8_compat);
}
template <Template_string Str>
constexpr auto operator""_as_utf8_compat_array [[nodiscard]] () noexcept
    -> decltype(auto) {
  return f::utf8_as_utf8_compat_array<Str>();
}
template <Template_string Str>
constexpr auto operator""_as_utf8_compat [[nodiscard]] () -> decltype(auto) {
  return f::utf8_as_utf8_compat<Str>();
}
ARTCCEL_CORE_EXPORT constexpr auto operator""_as_utf8_compat
    [[nodiscard]] (char8_t utf8) noexcept -> decltype(auto) {
  return f::utf8_as_utf8_compat(utf8);
}
} // namespace literals::encoding
} // namespace artccel::core::util

#endif
