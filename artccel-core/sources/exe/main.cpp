#include <algorithm>   // import std::ranges::for_each
#include <exception>   // import std::exception, std::rethrow_exception
#include <iostream>    // import std::cin, std::cout, std::flush
#include <memory>      // import std::make_shared
#include <string>      // import std::u8string
#include <string_view> // import std::u8string_view

#pragma warning(push)
#pragma warning(disable : 4820)
#include <gsl/gsl> // import gsl::final_action, gsl::index, gsl::wzstring, gsl::zstring
#pragma warning(pop)

#include <artccel/core/main_hooks.hpp> // import Argument::verbatim, Main_program, Raw_arguments, artccel::core::f::safe_main
#include <artccel/core/util/encoding.hpp> // import util::f::getline_utf8, util::f::utf8_as_utf8_compat, util::literals::encoding::operator""_as_utf8_compat, util::operators::utf8_compat::ostream::operator<<
#include <artccel/core/util/meta.hpp>     // import util::Template_string
#include <artccel/core/util/reflect.hpp>  // import util::f::type_name_array
#include <artccel/core/util/semantics.hpp> // import util::null_terminator_size

namespace artccel::core::detail {
using util::literals::encoding::operator""_as_utf8_compat;
using util::operators::utf8_compat::ostream::operator<<;

static void print_args(Main_program const &program) {
  std::cout << u8"arguments:\n"_as_utf8_compat;
  std::ranges::for_each(program.arguments(), [index{gsl::index{0}}](
                                                 auto const &arg) mutable {
    std::cout << u8"|- argument "_as_utf8_compat << index++
              << u8":\n"_as_utf8_compat;
    std::cout << u8" |- verbatim: "_as_utf8_compat << arg.verbatim()
              << u8'\n'_as_utf8_compat;
    if (auto const u8arg{arg.utf8()}) {
      std::cout << u8" |- UTF-8: "_as_utf8_compat << *u8arg
                << u8'\n'_as_utf8_compat;
    } else {
      std::cout << u8" |- UTF-8: (exception)\n"_as_utf8_compat;
      try {
        std::rethrow_exception(u8arg.error().exc_ptr_);
      } catch (std::exception const &exc) {
        constexpr static auto exception_type_name{
            util::f::utf8_as_utf8_compat<util::Template_string{
                util::f::type_name_array<std::exception>()}>()};
        std::cout << u8"  |- "_as_utf8_compat << exception_type_name
                  << u8": "_as_utf8_compat << exc.what()
                  << u8'\n'_as_utf8_compat;
      } catch (...) {
        std::cout << u8"  |- (unknown type): (unavailable)\n"_as_utf8_compat;
      }
    }
  });
  std::cout.flush();
}

static auto echo_cin() {
  std::cout << u8"echo in: "_as_utf8_compat << std::flush;
  auto const input{[] {
    std::u8string init{};
    util::f::getline_utf8(std::cin, init);
    return init;
  }()};
  std::cout << u8"echo out: "_as_utf8_compat << input << u8'\n'_as_utf8_compat
            << std::flush;
}

static auto main_0(Raw_arguments arguments) -> int {
  auto const program_dtor_excs{std::make_shared<
      typename Main_program::destructor_exceptions_out_type>()};
  gsl::final_action const rethrower{[&program_dtor_excs] {
    std::ranges::for_each(*program_dtor_excs,
                          [](auto exc) { std::rethrow_exception(exc); });
  }};
  Main_program const program{program_dtor_excs, arguments};
  detail::print_args(program);
  detail::echo_cin();
  return 0;
}
} // namespace artccel::core::detail

#ifdef _WIN32
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
auto wmain(int argc, gsl::wzstring argv[]) -> int {
#pragma clang diagnostic pop
#else
auto main(int argc, gsl::zstring argv[]) -> int {
#endif
  return artccel::core::f::safe_main(artccel::core::detail::main_0, argc, argv);
}
