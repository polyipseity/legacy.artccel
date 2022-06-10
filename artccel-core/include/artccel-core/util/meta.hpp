#ifndef ARTCCEL_CORE_UTIL_META_HPP
#define ARTCCEL_CORE_UTIL_META_HPP
#pragma once

#include <type_traits> // import std::remove_cv_t

namespace artccel::core::util {
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
} // namespace artccel::core::util

#endif
