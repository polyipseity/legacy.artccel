#include <cstdlib> // import EXIT_SUCCESS
#include <memory>  // import std::make_shared

#include <artccel/core/main_hooks.hpp> // import Main_program, Raw_arguments, artccel::core::f::safe_main

namespace detail {
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace artccel::core;

static auto main_0(artccel::core::Raw_arguments arguments) -> int {
  auto const program_dtor_excs{std::make_shared<
      typename Main_program::destructor_exceptions_out_type>()};
  Main_program const program [[maybe_unused]]{arguments, program_dtor_excs};
  return EXIT_SUCCESS;
}
} // namespace detail

#ifdef _WIN32
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
auto wmain(int argc, gsl::wzstring argv[]) -> int {
#pragma clang diagnostic pop
#else
auto main(int argc, gsl::zstring argv[]) -> int {
#endif
  return artccel::core::f::safe_main(detail::main_0, argc, argv);
}
