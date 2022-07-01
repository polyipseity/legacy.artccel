#include <artccel-core/compute/compute.hpp> // interface

#include <artccel-core/util/meta.hpp> // import util::f::type_name
#include <string_view>                // import std::u8string_view

namespace artccel::core::compute::detail {
template <typename T>
constexpr std::u8string_view Odr_type_name<T>::value{util::f::type_name<T>()};
template struct Odr_type_name<Compute_option>;
} // namespace artccel::core::compute::detail
