#ifndef GUARD_E7294948_95F0_4F1A_8194_D729FF88506C
#define GUARD_E7294948_95F0_4F1A_8194_D729FF88506C
#pragma once

#include <cassert>  // import assert
#include <concepts> // import std::default_initializable, std::equality_comparable_with, std::invocable, std::same_as
#include <exception> // import std::exception_ptr, std::make_exception_ptr, std::rethrow_exception
#include <functional>  // import std::invoke
#include <type_traits> // import std::decay_t, std::invoke_result_t, std::is_empty_v, std::remove_cvref_t
#include <utility>     // import std::forward, std::move
#include <variant>     // import std::monostate

#pragma warning(push)
#pragma warning(disable : 4626 4820)
#include <gsl/gsl> // import gsl::strict_not_null
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable : 4582 4583 4625 4626 4820 5026 5027)
#include <tl/expected.hpp> // import tl::expected, tl::unexpect
#pragma warning(pop)

#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT_DECLARATION

namespace artccel::core::util {
template <typename Error = std::monostate> class Error_with_exception;
using Exception_error = Error_with_exception<>;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
template <typename Error> class Error_with_exception {
#pragma clang diagnostic pop
public:
  using error_type = Error;
  using empty_error_type = std::monostate;
  constexpr static auto empty_v{std::same_as<error_type, empty_error_type>};

private:
#pragma warning(suppress : 4251)
  gsl::strict_not_null<std::exception_ptr> exc_ptr_;
  Error error_ [[no_unique_address, msvc::no_unique_address]];

public:
  auto exc_ptr [[nodiscard]] () &noexcept -> auto & { return exc_ptr_; }
  auto exc_ptr [[nodiscard]] () const &noexcept -> auto const & {
    return exc_ptr_;
  }
  auto exc_ptr [[nodiscard]] () &&noexcept { return std::move(exc_ptr_); }
  auto exc_ptr [[nodiscard]] () const &&noexcept { return exc_ptr_; }
  auto error [[nodiscard]] () &noexcept -> auto & { return error_; }
  auto error [[nodiscard]] () const &noexcept -> auto const & { return error_; }
  auto error [[nodiscard]] () &&noexcept(noexcept(Error{std::move(error_)}))
      -> Error {
    return std::move(error_);
  }
  auto error [[nodiscard]] () const &&noexcept(noexcept(Error{error_}))
      -> Error {
    return error_;
  }

  explicit Error_with_exception(
      gsl::strict_not_null<std::exception_ptr> exc_ptr, Error error)
      : exc_ptr_{std::move(exc_ptr)}, error_{std::move(error)} {}
  explicit Error_with_exception(
      gsl::strict_not_null<std::exception_ptr> exc_ptr) requires
      std::is_empty_v<Error> && std::default_initializable<Error>
      : Error_with_exception{std::move(exc_ptr), Error{}} {}
  template <typename Exception>
  requires(!std::same_as<
           std::remove_cvref_t<Exception>,
           std::exception_ptr>) explicit Error_with_exception(Exception &&exc,
                                                              Error error)
      : Error_with_exception{gsl::strict_not_null{std::make_exception_ptr(
                                 std::forward<Exception>(exc))},
                             std::move(error)} {}
  template <typename Exception>
  requires(!std::same_as<std::remove_cvref_t<Exception>, std::exception_ptr>) &&
      std::is_empty_v<Error> &&std::default_initializable<Error>
      // NOLINTNEXTLINE(bugprone-forwarding-reference-overload): constrained
      explicit Error_with_exception(Exception &&exc)
      : Error_with_exception{std::forward<Exception>(exc), Error{}} {}

  template <typename Ret>
  friend auto discard_exc
      [[nodiscard]] (tl::expected<Ret, Error_with_exception> const &result) {
    using out_type = tl::expected<Ret, Error>;
    if (result) {
      return out_type{*result};
    }
    return out_type{tl::unexpect, result.error()};
  }
  template <typename Ret>
  friend auto discard_exc
      [[nodiscard]] (tl::expected<Ret, Error_with_exception> &&result) {
    using out_type = tl::expected<Ret, Error>;
    if (result) {
      return out_type{*std::move(result)};
    }
    return out_type{tl::unexpect, std::move(result).error()};
  }
  template <typename Ret>
  friend auto map_err
      [[nodiscard]] (tl::expected<Ret, Error_with_exception> const &result,
                     std::invocable<Error> auto &&func) {
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
  friend auto map_err
      [[nodiscard]] (tl::expected<Ret, Error_with_exception> &&result,
                     std::invocable<Error> auto &&func) {
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
  requires(!empty_v) friend auto discard_err
      [[nodiscard]] (tl::expected<Ret, Error_with_exception> const &result) {
    return map_err(result, [](Error const &discard [[maybe_unused]]) noexcept {
      return std::monostate{};
    });
  }
  template <typename Ret>
  requires(!empty_v) friend auto discard_err
      [[nodiscard]] (tl::expected<Ret, Error_with_exception> &&result) {
    return map_err(std::move(result),
                   [](Error const &discard [[maybe_unused]]) noexcept {
                     return std::monostate{};
                   });
  }

  template <typename Ret>
  friend auto assert_success(
      tl::expected<Ret, Error_with_exception> const &
          result) noexcept(noexcept(std::decay_t<decltype(*result)>{*result})) {
    assert(result && u8"Unexpected failure");
    return *result;
  }
  template <typename Ret>
  friend auto
  assert_success(tl::expected<Ret, Error_with_exception> &&result) noexcept(
      noexcept(std::decay_t<decltype(*std::move(result))>{
          *std::move(result)})) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(result && u8"Unexpected failure");
    return *std::move(result);
  }
  template <typename Ret>
  friend auto
  success_or_throw(tl::expected<Ret, Error_with_exception> const &result) {
    if (result) {
      return *result;
    }
    std::rethrow_exception(result.error().exc_ptr());
  }
  template <typename Ret>
  friend auto
  success_or_throw(tl::expected<Ret, Error_with_exception> &&result) {
    if (result) {
      return *std::move(result);
    }
    std::rethrow_exception(std::move(result).error().exc_ptr());
  }
#pragma warning(suppress : 4820)
};
extern template class ARTCCEL_CORE_EXPORT_DECLARATION Error_with_exception<>;
static_assert(Exception_error::empty_v, u8"Implementation error");

namespace f {
constexpr auto
expect_zero(std::equality_comparable_with<decltype(0)> auto &&result) {
  using out_type = tl::expected<void, std::remove_cvref_t<decltype(result)>>;
  return result == 0
             ? out_type{}
             : out_type{tl::unexpect, std::forward<decltype(result)>(result)};
}
constexpr auto
expect_nonzero(std::equality_comparable_with<decltype(0)> auto &&result) {
  using out_type = tl::expected<std::remove_cvref_t<decltype(result)>,
                                std::remove_cvref_t<decltype(result)>>;
  return result == 0
             ? out_type{tl::unexpect, std::forward<decltype(result)>(result)}
             : out_type{std::forward<decltype(result)>(result)};
}
template <typename Ret, std::equality_comparable_with<Ret> Invalid>
constexpr auto expect_noninvalid(Ret &&result, Invalid &&invalid) {
  using out_type = tl::expected<std::remove_cvref_t<decltype(result)>,
                                std::remove_cvref_t<decltype(invalid)>>;
  return result == invalid
             ? out_type{tl::unexpect, std::forward<decltype(invalid)>(invalid)}
             : out_type{std::forward<decltype(result)>(result)};
}
} // namespace f
} // namespace artccel::core::util

#endif
