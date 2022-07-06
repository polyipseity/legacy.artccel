#ifndef ARTCCEL_CORE_UTIL_CODECVT_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CODECVT_EXTRAS_HPP
#pragma once

#include "encoding.hpp" // import literals::encoding::operator""_as_utf8_compat
#include "polyfill.hpp" // import f::unreachable
#include "utility_extras.hpp" // import dependent_false_v
#include <concepts>           // import std::derived_from, std::same_as
#include <cwchar>             // import std::mbstate_t
#include <locale>             // import std::codecvt, std::codecvt_base
#include <stdexcept>          // import std::range_error
#include <string> // import std::basic_string, std::data, std::size, std::string
#include <string_view> // import std::basic_string_view
#include <type_traits> // import std::conditional_t

namespace artccel::core::util {
using Codecvt_char_char = std::codecvt<char, char, std::mbstate_t>;
using Codecvt_utf16_utf8 = std::codecvt<char16_t, char8_t, std::mbstate_t>;
using Codecvt_utf32_utf8 = std::codecvt<char32_t, char8_t, std::mbstate_t>;
using Codecvt_wchar_char = std::codecvt<wchar_t, char, std::mbstate_t>;
template <typename T>
concept Codecvt_c = std::derived_from<
    T, std::codecvt<typename T::intern_type, typename T::extern_type,
                    typename T::state_type>>;

namespace detail {
using literals::encoding::operator""_as_utf8_compat;

template <Codecvt_c Codecvt, typename InCharT>
auto codecvt_convert(std::basic_string_view<InCharT> input) {
  using intern_type = typename Codecvt::intern_type;
  using extern_type = typename Codecvt::extern_type;
  constexpr auto int_to_ext{[]() noexcept {
    if constexpr (std::same_as<intern_type, InCharT>) {
      return true;
    } else if constexpr (std::same_as<extern_type, InCharT>) {
      return false;
    } else {
      static_assert(dependent_false_v<InCharT>,
                    u8"Input type is neither intern type nor extern type");
    }
  }()};
  using in_c_t = std::conditional_t<int_to_ext, intern_type, extern_type>;
  using out_c_t = std::conditional_t<int_to_ext, extern_type, intern_type>;

  Codecvt const codecvt{};
  std::basic_string<out_c_t> output(
      (int_to_ext ? codecvt.max_length() : 1) * std::size(input), out_c_t{});

  in_c_t const *input_next{};
  out_c_t *output_next{};
  switch (typename Codecvt::state_type state{};
          (codecvt.*([]() noexcept {
             if constexpr (int_to_ext) {
               return &Codecvt::out;
             } else {
               return &Codecvt::in;
             }
           }()))(state, std::data(input), std::data(input) + std::size(input),
                 input_next, std::data(output),
                 std::data(output) + std::size(output), output_next)) {
  case std::codecvt_base::ok:
    [[fallthrough]];
    [[unlikely]] case std::codecvt_base::noconv : break;
  case std::codecvt_base::error:
    throw std::range_error{std::string{u8"Partial conversion"_as_utf8_compat}};
  case std::codecvt_base::partial:
    throw std::range_error{std::string{u8"Errored conversion"_as_utf8_compat}};
  default:
    f::unreachable();
  }
  if (input_next != std::data(input) + std::size(input)) [[unlikely]] {
    throw std::range_error{
        std::string{u8"Incomplete conversion"_as_utf8_compat}};
  }

  output.resize(output_next - std::data(output));
  return output;
}
} // namespace detail

namespace f {
template <Codecvt_c Codecvt>
auto codecvt_convert_to_extern(
    std::basic_string_view<typename Codecvt::intern_type> intern) {
  return detail::codecvt_convert<Codecvt>(intern);
}
template <Codecvt_c Codecvt>
auto codecvt_convert_to_intern(
    std::basic_string_view<typename Codecvt::extern_type> ext) {
  return detail::codecvt_convert<Codecvt>(ext);
}
} // namespace f
} // namespace artccel::core::util

#endif
