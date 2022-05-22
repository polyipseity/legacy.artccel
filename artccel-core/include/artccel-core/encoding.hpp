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
constexpr auto cuchar_mbrtoc_null{static_cast<size_t>(0)};
constexpr auto cuchar_mbrtoc_error{static_cast<size_t>(-1)};
constexpr auto cuchar_mbrtoc_incomplete{static_cast<size_t>(-2)};
constexpr auto cuchar_mbrtoc_surrogate{static_cast<size_t>(-3)};
constexpr auto cuchar_crtomb_surrogate{static_cast<size_t>(0)};
constexpr auto cuchar_crtomb_error{static_cast<size_t>(-1)};

auto c8s_compatrtoc8s(std::string_view c8s_compat) -> std::u8string;
auto c8srtoc8s_compat(std::u8string_view c8s) -> std::string;

auto c8srtoc16s(std::u8string_view c8s) -> std::u16string;
auto c16srtoc8s(std::u16string_view c16s) -> std::u8string;

auto mbsrtoc8s(std::string_view mbs) -> std::u8string;
auto mbsrtoc16s(std::string_view mbs) -> std::u16string;
auto c8srtombs(std::u8string_view c8s) -> std::string;
auto c16srtombs(std::u16string_view c16s) -> std::string;

auto mbrlen_null(std::string_view mbs, std::mbstate_t &state) -> size_t;
} // namespace artccel::core::util

#endif
