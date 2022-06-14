#ifndef ARTCCEL_CORE_UTIL_META_HPP
#define ARTCCEL_CORE_UTIL_META_HPP
#pragma once

#include "encoding.hpp" // import mbsrtoc8s
#include <algorithm>    // import std::copy_n
#include <array>        // import std::array, std::to_array
#include <cstddef>      // import std::size_t
#include <string_view>  // import std::string_view
#include <type_traits>  // import std::remove_cv_t

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
  std::string_view const control_type_name{/*u8*/ "int"};
  std::string_view const control{raw_type_name<int>()};
  std::size_t const junk_prefix{control.find(control_type_name)};
  std::size_t const junk_suffix{control.size() - junk_prefix -
                                control_type_name.size()};
} type_name_format;
template <typename T>
static constexpr auto type_name_storage{[] {
  if constexpr (type_name_format.junk_prefix == std::string_view::npos) {
    return std::to_array(/*u8*/ "<type name unavailable>");
  }
  constexpr std::string_view type_name{
      raw_type_name<T>().cbegin() + type_name_format.junk_prefix,
      raw_type_name<T>().cend() - type_name_format.junk_suffix};
  std::array<char, type_name.size() + 1> ret{};
  std::copy_n(type_name.data(), type_name.size(), ret.data());
  return ret;
}()};
} // namespace detail

template <typename P> struct Remove_ptr;
// NOLINTNEXTLINE(altera-struct-pack-align)
template <typename T> struct Remove_ptr<T *> { using type = T; };
// NOLINTNEXTLINE(altera-struct-pack-align)
template <typename T> struct Remove_ptr<T &> { using type = T; };
// NOLINTNEXTLINE(altera-struct-pack-align)
template <typename T> struct Remove_ptr<T &&> { using type = T; };
template <template <typename...> typename P, typename T>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Remove_ptr<P<T>> {
  using type = T;
};
template <typename P> using Remove_ptr_t = typename Remove_ptr<P>::type;

// NOLINTNEXTLINE(altera-struct-pack-align)
template <typename P> struct Remove_cvptr {
  using type = std::remove_cv_t<Remove_ptr_t<P>>;
};
template <typename P> using Remove_cvptr_t = typename Remove_cvptr<P>::type;

template <typename P, typename R> struct Replace_ptr_value_type;
// NOLINTNEXTLINE(altera-struct-pack-align)
template <typename T, typename R> struct Replace_ptr_value_type<T *, R> {
  using type = R *;
};
// NOLINTNEXTLINE(altera-struct-pack-align)
template <typename T, typename R> struct Replace_ptr_value_type<T &, R> {
  using type = R &;
};
// NOLINTNEXTLINE(altera-struct-pack-align)
template <typename T, typename R> struct Replace_ptr_value_type<T &&, R> {
  using type = R &&;
};
template <template <typename...> typename P, typename T, typename R>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Replace_ptr_value_type<P<T>, R> {
  using type = P<R>;
};
template <typename P, typename R>
using Replace_ptr_value_type_t = typename Replace_ptr_value_type<P, R>::type;

template <typename T>
constexpr auto type_name_mbs_data [[nodiscard]] () noexcept {
  return detail::type_name_storage<T>;
}
template <typename T> constexpr auto type_name_mbs [[nodiscard]] () noexcept {
  constexpr std::string_view ret{detail::type_name_storage<T>.cbegin(),
                                 detail::type_name_storage<T>.cend() - 1};
  return ret;
}
template <typename T> auto type_name [[nodiscard]] () {
  return mbsrtoc8s(type_name_mbs<T>());
}
} // namespace artccel::core::util

#endif
