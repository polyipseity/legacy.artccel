#include <algorithm>
#include <clocale>
#include <gsl/gsl>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

namespace artccel {
auto main(std::span<std::string_view const> arguments [[maybe_unused]]) {
  std::setlocale(LC_ALL, "C.UTF-8"); // NOLINT(concurrency-mt-unsafe)
  std::ios_base::sync_with_stdio(false);
  return 0;
}
} // namespace artccel

auto main(int argc, gsl::zstring argv[]) -> int {
  std::vector<std::string_view> arguments(argc);
  std::transform(
      argv, argv + argc, arguments.begin(),
      [](gsl::zstring argument) { return std::string_view{argument}; });
  return artccel::main(arguments);
}
