#include "artccel-core/util/encoding.hpp" // interface
#include "artccel-core/util/polyfill.hpp" // import literals::operator""_UZ
#include <algorithm>                      // import std::min
#include <array>                          // import std::array
#include <cassert>                        // import assert
#include <cerrno>                         // import errno
#include <climits>                        // import MB_LEN_MAX
#include <codecvt>                        // import std::codecvt_utf8_utf16
#include <cstring>                        // import std::strerror
#include <cuchar> // import std::c16rtomb, std::c32rtomb, std::mbrtoc16, std::mbrtoc32
#include <cwchar>    // import std::mbrlen, std::mbstate_t, std::size_t
#include <locale>    // import std::wstring_convert
#include <span>      // import std::span
#include <stdexcept> // import std::invalid_argument
#include <string> // import std::basic_string, std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::basic_string_view, std::string_view, std::u16string_view, std::u32string_view, std::u8string_view
#include <utility>     // import std::as_const

namespace artccel::core::util {
using literals::operator""_UZ;

namespace detail {
constexpr auto cwchar_mbrlen_null{0_UZ};
constexpr auto cwchar_mbrlen_error{-1_UZ};
constexpr auto cwchar_mbrlen_incomplete{-2_UZ};
constexpr auto cuchar_mbrtoc_null{0_UZ};
constexpr auto cuchar_mbrtoc_error{-1_UZ};
constexpr auto cuchar_mbrtoc_incomplete{-2_UZ};
constexpr auto cuchar_mbrtoc_surrogate{-3_UZ};
constexpr auto cuchar_crtomb_surrogate{0_UZ};
constexpr auto cuchar_crtomb_error{-1_UZ};

auto mbrlen_null(std::string_view mbs, std::mbstate_t &state) -> std::size_t {
  auto const old_state{state};
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  auto const result{std::mbrlen(mbs.cbegin(), mbs.size(), &state)};
  // clang-format off
  // NOLINTNEXTLINE(google-readability-braces-around-statements, hicpp-braces-around-statements, readability-braces-around-statements)
  /* clang-format on */ if (result == cwchar_mbrlen_null) [[unlikely]] {
    auto const null_len_max{std::min(std::size_t{MB_LEN_MAX}, mbs.size())};
    for (auto null_len{1_UZ}; null_len <= null_len_max; ++null_len) {
      auto state_copy{old_state};
      // NOLINTNEXTLINE(concurrency-mt-unsafe)
      if (std::mbrlen(mbs.cbegin(), null_len, &state_copy) ==
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

template <typename CharT>
auto mbrtoc(CharT &output, std::string_view input, std::mbstate_t &state)
    -> std::size_t;
template <>
auto mbrtoc(char16_t &output, std::string_view input, std::mbstate_t &state)
    -> std::size_t {
  return std::mbrtoc16(&output, input.cbegin(), input.size(), &state);
}
template <>
auto mbrtoc(char32_t &output, std::string_view input, std::mbstate_t &state)
    -> std::size_t {
  return std::mbrtoc32(&output, input.cbegin(), input.size(), &state);
}

template <typename CharT>
auto crtomb(std::span<char, MB_LEN_MAX> output, CharT input,
            std::mbstate_t &state) -> std::size_t;
template <>
auto crtomb(std::span<char, MB_LEN_MAX> output, char16_t input,
            std::mbstate_t &state) -> std::size_t {
  return std::c16rtomb(output.data(), input, &state);
}
template <>
auto crtomb(std::span<char, MB_LEN_MAX> output, char32_t input,
            std::mbstate_t &state) -> std::size_t {
  return std::c32rtomb(output.data(), input, &state);
}

template <typename CharT>
auto mbsrtocs(std::string_view mbs) -> std::basic_string<CharT> {
  std::basic_string<CharT> result{};
  std::mbstate_t state{};
  while (!mbs.empty()) {
    auto old_state{state};
    CharT out_c{u8'\0'}; // not written to if the next character is null
    auto processed{mbrtoc(out_c, mbs, state)};
    switch (processed) {
      [[unlikely]] case cuchar_mbrtoc_error :
          // NOLINTNEXTLINE(concurrency-mt-unsafe)
          throw std::invalid_argument{std::strerror(errno)};
      [[unlikely]] case cuchar_mbrtoc_incomplete
          : throw std::invalid_argument{
                c8srtombs(u8"Incomplete byte sequence")};
    case cuchar_mbrtoc_surrogate:
      break;
      [[unlikely]] case cuchar_mbrtoc_null
          : switch (processed = mbrlen_null(mbs, old_state)) {
        [[unlikely]] case cwchar_mbrlen_null : [[fallthrough]];
        [[unlikely]] case cwchar_mbrlen_error : [[fallthrough]];
        [[unlikely]] case cwchar_mbrlen_incomplete :
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
    result.push_back(out_c);
  }
  for (CharT out_c{}; mbrtoc(out_c, mbs, state) == cuchar_mbrtoc_surrogate;) {
    result.push_back(out_c); // complete surrogate pair of the last character
  }
  return result;
}
template <typename CharT>
auto csrtombs(std::basic_string_view<CharT> cs_) -> std::string {
  std::string result{};
  std::mbstate_t state{};
  std::array<char, MB_LEN_MAX> out_mb{};
  for (auto const in_c : std::as_const(cs_)) {
    auto const processed{crtomb(out_mb, in_c, state)};
    switch (processed) {
      [[unlikely]] case cuchar_crtomb_error :
          // NOLINTNEXTLINE(concurrency-mt-unsafe)
          throw std::invalid_argument{std::strerror(errno)};
    case cuchar_crtomb_surrogate:
      [[fallthrough]];
    default:
      result.append(out_mb.cbegin(), processed);
      break;
    }
  }
  return result;
}
} // namespace detail

auto c8s_compatrtoc8s(std::string_view c8s_compat) -> std::u8string {
  return {c8s_compat.cbegin(), c8s_compat.cend()};
}
auto c8srtoc8s_compat(std::u8string_view c8s) -> std::string {
  return {c8s.cbegin(), c8s.cend()};
}

auto c8srtoc16s(std::u8string_view c8s) -> std::u16string {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
      .from_bytes(
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<char const *>(c8s.cbegin()), // defined behavior
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<char const *>(c8s.cend())); // defined behavior
}
auto c8srtoc16s(char8_t c8s) -> std::u16string {
  return c8srtoc16s({&c8s, 1_UZ});
}
auto c16srtoc8s(std::u16string_view c16s) -> std::u8string {
  return c8s_compatrtoc8s(
      std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
          .to_bytes(c16s.cbegin(), c16s.cend()));
}
auto c16srtoc8s(char16_t c16s) -> std::u8string {
  return c16srtoc8s({&c16s, 1_UZ});
}

auto mbsrtoc8s(std::string_view mbs) -> std::u8string {
  // TODO: use std::mbrtoc8
  return c16srtoc8s(mbsrtoc16s(mbs));
}
auto mbsrtoc8s(char mbs) -> std::u8string { return mbsrtoc8s({&mbs, 1_UZ}); }
auto mbsrtoc16s(std::string_view mbs) -> std::u16string {
  return detail::mbsrtocs<char16_t>(mbs);
}
auto mbsrtoc16s(char mbs) -> std::u16string { return mbsrtoc16s({&mbs, 1_UZ}); }
auto mbsrtoc32s(std::string_view mbs) -> std::u32string {
  return detail::mbsrtocs<char32_t>(mbs);
}
auto mbsrtoc32s(char mbs) -> std::u32string { return mbsrtoc32s({&mbs, 1_UZ}); }

auto c8srtombs(std::u8string_view c8s) -> std::string {
  // TODO: use std::c8rtomb
  return c16srtombs(c8srtoc16s(c8s));
}
auto c8srtombs(char8_t c8s) -> std::string { return c8srtombs({&c8s, 1_UZ}); }
auto c16srtombs(std::u16string_view c16s) -> std::string {
  return detail::csrtombs(c16s);
}
auto c16srtombs(char16_t c16s) -> std::string {
  return c16srtombs({&c16s, 1_UZ});
}
auto c32srtombs(std::u32string_view c32s) -> std::string {
  return detail::csrtombs(c32s);
}
auto c32srtombs(char32_t c32s) -> std::string {
  return c32srtombs({&c32s, 1_UZ});
}
} // namespace artccel::core::util
