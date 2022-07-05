#ifndef ARTCCEL_CORE_UTIL_META_HPP
#define ARTCCEL_CORE_UTIL_META_HPP
#pragma once

#include <concepts> // import std::same_as

namespace artccel::core::util {
namespace detail {
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
} // namespace artccel::core::util

#endif
