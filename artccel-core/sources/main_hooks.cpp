#include <artccel-core/main_hooks.hpp> // interface

#include <algorithm> // import std::ranges::for_each, std::ranges::transform
#include <artccel-core/util/containers_extras.hpp> // import util::f::const_span
#include <artccel-core/util/encoding.hpp> // import util::f::loc_enc_to_utf8
#include <functional>                     // import std::function
#include <gsl/gsl> // import gsl::czstring, gsl::final_action, gsl::not_null
#include <iostream> // import std::clog, std::cout, std::ios_base::sync_with_stdio
#include <locale>   // import std::locale, std::locale::global
#include <span>     // import std::begin, std::size, std::span
#include <string_view> // import std::string_view
#include <vector>      // import std::vector

namespace artccel::core::f {
auto safe_main(
    std::function<int(Arguments_t)> const &main_func, int argc,
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    gsl::czstring const argv[]) -> int {
  return main_func([args{util::f::const_span(argv, argv + argc)}] {
    std::vector<std::string_view> init(std::size(args));
    std::ranges::transform(args, std::begin(init),
                           [](gsl::not_null<gsl::czstring const> arg) {
                             return std::string_view{arg};
                           });
    return init;
  }());
}

auto main_setup(Arguments_t args) -> Main_setup_result {
  std::ios_base::sync_with_stdio(false);
  return {[args] {
    std::vector<std::pair<std::u8string const, std::string_view const>> init{};
    init.reserve(std::size(args));
    auto const prev_loc{std::locale::global(
        std::locale{/*u8*/ ""})}; // use user-preferred locale to convert args
    gsl::final_action const finalizer{
        [&prev_loc] { std::locale::global(prev_loc); }};
    std::ranges::for_each(args, [&init](auto arg) {
      init.emplace_back(util::f::loc_enc_to_utf8(arg), arg);
    });
    return init;
  }()};
}
auto main_cleanup(Arguments_t args [[maybe_unused]]) -> Main_cleanup_result {
  std::cout.flush();
  std::clog.flush();
  return {u8'\0'};
}
} // namespace artccel::core::f
