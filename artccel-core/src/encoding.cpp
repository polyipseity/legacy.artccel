#include "artccel-core/encoding.hpp"
#include "artccel-core/semantics.hpp"
#include <cassert>
#include <cerrno>
#include <codecvt>
#include <cstring>
#include <cuchar>
#include <gsl/gsl>
#include <locale>
#include <stdexcept>
#include <string>
#include <string_view>

namespace artccel::core::util {
auto mbrlen_null(std::string_view mbsr, std::mbstate_t &state) -> size_t {
  auto const old_state{state};
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  auto const result{std::mbrlen(mbsr.begin(), mbsr.size(), &state)};
  [[unlikely]] if (result == cwchar_mbrlen_null) {
    for (size_t null_len{1}; null_len < mbsr.size(); ++null_len) {
      auto state_copy{old_state};
      // NOLINTNEXTLINE(concurrency-mt-unsafe)
      if (std::mbrlen(mbsr.begin(), null_len, &state_copy) ==
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

auto mbsrtoc8s(std::string_view mbsr) -> std::u8string {
  // TODO: use std::mbrtoc8
  auto const c8s{
      std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
          .to_bytes(mbsrtoc16s(mbsr))};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return {reinterpret_cast<util::Cu8zstring>(c8s.data()), c8s.size()};
}

auto mbsrtoc16s(std::string_view mbsr) -> std::u16string {
  std::u16string result{};
  std::mbstate_t state{};
  while (!mbsr.empty()) {
    auto old_state{state};
    auto c16{
        u'\0'}; // not written by std::mbrtoc16 if the next character is null
    auto processed{std::mbrtoc16(&c16, mbsr.begin(), mbsr.size(), &state)};
    switch (processed) {
      [[unlikely]] case cuchar_error :
          // NOLINTNEXTLINE(concurrency-mt-unsafe)
          throw std::invalid_argument{std::strerror(errno)};
      break;
      [[unlikely]] case cuchar_incomplete
          : throw std::invalid_argument{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                reinterpret_cast<gsl::czstring>(u8"Incomplete byte sequence")};
      break;
    case cuchar_surrogate:
      break;
      [[unlikely]] case cuchar_null
          : switch (processed = mbrlen_null(mbsr, old_state)) {
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
      mbsr.remove_prefix(processed);
      break;
    }
    result.push_back(c16);
  }
  for (char16_t c16{}; std::mbrtoc16(&c16, mbsr.begin(), mbsr.size(), &state) ==
                       cuchar_surrogate;) {
    result.push_back(c16); // complete surrogate pair of the last character
  }
  return result;
}
} // namespace artccel::core::util
