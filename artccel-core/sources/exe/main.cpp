#include <algorithm>   // import std::transform
#include <clocale>     // import std::setlocale
#include <gsl/gsl>     // import gsl::zstring
#include <iostream>    // import std::ios_base
#include <span>        // import std::span
#include <string_view> // import std::string_view
#include <vector>      // import std::vector

namespace artccel::core {
auto main(std::span<std::string_view const> arguments [[maybe_unused]]) {
  std::setlocale(LC_ALL, "C.UTF-8"); // NOLINT(concurrency-mt-unsafe)
  std::ios_base::sync_with_stdio(false);
  return 0;
}
} // namespace artccel::core

auto main(int argc, gsl::zstring argv[]) -> int {
  std::vector<std::string_view> arguments(argc);
  std::transform(
      argv, argv + argc, arguments.begin(),
      [](gsl::zstring argument) { return std::string_view{argument}; });
  return artccel::core::main(arguments);
}
