#pragma warning(push) // suppress <gsl/pointers>, <gsl/util>
#pragma warning(disable : 4626 4820)
#include <artccel-core/main_hooks.hpp> // interface

#include <algorithm> // import std::ranges::transform
#include <artccel-core/util/containers_extras.hpp> // import util::f::const_span
#include <artccel-core/util/encoding.hpp> // import util::f::loc_enc_to_utf8
#include <artccel-core/util/utility_extras.hpp> // import util::dependent_false_v
#include <concepts>                             // import std::same_as
#include <exception>  // import std::current_exception, std::exception_ptr
#include <functional> // import std::function
#include <gsl/gsl>    // import gsl::czstring, gsl::final_action, gsl::not_null
#include <iostream> // import std::clog, std::cout, std::ios_base::sync_with_stdio
#include <locale>   // import std::locale, std::locale::global
#include <optional>    // import std::nullopt, std::optional
#include <span>        // import std::begin, std::size, std::span
#include <string>      // import std::u8string
#include <string_view> // import std::string_view, std::u8string_view
#include <type_traits> /// import std::decay_t
#include <variant>     // import std::get_if, std::variant, std::visit
#include <vector>      // import std::vector
#pragma warning(pop)

namespace artccel::core {
namespace f {
auto safe_main(
    std::function<int(Raw_arguments)> const &main_func, int argc,
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
} // namespace f

Main_program::Main_program(std::exception_ptr &destructor_exc_out,
                           Raw_arguments arguments)
    : early_init_{[] {
        std::ios_base::sync_with_stdio(false);
        return decltype(early_init_){};
      }()},
      destructor_exc_out_{destructor_exc_out}, arguments_{[arguments] {
        decltype(arguments_) init(std::size(arguments));
        auto const prev_loc{std::locale::global(std::locale{
            /*u8*/ ""})}; // use user-preferred locale to convert args
        gsl::final_action const finalizer{
            [&prev_loc] { std::locale::global(prev_loc); }};
        std::ranges::transform(arguments, std::begin(init),
                               [](auto arg) { return Argument{arg}; });
        return init;
      }()} {}
Main_program::~Main_program() noexcept {
  try {
    std::cout.flush();
    std::clog.flush();
  } catch (...) {
    destructor_exc_out_.get() = std::current_exception();
  }
}
auto Main_program::arguments [[nodiscard]] () const
    -> std::span<Argument const> {
  return arguments_;
}

Argument::Argument(std::string_view argument)
    : verbatim_{argument}, utf8_{[argument]() -> decltype(utf8_) {
        std::u8string init{};
        try {
          init = util::f::loc_enc_to_utf8(argument);
        } catch (...) {
          return std::current_exception();
        }
        return init;
      }()} {}
auto Argument::verbatim [[nodiscard]] () const noexcept -> std::string_view {
  return verbatim_;
}
auto Argument::utf8 [[nodiscard]] () const
    -> std::optional<std::u8string_view> {
  if (auto const *val{std::get_if<std::u8string>(&utf8_)}) {
    return std::u8string_view{*val};
  }
  return std::nullopt;
}
auto Argument::utf8_or_exc [[nodiscard]] () const
    -> std::variant<std::u8string_view, gsl::not_null<std::exception_ptr>> {
  using return_type = decltype(utf8_or_exc());
  return std::visit([](auto &&var) -> return_type { return var; }, utf8_);
}
} // namespace artccel::core
