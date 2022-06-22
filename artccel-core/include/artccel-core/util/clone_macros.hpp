#ifndef ARTCCEL_CORE_UTIL_CLONE_MACROS_HPP
#define ARTCCEL_CORE_UTIL_CLONE_MACROS_HPP
#pragma once

#include <functional> // import std::invoke

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARTCCEL_GENERATE_CLONE_FUNCTION_0(ptr_name, expr)                      \
  [](auto const &(ptr_name)) -> decltype(auto) {                               \
    constexpr struct {                                                         \
      constexpr auto operator()                                                \
          [[deprecated, nodiscard]] (decltype(ptr_name)(ptr_name)) const       \
          -> decltype(auto) {                                                  \
        return expr;                                                           \
      }                                                                        \
                                                                               \
    private:                                                                   \
      char const align [[maybe_unused]]{};                                     \
    } func{};                                                                  \
    return std::invoke(func, ptr_name);                                        \
  }
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARTCCEL_GENERATE_CLONE_FUNCTION(ptr_name, expr)                        \
  ARTCCEL_GENERATE_CLONE_FUNCTION_0(ptr_name, expr)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARTCCEL_GENERATE_CLONE_FUNCTION_EXPR(expr)                             \
  ARTCCEL_GENERATE_CLONE_FUNCTION_0(ptr, expr)

#endif
