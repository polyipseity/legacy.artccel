#ifndef ARTCCEL_CORE_UTIL_ERROR_HANDLING_HPP
#define ARTCCEL_CORE_UTIL_ERROR_HANDLING_HPP
#pragma once

#pragma clang diagnostic push // suppress <tl/expected.hpp>
#pragma clang diagnostic ignored "-Wpadded"
#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT_DECLARATION
#include <cassert>               // import assert
#include <exception>       // import std::exception_ptr, std::rethrow_exception
#include <gsl/gsl>         // import gsl::not_null
#include <tl/expected.hpp> // import tl::expected, tl::unexpect
#include <utility>         // import std::move
#include <variant>         // import std::monostate
#pragma clang diagnostic pop

namespace artccel::core::util {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
template <typename Error> struct Error_with_exception {
#pragma clang diagnostic pop
  gsl::not_null<std::exception_ptr> exc_ptr_;
  Error error_ [[no_unique_address, msvc::no_unique_address]]{};
  template <typename Ret>
  friend auto
  flatten(tl::expected<Ret, Error_with_exception> const &result) noexcept {
    using out_type = tl::expected<Ret, Error>;
    if (result) {
      return out_type{*result};
    }
    return out_type{tl::unexpect, result.error()};
  }
  template <typename Ret>
  friend auto
  flatten(tl::expected<Ret, Error_with_exception> &&result) noexcept {
    using out_type = tl::expected<Ret, Error>;
    if (result) {
      return out_type{*std::move(result)};
    }
    return out_type{tl::unexpect, std::move(result).error()};
  }
  template <typename Ret>
  friend auto assert_success(
      tl::expected<Ret, Error_with_exception> const &result) noexcept {
    assert(result && u8"Unexpected failure");
    return *result;
  }
  template <typename Ret>
  friend auto
  assert_success(tl::expected<Ret, Error_with_exception> &&result) noexcept {
    assert(result && u8"Unexpected failure");
    return *std::move(result);
  }
  template <typename Ret>
  friend auto
  success_or_throw(tl::expected<Ret, Error_with_exception> const &result) {
    if (result) {
      return *result;
    }
    std::rethrow_exception(result.error().exc_ptr_);
  }
  template <typename Ret>
  friend auto
  success_or_throw(tl::expected<Ret, Error_with_exception> &&result) {
    if (result) {
      return *std::move(result);
    }
    std::rethrow_exception(std::move(result).error().exc_ptr_);
  }
}; // TODO: C++23: tl::expected does not allow aggregate initialization
extern template struct ARTCCEL_CORE_EXPORT_DECLARATION
    Error_with_exception<std::monostate>;
using Exception_error = Error_with_exception<std::monostate>;
} // namespace artccel::core::util

#endif
