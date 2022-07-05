#ifndef ARTCCEL_CORE_MAIN_HOOKS_HPP
#define ARTCCEL_CORE_MAIN_HOOKS_HPP
#pragma once

#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT
#include <exception>             // import std::exception_ptr
#include <functional>            // import std::function, std::reference_wrapper
#include <gsl/gsl>               // import gsl::czstring, gsl::not_null
#include <optional>              // import std::optional
#include <span>                  // import std::span
#include <string>                // import std::u8string
#include <string_view>           // import std::string_view, std::u8string_view
#include <variant>               // import std::variant
#include <vector>                // import std::vector

namespace artccel::core {
class ARTCCEL_CORE_EXPORT Main_program;
class ARTCCEL_CORE_EXPORT Argument;

using Raw_arguments = std::span<std::string_view const>;

namespace f {
ARTCCEL_CORE_EXPORT auto safe_main(
    std::function<int(Raw_arguments)> const &main_func, int argc,
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    gsl::czstring const argv[]) -> int;
} // namespace f

class Main_program {
private:
  struct {
  } early_init_ [[no_unique_address, maybe_unused]];
  std::reference_wrapper<std::exception_ptr> destructor_exc_out_;
  std::vector<Argument> arguments_;

public:
  explicit Main_program(std::exception_ptr &destructor_exc_out,
                        Raw_arguments arguments);
  ~Main_program() noexcept;
  auto arguments [[nodiscard]] () const -> std::span<Argument const>;

  Main_program(Main_program const &) = delete;
  auto operator=(Main_program const &) = delete;
  Main_program(Main_program &&) = delete;
  auto operator=(Main_program &&) = delete;
};
class Argument {
private:
  std::string_view verbatim_;
  std::variant<std::u8string, gsl::not_null<std::exception_ptr>> utf8_;

public:
  constexpr Argument() noexcept = default;
  explicit Argument(std::string_view argument);
  auto verbatim [[nodiscard]] () const noexcept -> std::string_view;
  auto utf8 [[nodiscard]] () const -> std::optional<std::u8string_view>;
  auto utf8_or_exc [[nodiscard]] () const
      -> std::variant<std::u8string_view, gsl::not_null<std::exception_ptr>>;
};
} // namespace artccel::core

#endif
