#ifndef ARTCCEL_CORE_UTIL_META_HPP
#define ARTCCEL_CORE_UTIL_META_HPP
#pragma once

#include "encoding.hpp"  // import f::loc_enc_to_utf8
#include "semantics.hpp" // import null_terminator_size
#include <algorithm>     // import std::copy_n
#include <array> // import std::array, std::begin, std::cbegin, std::cend, std::to_array
#include <concepts>    // import std::same_as
#include <cstddef>     // import std::size_t
#include <string_view> // import std::string_view

namespace artccel::core::util {
namespace detail {
template <typename T>
consteval static auto raw_type_name [[nodiscard]] () -> std::string_view {
// internal linkage as it may be different
// propagate internal linkage to callers if necessary
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
private:
  std::string_view const control_type_name_{/*u8*/ "int"};
  std::string_view const control_{raw_type_name<int>()};

public:
  std::size_t const junk_prefix_{control_.find(control_type_name_)};
  std::size_t const junk_suffix_{control_.size() - junk_prefix_ -
                                 control_type_name_.size()};
} type_name_format{};
template <typename T>
constexpr static auto type_name_storage{[] {
  if constexpr (type_name_format.junk_prefix_ == std::string_view::npos) {
    return std::to_array(/*u8*/ "<type name unavailable>");
  } else {
    constexpr std::string_view type_name{
        std::cbegin(raw_type_name<T>()) + type_name_format.junk_prefix_,
        std::cend(raw_type_name<T>()) - type_name_format.junk_suffix_};
    std::array<char, type_name.size() + null_terminator_size> ret{};
    std::copy_n(std::cbegin(type_name), type_name.size(), std::begin(ret));
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

namespace f {
template <typename T>
consteval static auto type_name_loc_enc_data [[nodiscard]] () noexcept {
  return detail::type_name_storage<T>;
}
template <typename T>
consteval static auto type_name_loc_enc [[nodiscard]] () noexcept {
  return std::string_view{std::cbegin(detail::type_name_storage<T>),
                          std::cend(detail::type_name_storage<T>) -
                              null_terminator_size};
}
template <typename T> static auto type_name [[nodiscard]] () {
  return f::loc_enc_to_utf8(type_name_loc_enc<T>());
}
} // namespace f
} // namespace artccel::core::util

#endif
