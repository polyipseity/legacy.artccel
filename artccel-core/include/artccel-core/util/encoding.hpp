#ifndef ARTCCEL_CORE_UTIL_ENCODING_HPP
#define ARTCCEL_CORE_UTIL_ENCODING_HPP
#pragma once

#include "polyfill.hpp" // import literals::operator""_UZ
#include <cwchar>       // import std::mbstate_t, std::size_t
#include <string> // import std::string, std::u16string, std::u32string, std::u8string
#include <string_view> // import std::string_view, std::u16string_view, std::u32string_view, std::u8string_view

namespace artccel::core::util {
constexpr auto cwchar_mbrlen_null{0_UZ};
constexpr auto cwchar_mbrlen_error{-1_UZ};
constexpr auto cwchar_mbrlen_incomplete{-2_UZ};
constexpr auto cuchar_mbrtoc_null{0_UZ};
constexpr auto cuchar_mbrtoc_error{-1_UZ};
constexpr auto cuchar_mbrtoc_incomplete{-2_UZ};
constexpr auto cuchar_mbrtoc_surrogate{-3_UZ};
constexpr auto cuchar_crtomb_surrogate{0_UZ};
constexpr auto cuchar_crtomb_error{-1_UZ};

auto c8s_compatrtoc8s(std::string_view c8s_compat) -> std::u8string;
auto c8srtoc8s_compat(std::u8string_view c8s) -> std::string;

auto c8srtoc16s(std::u8string_view c8s) -> std::u16string;
auto c16srtoc8s(std::u16string_view c16s) -> std::u8string;

auto mbsrtoc8s(std::string_view mbs) -> std::u8string;
auto mbsrtoc16s(std::string_view mbs) -> std::u16string;
auto mbsrtoc32s(std::string_view mbs) -> std::u32string;
auto c8srtombs(std::u8string_view c8s) -> std::string;
auto c16srtombs(std::u16string_view c16s) -> std::string;
auto c32srtombs(std::u32string_view c32s) -> std::string;

auto mbrlen_null(std::string_view mbs, std::mbstate_t &state) -> std::size_t;
} // namespace artccel::core::util

#endif
