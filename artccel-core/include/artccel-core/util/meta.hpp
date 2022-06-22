#ifndef ARTCCEL_CORE_UTIL_META_HPP
#define ARTCCEL_CORE_UTIL_META_HPP
#pragma once

#include "encoding.hpp"  // import mbsrtoc8s
#include "semantics.hpp" // import null_terminator_size
#include <algorithm>     // import std::copy_n
#include <array>         // import std::array, std::to_array
#include <concepts>      // import std::same_as
#include <cstddef>       // import std::size_t
#include <string_view>   // import std::string_view

namespace artccel::core::util {
namespace detail {
template <typename T>
consteval static auto raw_type_name [[nodiscard]] () -> std::string_view {
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
constexpr static struct alignas(64) {
  std::string_view const control_type_name{/*u8*/ "int"};
  std::string_view const control{raw_type_name<int>()};
  std::size_t const junk_prefix{control.find(control_type_name)};
  std::size_t const junk_suffix{control.size() - junk_prefix -
                                control_type_name.size()};
} type_name_format{};
template <typename T>
constexpr static auto type_name_storage{[] {
  if constexpr (type_name_format.junk_prefix == std::string_view::npos) {
    return std::to_array(/*u8*/ "<type name unavailable>");
  } else {
    constexpr std::string_view type_name{
        raw_type_name<T>().cbegin() + type_name_format.junk_prefix,
        raw_type_name<T>().cend() - type_name_format.junk_suffix};
    std::array<char, type_name.size() + null_terminator_size> ret{};
    std::copy_n(type_name.cbegin(), type_name.size(), ret.begin());
    return ret;
  }
}()};

template <typename T, typename Find, typename Replace>
struct Find_and_replace_all;
enum class Find_and_replace_target : bool {
  self = false,
  container = true,
};
template <typename NotFound, typename Find, typename Replace>
requires(!std::same_as<NotFound, Find>)
    // NOLINTNEXTLINE(altera-struct-pack-align)
    struct Find_and_replace_all<NotFound, Find, Replace> {
  using type = NotFound;
};
template <typename Find, typename Replace>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Find_and_replace_all<Find, Find, Replace> {
  using type = Replace;
};
template <template <Find_and_replace_target> typename Find, typename Replace>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Find_and_replace_all<Find<Find_and_replace_target::self>,
                            Find<Find_and_replace_target::self>, Replace> {
  using type = Replace;
};
template <template <typename> typename T,
          template <Find_and_replace_target> typename Find, typename Replace>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Find_and_replace_all<T<Find<Find_and_replace_target::container>>,
                            Find<Find_and_replace_target::container>, Replace> {
  using type = Replace;
};
template <template <typename...> typename T, typename Find, typename Replace,
          typename... TArgs>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Find_and_replace_all<T<TArgs...>, Find, Replace> {
  using type = T<typename Find_and_replace_all<TArgs, Find, Replace>::type...>;
};
template <typename T, typename Find, typename Replace>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Find_and_replace_all<T *, Find, Replace> {
  using type = typename Find_and_replace_all<T, Find, Replace>::type *;
};
template <typename T, typename Find, typename Replace>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Find_and_replace_all<T &, Find, Replace> {
  using type = typename Find_and_replace_all<T, Find, Replace>::type &;
};
template <typename T, typename Find, typename Replace>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Find_and_replace_all<T &&, Find, Replace> {
  using type = typename Find_and_replace_all<T, Find, Replace>::type &&;
};
} // namespace detail

using Find_and_replace_target = detail::Find_and_replace_target;
template <typename T, typename Find, typename Replace>
using Find_and_replace_all = detail::Find_and_replace_all<T, Find, Replace>;
template <typename T, typename Find, typename Replace>
using Find_and_replace_all_t =
    typename Find_and_replace_all<T, Find, Replace>::type;
template <typename T, template <Find_and_replace_target> typename Find,
          typename Replace>
using Find_and_replace_all_t_t = Find_and_replace_all_t<
    Find_and_replace_all_t<T, Find<Find_and_replace_target::container>,
                           Replace>,
    Find<Find_and_replace_target::self>, Replace>;

template <typename T>
consteval static auto type_name_mbs_data [[nodiscard]] () noexcept {
  return detail::type_name_storage<T>;
}
template <typename T>
consteval static auto type_name_mbs [[nodiscard]] () noexcept {
  return std::string_view{detail::type_name_storage<T>.cbegin(),
                          detail::type_name_storage<T>.cend() -
                              null_terminator_size};
}
template <typename T> static auto type_name [[nodiscard]] () {
  return mbsrtoc8s(type_name_mbs<T>());
}
} // namespace artccel::core::util

#endif
