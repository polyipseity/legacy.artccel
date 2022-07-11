#ifndef ARTCCEL_CORE_UTIL_ERROR_HANDLING_HPP
#define ARTCCEL_CORE_UTIL_ERROR_HANDLING_HPP
#pragma once

#pragma clang diagnostic push // suppress <tl/expected.hpp>
#pragma clang diagnostic ignored "-Wpadded"
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT_DECLARATION
#include <cassert>               // import assert
#include <concepts> // import std::default_initializable, std::invocable, std::same_as
#include <exception> // import std::exception_ptr, std::make_exception_ptr, std::rethrow_exception
#include <functional>      // import std::invoke
#include <gsl/gsl>         // import gsl::not_null
#include <tl/expected.hpp> // import tl::expected, tl::unexpect
#include <type_traits> // import std::invoke_result_t, std::is_empty_v, std::is_nothrow_copy_constructible_v, std::is_nothrow_invocable_v, std::is_nothrow_move_constructible_v, std::remove_reference_t
#include <utility>     // import std::forward, std::move
#include <variant>     // import std::monostate
#pragma clang diagnostic pop

namespace artccel::core::util {
template <typename Error = std::monostate> struct Error_with_exception;
using Exception_error = Error_with_exception<>;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
template <typename Error> struct Error_with_exception {
#pragma clang diagnostic pop
  using error_type = Error;
  using empty_error_type = std::monostate;
  constexpr static auto empty_v{std::same_as<error_type, empty_error_type>};

  gsl::not_null<std::exception_ptr> exc_ptr_;
  Error error_ [[no_unique_address, msvc::no_unique_address]];

  explicit Error_with_exception(
      gsl::not_null<std::exception_ptr> exc_ptr,
      Error error) noexcept(std::is_nothrow_move_constructible_v<Error>)
      : exc_ptr_{std::move(exc_ptr)}, error_{std::move(error)} {}
  explicit Error_with_exception(
      gsl::not_null<std::exception_ptr>
          exc_ptr) noexcept(noexcept(Error_with_exception{std::move(exc_ptr),
                                                          Error{}})) requires
      std::is_empty_v<Error> && std::default_initializable<Error>
      : Error_with_exception{std::move(exc_ptr), Error{}} {}
  template <typename Exception>
  requires(!std::same_as<std::remove_reference_t<Exception>, std::exception_ptr>) explicit Error_with_exception(
      Exception &&exc, Error error) noexcept(noexcept(Error_with_exception{
      std::make_exception_ptr(std::forward<Exception>(exc)), std::move(error)}))
      : Error_with_exception{
            std::make_exception_ptr(std::forward<Exception>(exc)),
            std::move(error)} {}
  template <typename Exception>
  requires(
      !std::same_as<std::remove_reference_t<Exception>, std::exception_ptr>) &&
      std::is_empty_v<Error>
          &&std::default_initializable<Error> explicit Error_with_exception(
              Exception &&exc) noexcept(noexcept(Error_with_exception{
              std::forward<Exception>(exc), Error{}}))
      : Error_with_exception{std::forward<Exception>(exc), Error{}} {}

  template <typename Ret>
  friend auto discard_exc [[nodiscard]] (
      tl::expected<Ret, Error_with_exception> const
          &result) noexcept(std::is_nothrow_copy_constructible_v<Ret>
                                &&std::is_nothrow_copy_constructible_v<Error>) {
    using out_type = tl::expected<Ret, Error>;
    if (result) {
      return out_type{*result};
    }
    return out_type{tl::unexpect, result.error()};
  }
  template <typename Ret>
  friend auto discard_exc
      [[nodiscard]] (tl::expected<Ret, Error_with_exception> &&result) noexcept(
          std::is_nothrow_move_constructible_v<Ret>
              &&std::is_nothrow_move_constructible_v<Error>) {
    using out_type = tl::expected<Ret, Error>;
    if (result) {
      return out_type{*std::move(result)};
    }
    return out_type{tl::unexpect, std::move(result).error()};
  }
  template <typename Ret>
  friend auto map_err [[nodiscard]] (
      tl::expected<Ret, Error_with_exception> const &result,
      std::invocable<Error> auto
          &&func) noexcept(std::is_nothrow_invocable_v<decltype(func), Error>
                               &&std::is_nothrow_copy_constructible_v<Ret>
                                   &&std::is_nothrow_copy_constructible_v<Error>
                                       &&std::is_nothrow_copy_constructible_v<
                                           std::invoke_result_t<decltype(func),
                                                                Error>>) {
    using out_err_type = std::invoke_result_t<decltype(func), Error>;
    using out_type = tl::expected<Ret, Error_with_exception<out_err_type>>;
    if (result) {
      return out_type{*result};
    }
    auto [exc_ptr, error]{result.error()};
    // TODO: C++23: tl::in_place is broken
    return out_type{
        tl::unexpect,
        typename out_type::error_type{
            std::move(exc_ptr),
            std::invoke(std::forward<decltype(func)>(func), std::move(error))}};
  }
  template <typename Ret>
  friend auto map_err [[nodiscard]] (
      tl::expected<Ret, Error_with_exception> &&result,
      std::invocable<Error> auto
          &&func) noexcept(std::is_nothrow_invocable_v<decltype(func), Error>
                               &&std::is_nothrow_move_constructible_v<Ret>
                                   &&std::is_nothrow_move_constructible_v<Error>
                                       &&std::is_nothrow_move_constructible_v<
                                           std::invoke_result_t<decltype(func),
                                                                Error>>) {
    using out_err_type = std::invoke_result_t<decltype(func), Error>;
    using out_type = tl::expected<Ret, Error_with_exception<out_err_type>>;
    if (result) {
      return out_type{*std::move(result)};
    }
    auto [exc_ptr, error]{std::move(result).error()};
    // TODO: C++23: tl::in_place is broken
    return out_type{
        tl::unexpect,
        typename out_type::error_type{
            std::move(exc_ptr),
            std::invoke(std::forward<decltype(func)>(func), std::move(error))}};
  }
  template <typename Ret>
  requires(!empty_v) friend auto discard_err [[nodiscard]] (
      tl::expected<Ret, Error_with_exception> const
          &result) noexcept(std::is_nothrow_copy_constructible_v<Ret>) {
    return map_err(result, [](Error const &discard [[maybe_unused]]) noexcept {
      return std::monostate{};
    });
  }
  template <typename Ret>
  requires(!empty_v) friend auto discard_err
      [[nodiscard]] (tl::expected<Ret, Error_with_exception> &&result) noexcept(
          std::is_nothrow_move_constructible_v<Ret>) {
    return map_err(std::move(result),
                   [](Error const &discard [[maybe_unused]]) noexcept {
                     return std::monostate{};
                   });
  }

  template <typename Ret>
  friend auto assert_success(
      tl::expected<Ret, Error_with_exception> const
          &result) noexcept(std::is_nothrow_copy_constructible_v<Ret>) {
    assert(result && u8"Unexpected failure");
    return *result;
  }
  template <typename Ret>
  friend auto
  assert_success(tl::expected<Ret, Error_with_exception> &&result) noexcept(
      std::is_nothrow_move_constructible_v<Ret>) {
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
};
extern template struct ARTCCEL_CORE_EXPORT_DECLARATION Error_with_exception<>;
static_assert(Exception_error::empty_v, u8"Implementation error");
} // namespace artccel::core::util

#endif
