#ifndef ARTCCEL_CORE_UTIL_META_HPP
#define ARTCCEL_CORE_UTIL_META_HPP
#pragma once

#include <concepts> // import std::same_as

namespace artccel::core::util {
template <typename T, typename Find, typename Replace> struct Replace_all;
enum struct Replace_target : bool { self = false, container = true };
template <typename T, typename Find, typename Replace>
using Replace_all_t = typename Replace_all<T, Find, Replace>::type;
template <typename T, template <Replace_target> typename Find, typename Replace>
using Replace_all_t_t =
    Replace_all_t<Replace_all_t<T, Find<Replace_target::container>, Replace>,
                  Find<Replace_target::self>, Replace>;

template <typename NotFound, typename Find, typename Replace>
requires(
    !std::same_as<NotFound, Find>) struct Replace_all<NotFound, Find, Replace> {
  using type = NotFound;
};
template <typename Found, typename Replace>
struct Replace_all<Found, Found, Replace> {
  using type = Replace;
};
template <template <Replace_target> typename Found, typename Replace>
struct Replace_all<Found<Replace_target::self>, Found<Replace_target::self>,
                   Replace> {
  using type = Replace;
};
template <template <typename> typename Container,
          template <Replace_target> typename Found, typename Replace>
struct Replace_all<Container<Found<Replace_target::container>>,
                   Found<Replace_target::container>, Replace> {
  using type = Replace;
};
template <template <typename...> typename T, typename Find, typename Replace,
          typename... TArgs>
struct Replace_all<T<TArgs...>, Find, Replace> {
  using type = T<Replace_all_t<TArgs, Find, Replace>...>;
};
template <typename T, typename Find, typename Replace>
struct Replace_all<T *, Find, Replace> {
  using type = Replace_all_t<T, Find, Replace> *;
};
template <typename T, typename Find, typename Replace>
struct Replace_all<T &, Find, Replace> {
  using type = Replace_all_t<T, Find, Replace> &;
};
template <typename T, typename Find, typename Replace>
struct Replace_all<T &&, Find, Replace> {
  using type = Replace_all_t<T, Find, Replace> &&;
};
} // namespace artccel::core::util

#endif
