#include "artccel-core/encoding.hpp" // interface
#include "artccel-core/polyfill.hpp" // import literals::operator""_UZ
#include <algorithm>                 // import std::min
#include <array>                     // import std::array
#include <cassert>                   // import assert
#include <cerrno>                    // import errno
#include <climits>                   // import MB_LEN_MAX
#include <codecvt>                   // import std::codecvt_utf8_utf16
#include <cstring>                   // import std::strerror
#include <cuchar> // import std::c16rtomb, std::c32rtomb, std::mbrtoc16, std::mbrtoc32
#include <cwchar>    // import std::mbrlen, std::mbstate_t, std::size_t
#include <gsl/gsl>   // import gsl::czstring
#include <locale>    // import std::wstring_convert
#include <span>      // import std::span
#include <stdexcept> // import std::invalid_argument
#include <string> // import std::basic_string, std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::basic_string_view, std::string_view, std::u16string_view, std::u32string_view, std::u8string_view

namespace artccel::core::util {
namespace detail {
template <typename CharT>
auto mbrtoc(CharT &out, std::string_view in, std::mbstate_t &state)
    -> std::size_t;
template <>
auto mbrtoc(char16_t &out, std::string_view in, std::mbstate_t &state)
    -> std::size_t {
  return std::mbrtoc16(&out, in.data(), in.size(), &state);
}
template <>
auto mbrtoc(char32_t &out, std::string_view in, std::mbstate_t &state)
    -> std::size_t {
  return std::mbrtoc32(&out, in.data(), in.size(), &state);
}

template <typename CharT>
auto crtomb(std::span<char, MB_LEN_MAX> out, CharT in, std::mbstate_t &state)
    -> std::size_t;
template <>
auto crtomb(std::span<char, MB_LEN_MAX> out, char16_t in, std::mbstate_t &state)
    -> std::size_t {
  return std::c16rtomb(out.data(), in, &state);
}
template <>
auto crtomb(std::span<char, MB_LEN_MAX> out, char32_t in, std::mbstate_t &state)
    -> std::size_t {
  return std::c32rtomb(out.data(), in, &state);
}

template <typename CharT>
auto mbsrtocs(std::string_view mbs) -> std::basic_string<CharT> {
  std::basic_string<CharT> result{};
  std::mbstate_t state{};
  while (!mbs.empty()) {
    auto old_state{state};
    CharT c{u8'\0'}; // not written to if the next character is null
    auto processed{mbrtoc(c, mbs, state)};
    switch (processed) {
      [[unlikely]] case cuchar_mbrtoc_error :
          // NOLINTNEXTLINE(concurrency-mt-unsafe)
          throw std::invalid_argument{std::strerror(errno)};
      [[unlikely]] case cuchar_mbrtoc_incomplete
          : throw std::invalid_argument{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                reinterpret_cast<gsl::czstring>(u8"Incomplete byte sequence")};
    case cuchar_mbrtoc_surrogate:
      break;
      [[unlikely]] case cuchar_mbrtoc_null
          : switch (processed = mbrlen_null(mbs, old_state)) {
        [[unlikely]] case cwchar_mbrlen_null
            : [[unlikely]] case cwchar_mbrlen_error
            : [[unlikely]] case cwchar_mbrlen_incomplete :
            // clang-format off
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
            /* clang-format on */ assert(
                false && u8"Could not find the length of the null character");
        break;
      }
      [[fallthrough]];
    default:
      mbs.remove_prefix(processed);
      break;
    }
    result.push_back(c);
  }
  for (CharT c{}; mbrtoc(c, mbs, state) == cuchar_mbrtoc_surrogate;) {
    result.push_back(c); // complete surrogate pair of the last character
  }
  return result;
}
template <typename CharT>
auto csrtombs(std::basic_string_view<CharT> cs) -> std::string {
  std::string result{};
  std::mbstate_t state{};
  std::array<char, MB_LEN_MAX> mb{};
  for (auto const c : cs) {
    auto const processed{crtomb(mb, c, state)};
    switch (processed) {
      [[unlikely]] case cuchar_crtomb_error :
          // NOLINTNEXTLINE(concurrency-mt-unsafe)
          throw std::invalid_argument{std::strerror(errno)};
    case cuchar_crtomb_surrogate:
    default:
      result.append(mb.data(), processed);
      break;
    }
  }
  return result;
}
} // namespace detail

auto c8s_compatrtoc8s(std::string_view c8s_compat) -> std::u8string {
  return {c8s_compat.begin(), c8s_compat.end()};
}
auto c8srtoc8s_compat(std::u8string_view c8s) -> std::string {
  return {c8s.begin(), c8s.end()};
}

auto c8srtoc16s(std::u8string_view c8s) -> std::u16string {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
      .from_bytes(
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<char const *>(c8s.begin()), // defined behavior
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<char const *>(c8s.end())); // defined behavior
}
auto c16srtoc8s(std::u16string_view c16s) -> std::u8string {
  return c8s_compatrtoc8s(
      std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
          .to_bytes(c16s.begin(), c16s.end()));
}

auto mbsrtoc8s(std::string_view mbs) -> std::u8string {
  // TODO: use std::mbrtoc8
  return c16srtoc8s(mbsrtoc16s(mbs));
}
auto mbsrtoc16s(std::string_view mbs) -> std::u16string {
  return detail::mbsrtocs<char16_t>(mbs);
}
auto mbsrtoc32s(std::string_view mbs) -> std::u32string {
  return detail::mbsrtocs<char32_t>(mbs);
}

auto c8srtombs(std::u8string_view c8s) -> std::string {
  // TODO: use std::c8rtomb
  return c16srtombs(c8srtoc16s(c8s));
}
auto c16srtombs(std::u16string_view c16s) -> std::string {
  return detail::csrtombs(c16s);
}
auto c32srtombs(std::u32string_view c32s) -> std::string {
  return detail::csrtombs(c32s);
}

auto mbrlen_null(std::string_view mbs, std::mbstate_t &state) -> std::size_t {
  auto const old_state{state};
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  auto const result{std::mbrlen(mbs.begin(), mbs.size(), &state)};
  [[unlikely]] if (result == cwchar_mbrlen_null) {
    auto const null_len_max{
        std::min(static_cast<std::size_t>(MB_LEN_MAX), mbs.size())};
    for (auto null_len{1_UZ}; null_len <= null_len_max; ++null_len) {
      auto state_copy{old_state};
      // NOLINTNEXTLINE(concurrency-mt-unsafe)
      if (std::mbrlen(mbs.begin(), null_len, &state_copy) ==
          cwchar_mbrlen_null) {
        return null_len;
      }
    }
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
    /* clang-format on */ assert(
        false && u8"Could not find the length of the null character");
  }
  return result;
}
} // namespace artccel::core::util
