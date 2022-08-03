#pragma once
#ifndef GUARD_02070F53_CF57_421A_A459_103D9FEF0F37
#define GUARD_02070F53_CF57_421A_A459_103D9FEF0F37

#include <cassert>   // import assert
#include <concepts>  // import std::derived_from, std::same_as
#include <cstdint>   // import std::int_fast8_t
#include <cwchar>    // import std::mbstate_t
#include <locale>    // import std::codecvt, std::codecvt_base
#include <stdexcept> // import std::range_error
#include <string> // import std::basic_string, std::data, std::size, std::string
#include <string_view> // import std::basic_string_view
#include <type_traits> // import std::conditional_t, std::remove_cv_t

#pragma warning(push)
#pragma warning(disable : 4582 4583 4625 4626 4820 5026 5027)
#include <tl/expected.hpp> // import tl::expected, tl::unexpect
#pragma warning(pop)

#include "containers.hpp"  // import f::atad
#include "conversions.hpp" // import f::int_unsigned_exact_cast
#include "encoding.hpp" // import literals::encoding::operator""_as_utf8_compat
#include "error_handling.hpp" // import Error_with_exception
#include "polyfill.hpp"       // import f::unreachable
#include "string_extras.hpp"  // import Char_traits_c, Rebind_char_traits_t

namespace artccel::core::util {
using Codecvt_char_char = std::codecvt<char, char, std::mbstate_t>;
using Codecvt_utf16_utf8 = std::codecvt<char16_t, char8_t, std::mbstate_t>;
using Codecvt_utf32_utf8 = std::codecvt<char32_t, char8_t, std::mbstate_t>;
using Codecvt_wchar_char = std::codecvt<wchar_t, char, std::mbstate_t>;
template <typename Type>
concept Codecvt_c = std::derived_from < std::remove_cv_t<Type>,
        std::codecvt < typename std::remove_cv_t<Type>::intern_type,
typename std::remove_cv_t<Type>::extern_type,
    typename std::remove_cv_t<Type>::state_type >>
    ;
enum struct Codecvt_error : std::int_fast8_t;
using Codecvt_error_with_exception = Error_with_exception<Codecvt_error>;

enum struct Codecvt_error : std::int_fast8_t { error, partial };

namespace detail {
using literals::encoding::operator""_as_utf8_compat;

template <Codecvt_c Codecvt, typename InCharT, Char_traits_c Traits>
auto codecvt_convert(std::basic_string_view<InCharT, Traits> input) {
  using intern_type = typename Codecvt::intern_type;
  using extern_type = typename Codecvt::extern_type;
  constexpr auto int_to_ext{[]() noexcept {
    if constexpr (std::same_as<intern_type, InCharT>) {
      return true;
    } else {
      static_assert(std::same_as<extern_type, InCharT>,
                    u8"Input type is neither intern type nor extern type");
      return false;
    }
  }()};
  using in_char = std::conditional_t<int_to_ext, intern_type, extern_type>;
  using out_char = std::conditional_t<int_to_ext, extern_type, intern_type>;
  using out_str =
      std::basic_string<out_char, Rebind_char_traits_t<Traits, out_char>>;
  using return_type = tl::expected<out_str, Codecvt_error_with_exception>;

  Codecvt const codecvt{};
  out_str output((int_to_ext ? assert_success(f::int_unsigned_exact_cast(
                                   codecvt.max_length()))
                             : 1) *
                     std::size(input),
                 out_char{});

  in_char const *input_next{};
  out_char *output_next{};
  switch (typename Codecvt::state_type state{};
          (codecvt.*([]() noexcept {
             if constexpr (int_to_ext) {
               return &Codecvt::out;
             } else {
               return &Codecvt::in;
             }
           }()))(state, std::data(input), f::atad(input), input_next,
                 std::data(output), f::atad(output), output_next)) {
  case std::codecvt_base::ok:
    [[fallthrough]];
    [[unlikely]] case std::codecvt_base::noconv : break;
  case std::codecvt_base::error:
    return return_type{
        tl::unexpect,
        typename return_type::error_type{
            std::range_error{
                std::string{u8"Errored conversion"_as_utf8_compat}},
            Codecvt_error::error}}; // TODO: C++23: tl::in_place is broken
  case std::codecvt_base::partial:
    return return_type{
        tl::unexpect,
        typename return_type::error_type{
            std::range_error{
                std::string{u8"Partial conversion"_as_utf8_compat}},
            Codecvt_error::partial}}; // TODO: C++23: tl::in_place is broken
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
  default:
#pragma clang diagnostic pop
    f::unreachable();
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  assert(input_next == f::atad(input) && u8"Incomplete conversion");

  output.resize(assert_success(
      f::int_unsigned_exact_cast(output_next - std::data(output))));
  return return_type{std::move(output)};
}
} // namespace detail

namespace f {
template <Codecvt_c Codecvt, Char_traits_c Traits>
auto codecvt_convert_to_extern(
    std::basic_string_view<typename Codecvt::intern_type, Traits> intern) {
  return detail::codecvt_convert<Codecvt>(intern);
}
template <Codecvt_c Codecvt, Char_traits_c Traits>
auto codecvt_convert_to_intern(
    std::basic_string_view<typename Codecvt::extern_type, Traits> ext) {
  return detail::codecvt_convert<Codecvt>(ext);
}
} // namespace f
} // namespace artccel::core::util

#endif
