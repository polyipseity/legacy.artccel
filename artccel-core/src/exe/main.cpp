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
  for (gsl::index i = 0; i < argc; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    arguments.emplace_back(argv[i]);
  }
  return artccel::main(arguments);
}
