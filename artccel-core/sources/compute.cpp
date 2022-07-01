#include <artccel-core/compute/compute.hpp> // interface

#include <artccel-core/util/meta.hpp> // import util::f::type_name
#include <string_view>                // import std::u8string_view

namespace artccel::core::compute::detail {
template <typename T>
constinit std::u8string_view const odr_type_name{util::f::type_name<T>()};
template <>
// NOLINTNEXTLINE(clang-diagnostic-missing-constinit)
constinit std::u8string_view const odr_type_name<Compute_option>;
} // namespace artccel::core::compute::detail
