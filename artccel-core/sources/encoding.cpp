#include "artccel-core/encoding.hpp" // interface
#include "artccel-core/polyfill.hpp" // import literals::operator""_UZ
#include <algorithm>                 // import std::min
#include <array>                     // import std::array
#include <cassert>                   // import assert
#include <cerrno>                    // import errno
#include <climits>                   // import MB_LEN_MAX
#include <codecvt>                   // import std::codecvt_utf8_utf16
#include <cstring>                   // import std::strerror
#include <cuchar>                    // import std::c16rtomb, std::mbrtoc16
#include <cwchar>                    // import std::mbrlen, std::mbstate_t, std::size_t
#include <gsl/gsl>                   // import gsl::czstring
#include <locale>                    // import std::wstring_convert
#include <stdexcept>                 // import std::invalid_argument
#include <string> // import std::string_view, std::u16string_view, std::u8string_view
#include <string_view> // import std::string, std::u16string, std::u8string

namespace artccel::core::util {
auto c8s_compatrtoc8s(std::string_view c8s_compat) -> std::u8string {
  // undefined behavior??? (but no reinterpret_cast)
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
  std::u16string result{};
  std::mbstate_t state{};
  while (!mbs.empty()) {
    auto old_state{state};
    auto c16{
        u'\0'}; // not written by std::mbrtoc16 if the next character is null
    auto processed{std::mbrtoc16(&c16, mbs.begin(), mbs.size(), &state)};
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
    result.push_back(c16);
  }
  for (char16_t c16{}; std::mbrtoc16(&c16, mbs.begin(), mbs.size(), &state) ==
                       cuchar_mbrtoc_surrogate;) {
    result.push_back(c16); // complete surrogate pair of the last character
  }
  return result;
}

auto c8srtombs(std::u8string_view c8s) -> std::string {
  // TODO: use std::c8rtomb
  return c16srtombs(c8srtoc16s(c8s));
}
auto c16srtombs(std::u16string_view c16s) -> std::string {
  std::string result{};
  std::mbstate_t state{};
  std::array<char, MB_LEN_MAX> mb{};
  for (auto const c16 : c16s) {
    auto const processed{std::c16rtomb(mb.data(), c16, &state)};
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
