#include <algorithm>                 // import std::transform
#include <artccel-core/encoding.hpp> // import util::mbsrtoc8s
#include <gsl/gsl>                   // import gsl::zstring
#include <iostream>                  // import std::ios_base::sync_with_stdio
#include <locale>                    // import std::locale, std::locale::global
#include <span>                      // import std::span
#include <string>                    // import std::u8string
#include <string_view>               // import std::string_view
#include <utility>                   // import std::pair
#include <vector>                    // import std::vector

namespace artccel::core {
auto main(std::span<std::string_view const> args) {
  std::ios_base::sync_with_stdio(false);
  std::locale::global(
      std::locale{""}); // use user-preferred locale to convert args
  auto const norm_args{[args]() {
    std::vector<std::pair<std::u8string const, std::string_view const>> init{};
    init.reserve(args.size());
    for (auto const arg : args) {
      init.emplace_back(util::mbsrtoc8s(arg), arg);
    }
    return init;
  }()};
  std::locale::global(std::locale{"C.UTF-8"});
  return 0;
}
} // namespace artccel::core

auto main(int argc, gsl::zstring argv[]) -> int {
  // clang-format off
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
  /* clang-format on */ return artccel::core::main([argc, argv]() {
    std::vector<std::string_view> init(argc);
    std::transform(argv, argv + argc, init.begin(),
                   [](gsl::zstring arg) { return std::string_view{arg}; });
    return init;
  }());
}
