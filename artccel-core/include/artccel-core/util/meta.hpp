#ifndef ARTCCEL_CORE_UTIL_META_HPP
#define ARTCCEL_CORE_UTIL_META_HPP
#pragma once

#include "containers_extras.hpp" // import f::const_array
#include "encoding.hpp"          // import f::utf8_compat_as_utf8_array
#include "semantics.hpp"         // import null_terminator_size
#include <algorithm>             // import std::ranges::copy
#include <array> // import std::array, std::begin, std::cbegin, std::size, std::to_array
#include <concepts>    // import std::same_as
#include <cstddef>     // import std::size_t
#include <string_view> // import std::string_view, std::u8string_view
#include <utility>     // import std::move

namespace artccel::core::util {
namespace detail {
template <typename T /* needs to be named */>
consteval static auto raw_type_name [[nodiscard]] () -> std::string_view {
// internal linkage as it may be different
// propagate internal linkage to callers if necessary
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#ifdef __GNUC__ // GCC, Clang, ICC
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  return __PRETTY_FUNCTION__;
#elif defined _MSC_VER // MSVC // TODO: C++23: #elifdef
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  return __FUNCSIG__;
#else
  return /*u8*/ "";
#endif
#pragma clang diagnostic pop
}
constexpr static struct {
private:
  std::string_view const control_type_name_{/*u8*/ "char32_t"};
  std::string_view const control_{raw_type_name<char32_t>()};

public:
  std::size_t const junk_prefix_{control_.find(control_type_name_)};
  std::size_t const junk_suffix_{std::size(control_) - junk_prefix_ -
                                 std::size(control_type_name_)};
#pragma warning(suppress : 4324)
} type_name_format{};
template <typename T>
constexpr static auto type_name_storage{[] {
  if constexpr (type_name_format.junk_prefix_ == std::string_view::npos) {
    return std::to_array(/*u8*/ "<type name unavailable>");
  } else {
    constexpr auto type_name{[] {
      auto init{raw_type_name<T>()};
      init.remove_prefix(type_name_format.junk_prefix_);
      init.remove_suffix(type_name_format.junk_suffix_);
      return init;
    }()};
    std::array<char, std::size(type_name) + null_terminator_size> init{};
    std::ranges::copy(type_name, std::begin(init));
    return f::const_array(std::move(init));
  }
}()};

template <typename T, typename Find, typename Replace> struct Replace_all;
enum struct Replace_target : bool { self = false, container = true };
template <typename NotFound, typename Find, typename Replace>
requires(
    !std::same_as<NotFound, Find>) struct Replace_all<NotFound, Find, Replace> {
  using type = NotFound;
};
template <typename Find, typename Replace>
struct Replace_all<Find, Find, Replace> {
  using type = Replace;
};
template <template <Replace_target> typename Find, typename Replace>
struct Replace_all<Find<Replace_target::self>, Find<Replace_target::self>,
                   Replace> {
  using type = Replace;
};
template <template <typename> typename T,
          template <Replace_target> typename Find, typename Replace>
struct Replace_all<T<Find<Replace_target::container>>,
                   Find<Replace_target::container>, Replace> {
  using type = Replace;
};
template <template <typename...> typename T, typename Find, typename Replace,
          typename... TArgs>
struct Replace_all<T<TArgs...>, Find, Replace> {
  using type = T<typename Replace_all<TArgs, Find, Replace>::type...>;
};
template <typename T, typename Find, typename Replace>
struct Replace_all<T *, Find, Replace> {
  using type = typename Replace_all<T, Find, Replace>::type *;
};
template <typename T, typename Find, typename Replace>
struct Replace_all<T &, Find, Replace> {
  using type = typename Replace_all<T, Find, Replace>::type &;
};
template <typename T, typename Find, typename Replace>
struct Replace_all<T &&, Find, Replace> {
  using type = typename Replace_all<T, Find, Replace>::type &&;
};
} // namespace detail

using Replace_target = detail::Replace_target;
template <typename T, typename Find, typename Replace>
using Replace_all = detail::Replace_all<T, Find, Replace>;
template <typename T, typename Find, typename Replace>
using Replace_all_t = typename Replace_all<T, Find, Replace>::type;
template <typename T, template <Replace_target> typename Find, typename Replace>
using Replace_all_t_t =
    Replace_all_t<Replace_all_t<T, Find<Replace_target::container>, Replace>,
                  Find<Replace_target::self>, Replace>;

namespace f {
template <typename T>
consteval static auto type_name_loc_enc_array [[nodiscard]] () noexcept
    -> auto & {
  return detail::type_name_storage<T>;
}
template <typename T> consteval static auto type_name_loc_enc [[nodiscard]] () {
  return std::string_view{std::cbegin(f::type_name_loc_enc_array<T>()),
                          std::size(f::type_name_loc_enc_array<T>()) -
                              null_terminator_size};
}
template <typename T>
consteval static auto type_name_array [[nodiscard]] () noexcept -> auto & {
  // better hope that the literal encoding is UTF-8
  return f::utf8_compat_as_utf8_array<Template_string{
      f::type_name_loc_enc_array<T>()}>();
}
template <typename T> consteval static auto type_name [[nodiscard]] () {
  return std::u8string_view{std::cbegin(f::type_name_array<T>()),
                            std::size(f::type_name_array<T>()) -
                                null_terminator_size};
}
} // namespace f
} // namespace artccel::core::util

#endif
