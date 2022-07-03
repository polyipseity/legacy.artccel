#pragma warning(push)
#pragma warning(disable : 4820)
#include <artccel-core/main_hooks.hpp> // import Arguments_t, artccel::core::f::safe_main, f::main_cleanup, f::main_setup
#include <gsl/gsl>                     // import gsl::final_action
#pragma warning(pop)

namespace artccel::core::detail {
static auto main_0(Arguments_t args) -> int {
  [[maybe_unused]] auto const [norm_args]{f::main_setup(args)};
  gsl::final_action const finalizer{[args] {
    [[maybe_unused]] auto const [placeholder]{f::main_cleanup(args)};
  }};
  return 0;
}
} // namespace artccel::core::detail

auto main(int argc, gsl::zstring argv[]) -> int {
  return artccel::core::f::safe_main(artccel::core::detail::main_0, argc, argv);
}
