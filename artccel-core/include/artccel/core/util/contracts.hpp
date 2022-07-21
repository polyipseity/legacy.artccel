#ifndef GUARD_EBF8F06E_8088_4FBC_AEBA_8CEB64E4F6AD
#define GUARD_EBF8F06E_8088_4FBC_AEBA_8CEB64E4F6AD
#pragma once

#include <concepts>    // import std::same_as
#include <type_traits> // import std::is_nothrow_constructible_v, std::remove_cv_t
#include <utility>     // import std::forward, std::move

#include "concepts_extras.hpp" // import Brace_convertible_to, Differ_from
#include "utility_extras.hpp"  // import Consteval_t, Delegate

namespace artccel::core::util {
template <typename Type>
concept Validator_c =
    requires(Type const &validator,
             typename std::remove_cv_t<Type>::validate_type const &value) {
  { validator.validate(value) } -> std::same_as<void>;
  { validator.subsumes(validator) } -> std::same_as<bool>;
};
template <Validator_c auto Validator> class Validate;
template <typename Type>
concept Validate_c = requires(std::remove_cv_t<Type> type_norm) {
  std::same_as<decltype(type_norm), Validate<decltype(type_norm)::validator>>;
};

template <Validator_c auto Validator>
class Validate
    : public Delegate<
          typename std::remove_cv_t<decltype(Validator)>::validate_type,
          false> {
public:
  constexpr static auto validator_{Validator};
  using type = typename Validate::type;

  consteval Validate(Consteval_t tag [[maybe_unused]],
                     type value) noexcept(noexcept(Validate{std::move(value)}))
      : Validate{std::move(value)} {}
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  constexpr Validate(type value) noexcept(
      std::is_nothrow_constructible_v<typename Validate::Delegate,
                                      decltype(std::move(value))>
          &&noexcept(Validator.validate(this->value_)))
      : Validate::Delegate{std::move(value)} {
    Validator.validate(this->value_);
  }
  template <typename Other>
  requires Differ_from<std::remove_cvref_t<Other>, Validate> &&
      Validate_c<std::remove_cvref_t<Other>>
  constexpr Validate(Other &&other) = delete;
  template <typename Other>
  requires Differ_from<std::remove_cvref_t<Other>, Validate> &&
      Validate_c<std::remove_cvref_t<Other>> &&
      (Validator.subsumes(Other::validator_)) constexpr Validate(
          Other &&
              other) noexcept(std::
                                  is_nothrow_constructible_v<
                                      typename Validate::Delegate,
                                      decltype(type{
                                          std::forward<Other>(other).value_})>)

      : Validate::Delegate{type{std::forward<Other>(other).value_}} {}
};
} // namespace artccel::core::util

#endif
