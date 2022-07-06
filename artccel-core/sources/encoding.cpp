#pragma warning(push) // suppress <xstring>
#pragma warning(disable : 4365)
#include <artccel-core/util/encoding.hpp> // interface

#include <algorithm> // import std::min, std::ranges::for_each
#include <array> // import std::array, std::cbegin, std::cend, std::data, std::empty, std::size
#include <artccel-core/util/cerrno_extras.hpp>  // import Errno_guard
#include <artccel-core/util/codecvt_extras.hpp> // import Codecvt_utf16_utf8, f::codecvt_convert_to_extern, f::codecvt_convert_to_intern
#include <artccel-core/util/exception_extras.hpp> // import f::throw_multiple_as_nested
#include <artccel-core/util/meta.hpp>             // import Unprotect_structors
#include <artccel-core/util/polyfill.hpp>  // import literals::operator""_UZ
#include <artccel-core/util/semantics.hpp> // import null_terminator_size
#include <artccel-core/util/utility_extras.hpp> // import dependent_false_v
#include <cassert>                              // import assert
#include <cerrno>                               // import errno
#include <climits>                              // import MB_LEN_MAX
#include <concepts>                             // import std::same_as
#include <cstring>                              // import std::strerror
#include <cuchar> // import std::c16rtomb, std::c32rtomb, std::mbrtoc16, std::mbrtoc32
#include <cwchar>    // import std::mbrlen, std::mbstate_t, std::size_t
#include <span>      // import std::span
#include <stdexcept> // import std::invalid_argument, std::throw_with_nested
#include <string> // import std::basic_string, std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::basic_string_view, std::string_view, std::u16string_view, std::u32string_view, std::u8string_view
#include <system_error> // import std::generic_category, std::system_error
#include <utility>      // import std::as_const, std::move
#pragma warning(pop)

namespace artccel::core::util {
namespace detail {
using literals::operator""_UZ;

#pragma warning(push)
#pragma warning(disable : 4146)
constexpr static auto cwchar_mbrlen_null{0_UZ};
constexpr static auto cwchar_mbrlen_error [[maybe_unused]]{-1_UZ};
constexpr static auto cwchar_mbrlen_incomplete [[maybe_unused]]{-2_UZ};
constexpr static auto cuchar_mbrtoc_null{0_UZ};
constexpr static auto cuchar_mbrtoc_error{-1_UZ};
constexpr static auto cuchar_mbrtoc_incomplete{-2_UZ};
constexpr static auto cuchar_mbrtoc_surrogate{-3_UZ};
constexpr static auto cuchar_crtomb_surrogate{0_UZ};
constexpr static auto cuchar_crtomb_error{-1_UZ};
#pragma warning(pop)

template <typename UTFCharT>
static auto mbrtoc(UTFCharT &utf_out, std::string_view loc_enc,
                   std::mbstate_t &state) noexcept {
  if constexpr (std::same_as<UTFCharT, char16_t>) {
    return std::mbrtoc16(&utf_out, std::data(loc_enc), std::size(loc_enc),
                         &state);
  } else if constexpr (std::same_as<UTFCharT, char32_t>) {
    return std::mbrtoc32(&utf_out, std::data(loc_enc), std::size(loc_enc),
                         &state);
  } else {
    static_assert(dependent_false_v<UTFCharT>, u8"Unimplemented");
  }
}
template <typename UTFCharT>
static auto crtomb(std::span<char, MB_LEN_MAX> loc_enc_out, UTFCharT utf,
                   std::mbstate_t &state) noexcept {
  if constexpr (std::same_as<UTFCharT, char16_t>) {
    return std::c16rtomb(std::data(loc_enc_out), utf, &state);
  } else if constexpr (std::same_as<UTFCharT, char32_t>) {
    return std::c32rtomb(std::data(loc_enc_out), utf, &state);
  } else {
    static_assert(dependent_false_v<UTFCharT>, u8"Unimplemented");
  }
}

template <typename UTFCharT>
static auto loc_enc_to_utf(std::string_view loc_enc) {
  Errno_guard const errno_guard{};
  std::basic_string<UTFCharT> result{};
  std::mbstate_t state{};
  for (auto old_state{state}; !std::empty(loc_enc); old_state = state) {
    UTFCharT utf_c{}; // not written to if the next character is null
    switch (auto processed{mbrtoc(utf_c, loc_enc, state)}) {
      [[unlikely]] case cuchar_mbrtoc_error : {
        std::system_error errno_exc{errno, std::generic_category()};
        f::throw_multiple_as_nested(std::invalid_argument{errno_exc.what()},
                                    std::move(errno_exc));
      }
      [[unlikely]] case cuchar_mbrtoc_incomplete
          : throw std::invalid_argument{
                std::string{u8"Incomplete byte sequence"_as_utf8_compat}};
    case cuchar_mbrtoc_surrogate:
      break;
      [[unlikely]] case cuchar_mbrtoc_null : {
        auto const null_len_max{
            std::min(std::size_t{MB_LEN_MAX}, std::size(loc_enc))};
        for (auto null_len{1_UZ}; null_len <= null_len_max; ++null_len) {
          if (auto old_state_copy{old_state};
              // NOLINTNEXTLINE(concurrency-mt-unsafe)
              std::mbrlen(std::data(loc_enc), null_len, &old_state_copy) ==
              cwchar_mbrlen_null
              // NOLINTNEXTLINE(google-readability-braces-around-statements,hicpp-braces-around-statements,readability-braces-around-statements)
              ) [[likely]] {
            processed = null_len;
            goto has_null_len;
          }
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
        assert(false && u8"Could not find the length of the null character");
        processed = null_terminator_size;
      has_null_len:
        [[likely]];
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
static auto utf_to_loc_enc(std::basic_string_view<UTFCharT> utf) {
  Errno_guard const errno_guard{};
  std::string result{};
  std::mbstate_t state{};
  std::array<char, MB_LEN_MAX> loc_enc{};
  std::ranges::for_each(
      std::as_const(utf), [&result, &state, &loc_enc](auto utf_c) {
        switch (auto const processed{crtomb(loc_enc, utf_c, state)}) {
          [[unlikely]] case cuchar_crtomb_error : {
            std::system_error errno_exc{errno, std::generic_category()};
            f::throw_multiple_as_nested(std::invalid_argument{errno_exc.what()},
                                        std::move(errno_exc));
          }
        case cuchar_crtomb_surrogate:
          [[fallthrough]];
        default:
          result.append(std::data(loc_enc), processed);
          break;
        }
      });
  return result;
}
} // namespace detail

namespace f {
using literals::operator""_UZ;

auto utf8_compat_as_utf8(std::string_view utf8_compat) -> std::u8string {
  return {std::cbegin(utf8_compat), std::cend(utf8_compat)};
}
auto utf8_as_utf8_compat(std::u8string_view utf8) -> std::string {
  return {std::cbegin(utf8), std::cend(utf8)};
}

auto utf8_to_utf16(std::u8string_view utf8) -> std::u16string {
  return f::codecvt_convert_to_intern<Unprotect_structors<Codecvt_utf16_utf8>>(
      utf8);
}
auto utf8_to_utf16(char8_t utf8) -> std::u16string {
  return utf8_to_utf16({&utf8, 1_UZ});
}
auto utf16_to_utf8(std::u16string_view utf16) -> std::u8string {
  return f::codecvt_convert_to_extern<Unprotect_structors<Codecvt_utf16_utf8>>(
      utf16);
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
