#include <algorithm> // import std::min
#include <array>     // import std::array, std::data, std::empty, std::size
#include <cassert>   // import assert
#include <cerrno>    // import errno
#include <climits>   // import MB_LEN_MAX
#include <concepts>  // import std::same_as
#include <cstring>   // import std::memcpy
#include <cuchar> // import std::c16rtomb, std::c32rtomb, std::mbrtoc16, std::mbrtoc32
#include <cwchar>    // import std::mbrlen, std::mbstate_t, std::size_t
#include <span>      // import std::span
#include <stdexcept> // import std::invalid_argument
#include <string> // import std::basic_string, std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::basic_string_view, std::string_view, std::u16string_view, std::u32string_view, std::u8string_view
#include <system_error> // import std::generic_category, std::system_error
#include <utility>      // import std::as_const, std::move

#pragma warning(push)
#pragma warning(disable : 4582 4583 4625 4626 4820 5026 5027)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#include <tl/expected.hpp> // import tl::expected, tl::unexpect, tl::unexpected
#pragma clang diagnostic pop
#pragma warning(pop)

#include <artccel/core/util/encoding.hpp> // interface

#include <artccel/core/util/cerrno_extras.hpp>  // import Errno_guard
#include <artccel/core/util/codecvt_extras.hpp> // import Codecvt_error, Codecvt_utf16_utf8, f::codecvt_convert_to_extern, f::codecvt_convert_to_intern
#include <artccel/core/util/exception_extras.hpp> // import f::make_nested_exception
#include <artccel/core/util/polyfill.hpp>         // import f::unreachable
#include <artccel/core/util/semantics.hpp>        // import null_terminator_size
#include <artccel/core/util/utility_extras.hpp> // import Semiregularize, dependent_false_v

namespace artccel::core::util {
namespace detail {
#pragma warning(push)
#pragma warning(disable : 4146)
// TODO: C++23: UZ
constexpr static std::size_t cwchar_mbrlen_null{0};
constexpr static auto cwchar_mbrlen_error [[maybe_unused]]{std::size_t(-1)};
constexpr static auto cwchar_mbrlen_incomplete
    [[maybe_unused]]{std::size_t(-2)};
constexpr static std::size_t cuchar_mbrtoc_null{0};
constexpr static auto cuchar_mbrtoc_error{std::size_t(-1)};
constexpr static auto cuchar_mbrtoc_incomplete{std::size_t(-2)};
constexpr static auto cuchar_mbrtoc_surrogate{std::size_t(-3)};
constexpr static std::size_t cuchar_crtomb_surrogate{0};
constexpr static auto cuchar_crtomb_error{std::size_t(-1)};
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
  using return_type =
      tl::expected<decltype(result), Cuchar_error_with_exception>;

  std::mbstate_t state{};
  for (auto old_state{state}; !std::empty(loc_enc); old_state = state) {
    UTFCharT utf_c{}; // not written to if the next character is null
    switch (auto processed{detail::mbrtoc(utf_c, loc_enc, state)}) {
      [[unlikely]] case cuchar_mbrtoc_error : {
        std::system_error errno_exc{errno, std::generic_category()};
        return return_type{
            tl::unexpect,
            typename return_type::error_type{
                f::make_nested_exception(
                    std::invalid_argument{errno_exc.what()},
                    std::move(errno_exc)),
                Cuchar_error::error}}; // TODO: C++23: tl::in_place is broken
      }
      [[unlikely]] case cuchar_mbrtoc_incomplete :
          // TODO: C++23: tl::in_place is broken
          return return_type{
              tl::unexpect,
              typename return_type::error_type{
                  std::invalid_argument{
                      std::string{u8"Incomplete byte sequence"_as_utf8_compat}},
                  Cuchar_error::partial}};
    case cuchar_mbrtoc_surrogate:
      break;
      [[unlikely]] case cuchar_mbrtoc_null : {
        auto const null_len_max{
            std::min(std::size_t{MB_LEN_MAX}, std::size(loc_enc))};
        // TODO: C++23: UZ
        for (std::size_t null_len{1}; null_len <= null_len_max; ++null_len) {
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
       detail::mbrtoc(utf_c, loc_enc, state) == cuchar_mbrtoc_surrogate;) {
    result.push_back(utf_c); // complete surrogate pair of the last character
  }
  return return_type{std::move(result)};
}
template <typename UTFCharT>
static auto utf_to_loc_enc(std::basic_string_view<UTFCharT> utf) {
  Errno_guard const errno_guard{};
  std::string result{};
  using return_type =
      tl::expected<decltype(result), Cuchar_error_with_exception>;

  std::mbstate_t state{};
  for (std::array<char, MB_LEN_MAX> loc_enc{};
       auto utf_c : std::as_const(utf)) {
    switch (auto const processed{detail::crtomb(loc_enc, utf_c, state)}) {
      [[unlikely]] case cuchar_crtomb_error : {
        std::system_error errno_exc{errno, std::generic_category()};
        return return_type{
            tl::unexpect,
            typename return_type::error_type{
                f::make_nested_exception(
                    std::invalid_argument{errno_exc.what()},
                    std::move(errno_exc)),
                Cuchar_error::error}}; // TODO: C++23: tl::in_place is broken
      }
    case cuchar_crtomb_surrogate:
      [[fallthrough]];
    default:
      result.append(std::data(loc_enc), processed);
      break;
    }
  }
  return return_type{std::move(result)};
}
static auto to_convert_err(Codecvt_error error) noexcept {
  switch (error) {
  case Codecvt_error::error:
    return Convert_error::error;
  case Codecvt_error::partial:
    return Convert_error::partial;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
  default:
#pragma clang diagnostic pop
    f::unreachable();
  }
}
static auto to_cuchar_err(Convert_error error) noexcept {
  switch (error) {
  case Convert_error::error:
    return Cuchar_error::error;
  case Convert_error::partial:
    return Cuchar_error::partial;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
  default:
#pragma clang diagnostic pop
    f::unreachable();
  }
}
} // namespace detail

namespace f {
auto utf8_compat_as_utf8(std::string_view utf8_compat) -> std::u8string {
  std::u8string ret(std::size(utf8_compat), char8_t{});
  std::memcpy(std::data(ret), std::data(utf8_compat), std::size(ret));
  return ret;
}
auto utf8_as_utf8_compat(std::u8string_view utf8) -> std::string {
  std::string ret(std::size(utf8), char{});
  std::memcpy(std::data(ret), std::data(utf8), std::size(ret));
  return ret;
}

auto utf8_to_utf16(std::u8string_view utf8)
    -> tl::expected<std::u16string, Convert_error_with_exception> {
  return map_err(
      f::codecvt_convert_to_intern<Semiregularize<Codecvt_utf16_utf8>>(utf8),
      &detail::to_convert_err);
}
auto utf8_to_utf16(char8_t utf8)
    -> tl::expected<std::u16string, Convert_error_with_exception> {
  return utf8_to_utf16({&utf8, 1});
}
auto utf16_to_utf8(std::u16string_view utf16)
    -> tl::expected<std::u8string, Convert_error_with_exception> {
  return map_err(
      f::codecvt_convert_to_extern<Semiregularize<Codecvt_utf16_utf8>>(utf16),
      &detail::to_convert_err);
}
auto utf16_to_utf8(char16_t utf16)
    -> tl::expected<std::u8string, Convert_error_with_exception> {
  return utf16_to_utf8({&utf16, 1});
}

auto loc_enc_to_utf8(std::string_view loc_enc)
    -> tl::expected<std::u8string, Cuchar_error_with_exception> {
  // TODO: use std::mbrtoc8
  auto u16{loc_enc_to_utf16(loc_enc)};
  if (u16) {
    return map_err(utf16_to_utf8(*u16), &detail::to_cuchar_err);
  }
  return tl::unexpected{std::move(u16).error()};
}
auto loc_enc_to_utf8(char loc_enc)
    -> tl::expected<std::u8string, Cuchar_error_with_exception> {
  return loc_enc_to_utf8({&loc_enc, 1});
}
auto loc_enc_to_utf16(std::string_view loc_enc)
    -> tl::expected<std::u16string, Cuchar_error_with_exception> {
  return detail::loc_enc_to_utf<char16_t>(loc_enc);
}
auto loc_enc_to_utf16(char loc_enc)
    -> tl::expected<std::u16string, Cuchar_error_with_exception> {
  return loc_enc_to_utf16({&loc_enc, 1});
}
auto loc_enc_to_utf32(std::string_view loc_enc)
    -> tl::expected<std::u32string, Cuchar_error_with_exception> {
  return detail::loc_enc_to_utf<char32_t>(loc_enc);
}
auto loc_enc_to_utf32(char loc_enc)
    -> tl::expected<std::u32string, Cuchar_error_with_exception> {
  return loc_enc_to_utf32({&loc_enc, 1});
}

auto utf8_to_loc_enc(std::u8string_view utf8)
    -> tl::expected<std::string, Cuchar_error_with_exception> {
  // TODO: use std::c8rtomb
  auto u16{utf8_to_utf16(utf8)};
  if (u16) {
    return utf16_to_loc_enc(*u16);
  }
  return tl::unexpected{
      map_err(std::move(u16), &detail::to_cuchar_err).error()};
}
auto utf8_to_loc_enc(char8_t utf8)
    -> tl::expected<std::string, Cuchar_error_with_exception> {
  return utf8_to_loc_enc({&utf8, 1});
}
auto utf16_to_loc_enc(std::u16string_view utf16)
    -> tl::expected<std::string, Cuchar_error_with_exception> {
  return detail::utf_to_loc_enc(utf16);
}
auto utf16_to_loc_enc(char16_t utf16)
    -> tl::expected<std::string, Cuchar_error_with_exception> {
  return utf16_to_loc_enc({&utf16, 1});
}
auto utf32_to_loc_enc(std::u32string_view utf32)
    -> tl::expected<std::string, Cuchar_error_with_exception> {
  return detail::utf_to_loc_enc(utf32);
}
auto utf32_to_loc_enc(char32_t utf32)
    -> tl::expected<std::string, Cuchar_error_with_exception> {
  return utf32_to_loc_enc({&utf32, 1});
}
} // namespace f
} // namespace artccel::core::util
