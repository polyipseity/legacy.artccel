#ifndef ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CONTAINERS_EXTRAS_HPP
#pragma once

#include <span>        // import std::span
#include <type_traits> // import std::add_const_t
#include <utility>     // import std::forward

namespace artccel::core::util::f {
template <typename... Args>
constexpr auto const_span
    [[nodiscard]] (Args &&...args) noexcept(noexcept(std::span{
        std::forward<Args>(args)...})) {
  using span_type = decltype(std::span{std::forward<Args>(args)...});
  return std::span<std::add_const_t<typename span_type::element_type>,
                   span_type::extent>{std::forward<Args>(args)...};
}
} // namespace artccel::core::util::f

#endif
