#ifndef ARTCCEL_CORE_MAIN_HOOKS_HPP
#define ARTCCEL_CORE_MAIN_HOOKS_HPP
#pragma once

#include <functional>  // import std::function
#include <gsl/gsl>     // import gsl::czstring
#include <span>        // import std::span
#include <string>      // import std::u8string
#include <string_view> // import std::string_view
#include <utility>     // import std::pairr
#include <vector>      // import std::vector

namespace artccel::core {
struct Main_setup_result;
struct Main_cleanup_result;

using Arguments_t = std::span<std::string_view const>;

namespace f {
auto safe_main(
    std::function<int(Arguments_t)> const &main_func, int argc,
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    gsl::czstring const argv[]) -> int;

auto main_setup(Arguments_t args) -> Main_setup_result;
auto main_cleanup(Arguments_t args) -> Main_cleanup_result;
} // namespace f

// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
struct alignas(32) Main_setup_result {
  std::vector<std::pair<std::u8string const, std::string_view const>>
      normalized_args;
};
struct Main_cleanup_result {
  char8_t placeholder;
};
} // namespace artccel::core

#endif
