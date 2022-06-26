#ifndef ARTCCEL_CORE_UTIL_ENCODING_HPP
#define ARTCCEL_CORE_UTIL_ENCODING_HPP
#pragma once

#include <string> // import std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::string_view, std::u16string_view, std::u32string_view, std::u8string_view

namespace artccel::core::util::f {
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
} // namespace artccel::core::util::f

#endif
