#ifndef ARTCCEL_CORE_UTIL_ENCODING_HPP
#define ARTCCEL_CORE_UTIL_ENCODING_HPP
#pragma once

#include <algorithm> // import std::ranges::transform
#include <array>     // import std::array, std::begin, std::data, std::size
#include <cstddef>   // import std::size_t
#include <cstdint>   // import std::int_fast8_t
#include <cstring>   // import std::memcpy
#include <istream>   // import std::basic_istream
#include <ostream>   // import std::basic_ostream
#include <string> // import std::basic_string, std::getline, std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::basic_string_view, std::string_view, std::u16string_view, std::u32string_view, std::u8string_view
#include <tuple>       // import std::ignore
#include <utility>     // import std::as_const

#pragma warning(push)
#pragma warning(disable : 4582 4583 4625 4626 4820 5026 5027)
#include <tl/expected.hpp> // import tl::expected
#pragma warning(pop)

#include "containers.hpp"        // import f::const_array
#include "conversions.hpp"       // import f::int_modulo_cast
#include "error_handling.hpp"    // import Error_with_exception
#include "meta.hpp"              // import Template_string
#include "semantics.hpp"         // import null_terminator_size
#include "string_extras.hpp"     // import Char_traits_c, Compatible_char_traits
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core::util {
enum struct Convert_error : std::int_fast8_t;
using Convert_error_with_exception = Error_with_exception<Convert_error>;
enum struct Cuchar_error : std::int_fast8_t;
using Cuchar_error_with_exception = Error_with_exception<Cuchar_error>;

enum struct Convert_error : std::int_fast8_t { error, partial };
enum struct Cuchar_error : std::int_fast8_t { error, partial };

namespace detail {
template <typename AsCharT, Template_string Str>
constexpr auto reinterpretation_storage{[] {
  static_assert(sizeof(AsCharT) == sizeof(typename decltype(Str)::char_type),
                u8"sizeof the two character types mismatch");
  std::array<AsCharT, std::size(Str.data_)> init{};
  std::ranges::transform(
      std::as_const(Str.data_), std::begin(init),
      [](auto chr) noexcept { return f::int_modulo_cast<AsCharT>(chr); });
  return f::const_array(init);
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
  return f::int_modulo_cast<char8_t>(utf8_compat);
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
  return f::int_modulo_cast<char>(utf8);
}

ARTCCEL_CORE_EXPORT auto utf8_to_utf16(std::u8string_view utf8)
    -> tl::expected<std::u16string, Convert_error_with_exception>;
ARTCCEL_CORE_EXPORT auto utf8_to_utf16(char8_t utf8)
    -> tl::expected<std::u16string, Convert_error_with_exception>;
ARTCCEL_CORE_EXPORT auto utf16_to_utf8(std::u16string_view utf16)
    -> tl::expected<std::u8string, Convert_error_with_exception>;
ARTCCEL_CORE_EXPORT auto utf16_to_utf8(char16_t utf16)
    -> tl::expected<std::u8string, Convert_error_with_exception>;

ARTCCEL_CORE_EXPORT auto loc_enc_to_utf8(std::string_view loc_enc)
    -> tl::expected<std::u8string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf8(char loc_enc)
    -> tl::expected<std::u8string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf16(std::string_view loc_enc)
    -> tl::expected<std::u16string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf16(char loc_enc)
    -> tl::expected<std::u16string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf32(std::string_view loc_enc)
    -> tl::expected<std::u32string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto loc_enc_to_utf32(char loc_enc)
    -> tl::expected<std::u32string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto utf8_to_loc_enc(std::u8string_view utf8)
    -> tl::expected<std::string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto utf8_to_loc_enc(char8_t utf8)
    -> tl::expected<std::string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto utf16_to_loc_enc(std::u16string_view utf16)
    -> tl::expected<std::string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto utf16_to_loc_enc(char16_t utf16)
    -> tl::expected<std::string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto utf32_to_loc_enc(std::u32string_view utf32)
    -> tl::expected<std::string, Cuchar_error_with_exception>;
ARTCCEL_CORE_EXPORT auto utf32_to_loc_enc(char32_t utf32)
    -> tl::expected<std::string, Cuchar_error_with_exception>;

template <Char_traits_c StreamTraits,
          Compatible_char_traits<StreamTraits, char8_t> StrTraits,
          typename Allocator>
auto getline_utf8(std::basic_istream<char, StreamTraits> &input,
                  std::basic_string<char8_t, StrTraits, Allocator> &str,
                  char8_t delim = u8'\n') -> decltype(auto) {
  std::basic_string<char, StreamTraits> temp{};
  decltype(auto) ret{std::getline(input, temp, utf8_as_utf8_compat(delim))};
  str.resize(std::size(temp));
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.InnerPointer)
  std::memcpy(std::data(str), std::data(temp), std::size(temp));
  return ret;
}
template <Char_traits_c StreamTraits,
          Compatible_char_traits<StreamTraits, char8_t> StrTraits,
          typename Allocator>
auto getline_utf8(std::basic_istream<char, StreamTraits> &&input,
                  std::basic_string<char8_t, StrTraits, Allocator> &str,
                  char8_t delim = u8'\n') -> decltype(auto) {
  return getline_utf8(input, str, delim);
}
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

namespace operators::utf8_compat {
namespace ostream {
template <Char_traits_c StreamTraits, std::size_t Size>
auto operator<<(
    std::basic_ostream<char, StreamTraits> &left,
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    char8_t (&right)[Size]) = delete;
template <Char_traits_c StreamTraits,
          Compatible_char_traits<StreamTraits, char8_t> StrTraits>
auto operator<<(std::basic_ostream<char, StreamTraits> &left,
                std::basic_string_view<char8_t, StrTraits> right)
    -> decltype(auto) {
  return left << f::utf8_as_utf8_compat(right);
}
template <Char_traits_c StreamTraits,
          Compatible_char_traits<StreamTraits, char8_t> StrTraits,
          typename Allocator>
auto operator<<(std::basic_ostream<char, StreamTraits> &left,
                std::basic_string<char8_t, StrTraits, Allocator> const &right)
    -> decltype(auto) {
  return left << std::basic_string_view<char8_t, StrTraits>{right};
}
} // namespace ostream
namespace istream {
template <Char_traits_c StreamTraits,
          Compatible_char_traits<StreamTraits, char8_t> StrTraits,
          typename Allocator>
auto operator>>(std::basic_istream<char, StreamTraits> &left,
                std::basic_string<char8_t, StrTraits, Allocator> &right)
    -> decltype(auto) {
  std::basic_string<char, StreamTraits> temp{};
  decltype(auto) ret{left >> temp};
  right.resize(std::size(temp));
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.InnerPointer)
  std::memcpy(std::data(right), std::data(temp), std::size(temp));
  return ret;
}
} // namespace istream
} // namespace operators::utf8_compat
} // namespace artccel::core::util

#endif
