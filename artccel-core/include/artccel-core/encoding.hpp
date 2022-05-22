#ifndef ARTCCEL_CORE_ENCODING_HPP
#define ARTCCEL_CORE_ENCODING_HPP
#pragma once

#include <cwchar>
#include <string>
#include <string_view>

namespace artccel::core::util {
constexpr auto cwchar_mbrlen_null{static_cast<size_t>(0)};
constexpr auto cwchar_mbrlen_error{static_cast<size_t>(-1)};
constexpr auto cwchar_mbrlen_incomplete{static_cast<size_t>(-2)};
constexpr auto cuchar_null{static_cast<size_t>(0)};
constexpr auto cuchar_error{static_cast<size_t>(-1)};
constexpr auto cuchar_incomplete{static_cast<size_t>(-2)};
constexpr auto cuchar_surrogate{static_cast<size_t>(-3)};

auto mbrlen_null(std::string_view mbsr, std::mbstate_t &state) -> size_t;
auto mbsrtoc8s(std::string_view mbsr) -> std::u8string;
auto mbsrtoc16s(std::string_view mbsr) -> std::u16string;
} // namespace artccel::core::util

#endif
