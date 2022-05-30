#ifndef ARTCCEL_CORE_REFLECT_HPP
#define ARTCCEL_CORE_REFLECT_HPP
#pragma once

#include "encoding.hpp" // import mbsrtoc8s
#include <algorithm>    // import std::copy_n
#include <array>        // import std::array, std::to_array
#include <cstddef>      // import std::size_t
#include <string_view>  // import std::string_view

namespace artccel::core::util {
namespace detail {
template <typename T>
constexpr auto raw_type_name [[nodiscard]] () -> std::string_view {
#ifndef _MSC_VER
  // clang-format off
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
  /* clang-format on */ return __PRETTY_FUNCTION__;
#else
  // clang-format off
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
  /* clang-format on */ return __FUNCSIG__;
#endif
}
// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
constexpr struct alignas(64) {
  std::string_view const control_type_name{"int"};
  std::string_view const control{raw_type_name<int>()};
  std::size_t const junk_prefix{control.find(control_type_name)};
  std::size_t const junk_suffix{control.size() - junk_prefix -
                                control_type_name.size()};
} type_name_format;
template <typename T>
static constexpr auto type_name_storage{[] {
  if constexpr (type_name_format.junk_prefix == std::string_view::npos) {
    return std::to_array("<type name unavailable>");
  }
  constexpr std::string_view type_name{
      raw_type_name<T>().cbegin() + type_name_format.junk_prefix,
      raw_type_name<T>().cend() - type_name_format.junk_suffix};
  std::array<char, type_name.size() + 1> ret{};
  std::copy_n(type_name.data(), type_name.size(), ret.data());
  return ret;
}()};
} // namespace detail

template <typename T>
constexpr auto type_name_mbs_data [[nodiscard]] () noexcept {
  return detail::type_name_storage<T>;
}
template <typename T> constexpr auto type_name_mbs [[nodiscard]] () {
  constexpr std::string_view ret{detail::type_name_storage<T>.cbegin(),
                                 detail::type_name_storage<T>.cend() - 1};
  return ret;
}
template <typename T> auto type_name [[nodiscard]] () {
  return mbsrtoc8s(type_name_mbs<T>());
}
} // namespace artccel::core::util

#endif
