#ifndef ARTCCEL_CORE_MAIN_HOOKS_HPP
#define ARTCCEL_CORE_MAIN_HOOKS_HPP
#pragma once

#include <concepts>   // import std::invocable
#include <exception>  // import std::exception_ptr
#include <functional> // import std::function
#include <memory>     // import std::make_shared, std::shared_ptr, std::weak_ptr
#include <span>       // import std::span
#include <string>     // import std::u8string
#include <string_view> // import std::string_view, std::u8string_view
#include <utility>     // import std::forward
#include <variant>     // import std::variant
#include <vector>      // import std::vector

#pragma warning(push)
#pragma warning(disable : 4626 4820)
#include <gsl/gsl> // import gsl::cwzstring, gsl::czstring, gsl::final_action
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable : 4582 4583 4625 4626 4820 5026 5027)
#include <tl/expected.hpp> // import tl::expected
#pragma warning(pop)

#include "util/error_handling.hpp" // import util::Exception_error
#include <artccel/core/export.h>   // import ARTCCEL_CORE_EXPORT

namespace artccel::core {
class ARTCCEL_CORE_EXPORT Main_program;
class ARTCCEL_CORE_EXPORT Argument;

using Raw_arguments = std::span<std::string_view const>;

namespace detail {
#ifdef _WIN32
using Argv_string_t = gsl::cwzstring;
#else
using Argv_string_t = gsl::czstring;
#endif
} // namespace detail

namespace f {
// TODO: C++23: std::move_only_function
ARTCCEL_CORE_EXPORT auto safe_main(
    std::function<int(Raw_arguments)> const &main_func, int argc,
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    detail::Argv_string_t const argv[]) -> int;
} // namespace f

class Main_program {
public:
  using copyable_finalizer_type = std::shared_ptr<gsl::final_action<
      std::function<void()>>>; // TODO: C++23: use std::move_only_function
  using destructor_exceptions_out_type = std::vector<std::exception_ptr>;
  static auto make_copyable_finalizer(std::invocable<> auto &&finalizer) {
    return std::make_shared<typename copyable_finalizer_type::element_type>(
        std::forward<decltype(finalizer)>(finalizer));
  }

private:
#pragma warning(push)
#pragma warning(disable : 4251)
  copyable_finalizer_type early_structor_;
  std::vector<Argument> arguments_;
  copyable_finalizer_type late_structor_;
#pragma warning(pop)

public:
  explicit Main_program(
      std::weak_ptr<destructor_exceptions_out_type> destructor_excs_out,
      Raw_arguments arguments);
  ~Main_program() noexcept;
  auto arguments [[nodiscard]] () const -> std::span<Argument const>;

  Main_program(Main_program const &) = delete;
  auto operator=(Main_program const &) = delete;
  Main_program(Main_program &&) = delete;
  auto operator=(Main_program &&) = delete;
};
// NOLINTNEXTLINE(bugprone-exception-escape): should work, dumb time waster
class Argument {
private:
#pragma warning(push)
#pragma warning(disable : 4251)
  std::string_view verbatim_{};
  tl::expected<std::u8string, util::Exception_error> utf8_{};
#pragma warning(pop)

public:
  constexpr Argument() noexcept = default;
  explicit Argument(std::string_view argument);
  auto verbatim [[nodiscard]] () const noexcept -> std::string_view;
  auto utf8 [[nodiscard]] () const
      -> tl::expected<std::u8string_view, util::Exception_error>;
};
} // namespace artccel::core

#endif
