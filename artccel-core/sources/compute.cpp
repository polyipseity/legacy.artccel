#include <artccel-core/compute/compute.hpp> // interface

#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT_DEFINITION
#include <artccel-core/util/reflect.hpp> // import util::f::type_name
#include <string_view>                   // import std::u8string_view

namespace artccel::core::compute::detail {
template <typename T>
constexpr static auto Odr_type_name_init{util::f::type_name<T>()};
template <typename T>
constinit std::u8string_view const Odr_type_name<T>::value{
    Odr_type_name_init<T>};
template struct ARTCCEL_CORE_EXPORT_DEFINITION Odr_type_name<Compute_option>;
} // namespace artccel::core::compute::detail
