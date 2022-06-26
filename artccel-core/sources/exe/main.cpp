#include <algorithm>                               // import std::transform
#include <artccel-core/util/containers_extras.hpp> // import artccel::core::util::f::const_span
#include <artccel-core/util/encoding.hpp> // import util::f::loc_enc_to_utf8
#include <gsl/gsl>                        // import gsl::not_null, gsl::zstring
#include <iostream>    // import std::ios_base::sync_with_stdio
#include <locale>      // import std::locale, std::locale::global
#include <span>        // import std::begin, std::cbegin, std::cend, std::span
#include <string>      // import std::u8string
#include <string_view> // import std::string_view
#include <utility>     // import std::as_const, std::pair
#include <vector>      // import std::vector

namespace artccel::core::f {
auto main(std::span<std::string_view const> args) -> int {
  std::ios_base::sync_with_stdio(false);
  std::locale::global(
      std::locale{/*u8*/ ""}); // use user-preferred locale to convert args
  auto const norm_args{[args] {
    std::vector<std::pair<std::u8string const, std::string_view const>> init{};
    init.reserve(args.size());
    for (auto const arg : std::as_const(args)) {
      init.emplace_back(util::f::loc_enc_to_utf8(arg), arg);
    }
    return init;
  }()};
  std::locale::global(std::locale{/*u8*/ "C.UTF-8"});
  std::cout.flush();
  std::clog.flush();
  return 0;
}
} // namespace artccel::core::f

auto main(int argc, gsl::zstring argv[]) -> int {
  return artccel::core::f::main(
      [args{artccel::core::util::f::const_span(argv, argv + argc)}] {
        std::vector<std::string_view> init(args.size());
        std::transform(std::cbegin(args), std::cend(args), std::begin(init),
                       [](gsl::not_null<gsl::zstring> arg) {
                         return std::string_view{arg};
                       });
        return init;
      }());
}
