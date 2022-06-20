#include <algorithm>                      // import std::transform
#include <artccel-core/util/encoding.hpp> // import util::mbsrtoc8s
#include <gsl/gsl>                        // import gsl::not_null, gsl::zstring
#include <iostream>    // import std::ios_base::sync_with_stdio
#include <locale>      // import std::locale, std::locale::global
#include <span>        // import std::span
#include <string>      // import std::u8string
#include <string_view> // import std::string_view
#include <utility>     // import std::as_const, std::pair
#include <vector>      // import std::vector

namespace artccel::core {
auto main(std::span<std::string_view const> args) -> int {
  std::ios_base::sync_with_stdio(false);
  std::locale::global(
      std::locale{/*u8*/ ""}); // use user-preferred locale to convert args
  auto const norm_args{[args] {
    std::vector<std::pair<std::u8string const, std::string_view const>> init{};
    init.reserve(args.size());
    for (auto const arg : std::as_const(args)) {
      init.emplace_back(util::mbsrtoc8s(arg), arg);
    }
    return init;
  }()};
  std::locale::global(std::locale{/*u8*/ "C.UTF-8"});
  std::cout.flush();
  std::clog.flush();
  return 0;
}
} // namespace artccel::core

auto main(int argc, gsl::zstring argv[]) -> int {
  return artccel::core::main([args{std::span{argv, argv + argc}}] {
    std::vector<std::string_view> init(args.size());
    std::transform(
        args.begin(), args.end(), init.begin(),
        [](gsl::not_null<gsl::zstring> arg) { return std::string_view{arg}; });
    return init;
  }());
}
