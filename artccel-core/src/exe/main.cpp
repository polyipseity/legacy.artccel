#include <algorithm>
#include <gsl/gsl>
#include <span>
#include <string_view>
#include <vector>

namespace artccel {
auto main(std::span<std::string_view const> arguments [[maybe_unused]]) {
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
