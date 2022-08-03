#pragma once
#ifndef GUARD_DF82087C_C553_4C40_AFF6_0D9AE2D0D59F
#define GUARD_DF82087C_C553_4C40_AFF6_0D9AE2D0D59F

#include <cstddef> // import std::size_t, std::span
#include <string_view> // import std::data, std::size, std::string_view, std::u8string_view
#include <utility> // import std::move

#include "containers.hpp" // import f::const_array, f::static_span
#include "encoding.hpp"   // import f::utf8_compat_as_utf8_array
#include "meta.hpp"       // import Template_string
#include "semantics.hpp"  // import null_terminator_size

namespace artccel::core::util {
namespace detail {
template <typename Type /* needs to be named */>
consteval static auto raw_type_name [[nodiscard]] () -> std::string_view {
// internal linkage as it may be different
// propagate internal linkage to callers if necessary
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#ifdef __GNUC__ // GCC, Clang, ICC
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  return __PRETTY_FUNCTION__;
#elif defined _MSC_VER // MSVC // TODO: C++23: #elifdef
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  return __FUNCSIG__;
#else
  return /*u8*/ "";
#endif
#pragma clang diagnostic pop
}
constexpr static struct {
private:
  std::string_view const control_type_name_{/*u8*/ "char32_t"};
  std::string_view const control_{raw_type_name<char32_t>()};

public:
  std::size_t const junk_prefix_{control_.find(control_type_name_)};
  std::size_t const junk_suffix_{std::size(control_) - junk_prefix_ -
                                 std::size(control_type_name_)};
#pragma warning(suppress : 4324)
} type_name_format{};
template <typename Type>
constexpr static auto type_name_storage{[] {
  if constexpr (type_name_format.junk_prefix_ == std::string_view::npos) {
    return f::to_array_cv(/*u8*/ "<type name unavailable>");
  } else {
    constexpr auto type_name{[] {
      auto init{raw_type_name<Type>()};
      init.remove_prefix(type_name_format.junk_prefix_);
      init.remove_suffix(type_name_format.junk_suffix_);
      return init;
    }()};
    return f::const_array<std::size(type_name) + null_terminator_size>(
        f::static_span<std::size(type_name)>(type_name));
  }
}()};
} // namespace detail

namespace f {
template <typename Type>
consteval static auto type_name_lit_enc_array [[nodiscard]] () noexcept
    -> auto & {
  return detail::type_name_storage<Type>;
}
template <typename Type>
consteval static auto type_name_lit_enc [[nodiscard]] () {
  return std::string_view{std::data(f::type_name_lit_enc_array<Type>()),
                          std::size(f::type_name_lit_enc_array<Type>()) -
                              null_terminator_size};
}
template <typename Type>
consteval static auto type_name_array [[nodiscard]] () noexcept -> auto & {
  // better hope that the literal encoding is UTF-8
  return f::utf8_compat_as_utf8_array<Template_string{
      f::type_name_lit_enc_array<Type>()}>();
}
template <typename Type> consteval static auto type_name [[nodiscard]] () {
  return std::u8string_view{std::data(f::type_name_array<Type>()),
                            std::size(f::type_name_array<Type>()) -
                                null_terminator_size};
}
} // namespace f
} // namespace artccel::core::util

#endif
