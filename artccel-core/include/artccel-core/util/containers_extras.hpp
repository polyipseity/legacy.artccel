#ifndef ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#pragma once

#include <span>        // import std::span
#include <type_traits> // import std::add_const_t
#include <utility>     // import std::forward

namespace artccel::core::util {
template <typename... Args>
constexpr auto const_span(Args &&...args) noexcept(noexcept(std::span{
    std::forward<Args>(args)...})) {
  std::span const ret{std::forward<Args>(args)...};
  using ret_type = decltype(ret);
  return std::span<std::add_const_t<typename ret_type::element_type>,
                   ret_type::extent>{ret};
}
} // namespace artccel::core::util

#endif
