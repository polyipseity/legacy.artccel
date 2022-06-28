#include <algorithm> // import std::min, std::ranges::for_each
#include <array> // import std::array, std::cbegin, std::cend, std::data, std::empty, std::size
#include <artccel-core/util/encoding.hpp> // interface
#include <artccel-core/util/polyfill.hpp> // import literals::operator""_UZ
#include <artccel-core/util/utility_extras.hpp> // import dependent_false_v
#include <cassert>                              // import assert
#include <cerrno>                               // import errno
#include <climits>                              // import MB_LEN_MAX
#include <codecvt>  // import std::codecvt_utf8_utf16
#include <concepts> // import std::same_as
#include <cstring>  // import std::strerror
#include <cuchar> // import std::c16rtomb, std::c32rtomb, std::mbrtoc16, std::mbrtoc32
#include <cwchar>    // import std::mbrlen, std::mbstate_t, std::size_t
#include <locale>    // import std::wstring_convert
#include <span>      // import std::span
#include <stdexcept> // import std::invalid_argument
#include <string> // import std::basic_string, std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::basic_string_view, std::string_view, std::u16string_view, std::u32string_view, std::u8string_view
#include <utility>     // import std::as_const

namespace artccel::core::util {
namespace detail {
using literals::operator""_UZ;

constexpr auto cwchar_mbrlen_null{0_UZ};
constexpr auto cwchar_mbrlen_error{-1_UZ};
constexpr auto cwchar_mbrlen_incomplete{-2_UZ};
constexpr auto cuchar_mbrtoc_null{0_UZ};
constexpr auto cuchar_mbrtoc_error{-1_UZ};
constexpr auto cuchar_mbrtoc_incomplete{-2_UZ};
constexpr auto cuchar_mbrtoc_surrogate{-3_UZ};
constexpr auto cuchar_crtomb_surrogate{0_UZ};
constexpr auto cuchar_crtomb_error{-1_UZ};

template <typename UTFCharT>
auto mbrtoc(UTFCharT &utf_out, std::string_view loc_enc,
            std::mbstate_t &state) noexcept {
  if constexpr (std::same_as<UTFCharT, char16_t>) {
    return std::mbrtoc16(&utf_out, std::cbegin(loc_enc), std::size(loc_enc),
                         &state);
  } else if constexpr (std::same_as<UTFCharT, char32_t>) {
    return std::mbrtoc32(&utf_out, std::cbegin(loc_enc), std::size(loc_enc),
                         &state);
  } else {
    static_assert(dependent_false_v<UTFCharT>, u8"Unimplemented");
  }
}
template <typename UTFCharT>
auto crtomb(std::span<char, MB_LEN_MAX> loc_enc_out, UTFCharT utf,
            std::mbstate_t &state) noexcept {
  if constexpr (std::same_as<UTFCharT, char16_t>) {
    return std::c16rtomb(std::data(loc_enc_out), utf, &state);
  } else if constexpr (std::same_as<UTFCharT, char32_t>) {
    return std::c32rtomb(std::data(loc_enc_out), utf, &state);
  } else {
    static_assert(dependent_false_v<UTFCharT>, u8"Unimplemented");
  }
}

auto mbrlen_unspecialized_null(std::string_view loc_enc,
                               std::mbstate_t &state) noexcept {
  auto const old_state{state};
  auto const result{
      // NOLINTNEXTLINE(concurrency-mt-unsafe)
      std::mbrlen(std::cbegin(loc_enc), std::size(loc_enc), &state)};
  // NOLINTNEXTLINE(google-readability-braces-around-statements,hicpp-braces-around-statements,readability-braces-around-statements)
  if (result == cwchar_mbrlen_null) [[unlikely]] {
    auto const null_len_max{
        std::min(std::size_t{MB_LEN_MAX}, std::size(loc_enc))};
    for (auto null_len{1_UZ}; null_len <= null_len_max; ++null_len) {
      auto state_copy{old_state};
      // NOLINTNEXTLINE(concurrency-mt-unsafe)
      if (std::mbrlen(std::cbegin(loc_enc), null_len, &state_copy) ==
          cwchar_mbrlen_null) {
        return null_len;
      }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(false && u8"Could not find the length of the null character");
  }
  return result;
}

template <typename UTFCharT> auto loc_enc_to_utf(std::string_view loc_enc) {
  std::basic_string<UTFCharT> result{};
  std::mbstate_t state{};
  while (!std::empty(loc_enc)) {
    auto old_state{state};
    UTFCharT utf_c{u8'\0'}; // not written to if the next character is null
    auto processed{mbrtoc(utf_c, loc_enc, state)};
    switch (processed) {
      [[unlikely]] case cuchar_mbrtoc_error :
          // NOLINTNEXTLINE(concurrency-mt-unsafe)
          throw std::invalid_argument{std::strerror(errno)};
      [[unlikely]] case cuchar_mbrtoc_incomplete
          : throw std::invalid_argument{
                f::utf8_to_loc_enc(u8"Incomplete byte sequence")};
    case cuchar_mbrtoc_surrogate:
      break;
      [[unlikely]] case cuchar_mbrtoc_null
          : switch (processed = mbrlen_unspecialized_null(loc_enc, old_state)) {
        [[unlikely]] case cwchar_mbrlen_null : [[fallthrough]];
        [[unlikely]] case cwchar_mbrlen_error : [[fallthrough]];
        [[unlikely]] case cwchar_mbrlen_incomplete :
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
            assert(false &&
                   u8"Could not find the length of the null character");
        break;
      }
      [[fallthrough]];
    default:
      loc_enc.remove_prefix(processed);
      break;
    }
    result.push_back(utf_c);
  }
  for (UTFCharT utf_c{};
       mbrtoc(utf_c, loc_enc, state) == cuchar_mbrtoc_surrogate;) {
    result.push_back(utf_c); // complete surrogate pair of the last character
  }
  return result;
}
template <typename UTFCharT>
auto utf_to_loc_enc(std::basic_string_view<UTFCharT> utf) {
  std::string result{};
  std::mbstate_t state{};
  std::array<char, MB_LEN_MAX> loc_enc{};
  std::ranges::for_each(
      std::as_const(utf), [&result, &state, &loc_enc](auto utf_c) {
        auto const processed{crtomb(loc_enc, utf_c, state)};
        switch (processed) {
          [[unlikely]] case cuchar_crtomb_error :
              // NOLINTNEXTLINE(concurrency-mt-unsafe)
              throw std::invalid_argument{std::strerror(errno)};
        case cuchar_crtomb_surrogate:
          [[fallthrough]];
        default:
          result.append(std::cbegin(loc_enc), processed);
          break;
        }
      });
  return result;
}
} // namespace detail

namespace f {
using literals::operator""_UZ;

auto utf8_compat_as_utf8(std::string_view utf8_compat) -> std::u8string {
  return {std::cbegin(utf8_compat), std::size(utf8_compat)};
}
auto utf8_as_utf8_compat(std::u8string_view utf8) -> std::string {
  return {std::cbegin(utf8), std::size(utf8)};
}

auto utf8_to_utf16(std::u8string_view utf8) -> std::u16string {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
      .from_bytes(
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<char const *>(std::cbegin(utf8)), // defined behavior
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<char const *>(std::cend(utf8))); // defined behavior
}
auto utf8_to_utf16(char8_t utf8) -> std::u16string {
  return utf8_to_utf16({&utf8, 1_UZ});
}
auto utf16_to_utf8(std::u16string_view utf16) -> std::u8string {
  return utf8_compat_as_utf8(
      std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
          .to_bytes(std::cbegin(utf16), std::cend(utf16)));
}
auto utf16_to_utf8(char16_t utf16) -> std::u8string {
  return utf16_to_utf8({&utf16, 1_UZ});
}

auto loc_enc_to_utf8(std::string_view loc_enc) -> std::u8string {
  // TODO: use std::mbrtoc8
  return utf16_to_utf8(loc_enc_to_utf16(loc_enc));
}
auto loc_enc_to_utf8(char loc_enc) -> std::u8string {
  return loc_enc_to_utf8({&loc_enc, 1_UZ});
}
auto loc_enc_to_utf16(std::string_view loc_enc) -> std::u16string {
  return detail::loc_enc_to_utf<char16_t>(loc_enc);
}
auto loc_enc_to_utf16(char loc_enc) -> std::u16string {
  return loc_enc_to_utf16({&loc_enc, 1_UZ});
}
auto loc_enc_to_utf32(std::string_view loc_enc) -> std::u32string {
  return detail::loc_enc_to_utf<char32_t>(loc_enc);
}
auto loc_enc_to_utf32(char loc_enc) -> std::u32string {
  return loc_enc_to_utf32({&loc_enc, 1_UZ});
}

auto utf8_to_loc_enc(std::u8string_view utf8) -> std::string {
  // TODO: use std::c8rtomb
  return utf16_to_loc_enc(utf8_to_utf16(utf8));
}
auto utf8_to_loc_enc(char8_t utf8) -> std::string {
  return utf8_to_loc_enc({&utf8, 1_UZ});
}
auto utf16_to_loc_enc(std::u16string_view utf16) -> std::string {
  return detail::utf_to_loc_enc(utf16);
}
auto utf16_to_loc_enc(char16_t utf16) -> std::string {
  return utf16_to_loc_enc({&utf16, 1_UZ});
}
auto utf32_to_loc_enc(std::u32string_view utf32) -> std::string {
  return detail::utf_to_loc_enc(utf32);
}
auto utf32_to_loc_enc(char32_t utf32) -> std::string {
  return utf32_to_loc_enc({&utf32, 1_UZ});
}
} // namespace f
} // namespace artccel::core::util
