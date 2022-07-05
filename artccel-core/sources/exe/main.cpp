#include <algorithm>                   // import std::ranges::for_each
#include <artccel-core/main_hooks.hpp> // import Argument::verbatim, Main_program, Raw_arguments, artccel::core::f::safe_main
#include <artccel-core/util/encoding.hpp> // import util::literals::encoding::operator""_as_utf8_compat, util::f::utf8_as_utf8_compat
#include <artccel-core/util/exception_extras.hpp> // import util::Rethrow_on_lexical_scope_exit
#include <artccel-core/util/meta.hpp> // import util::Template_string, util::f::type_name_array
#include <artccel-core/util/semantics.hpp> // import util::null_terminator_size
#include <artccel-core/util/utility_extras.hpp> // import util::Overloader
#include <exception>   // import std::exception, std::exception_ptr
#include <gsl/gsl>     // import gsl::index, gsl::not_null
#include <iostream>    // import std::cout
#include <string_view> // import std::u8string_view
#include <variant>     // import std::visit

namespace artccel::core::detail {
using util::literals::encoding::operator""_as_utf8_compat;

static auto main_0(Raw_arguments arguments) -> int {
  util::Rethrow_on_lexical_scope_exit lexical_rethrower{};
  Main_program const program{lexical_rethrower.write(), arguments};
  std::cout << u8"arguments:\n"_as_utf8_compat;
  std::ranges::for_each(program.arguments(), [index{gsl::index{0}}](
                                                 auto arg) mutable {
    std::cout << u8"|- argument "_as_utf8_compat << index++
              << u8":\n"_as_utf8_compat;
    std::cout << u8" |- verbatim: "_as_utf8_compat << arg.verbatim()
              << u8'\n'_as_utf8_compat;
    std::visit(
        util::Overloader{
            [](std::u8string_view u8arg) {
              std::cout << u8" |- UTF-8: "_as_utf8_compat
                        << util::f::utf8_as_utf8_compat(u8arg)
                        << u8'\n'_as_utf8_compat;
            },
            [](gsl::not_null<std::exception_ptr const> const &exc_ptr) {
              std::cout << u8" |- UTF-8: (exception)\n"_as_utf8_compat;
              try {
                std::rethrow_exception(exc_ptr);
              } catch (std::exception const &exc) {
                std::cout << u8"  |- "_as_utf8_compat
                          << util::f::utf8_as_utf8_compat<util::Template_string{
                                 util::f::type_name_array<std::exception>()}>()
                          << u8": "_as_utf8_compat << exc.what()
                          << u8'\n'_as_utf8_compat;
              } catch (...) {
                std::cout
                    << u8"  |- (unknown type): (unavailable)\n"_as_utf8_compat;
              }
            }},
        arg.utf8_or_exc());
  });
  std::cout.flush();
  return 0;
}
} // namespace artccel::core::detail

auto main(int argc, gsl::zstring argv[]) -> int {
  return artccel::core::f::safe_main(artccel::core::detail::main_0, argc, argv);
}
