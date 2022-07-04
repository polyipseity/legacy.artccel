#include <artccel-core/main_hooks.hpp> // import Main_program, Raw_arguments, artccel::core::f::safe_main
#include <artccel-core/util/exception_extras.hpp> // import util::Rethrow_on_destruct

namespace artccel::core::detail {
static auto main_0(Raw_arguments arguments) -> int {
  util::Rethrow_on_destruct rethrow_on_destruct{};
  Main_program const program
      [[maybe_unused]]{rethrow_on_destruct.ptr(), arguments};
  return 0;
}
} // namespace artccel::core::detail

auto main(int argc, gsl::zstring argv[]) -> int {
  return artccel::core::f::safe_main(artccel::core::detail::main_0, argc, argv);
}
