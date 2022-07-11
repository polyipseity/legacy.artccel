#include <artccel-core/util/error_handling.hpp> // interface

#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT_DEFINITION
#include <variant>               // import std::monostate

namespace artccel::core::util {
template struct ARTCCEL_CORE_EXPORT_DEFINITION
    Error_with_exception<std::monostate>;
} // namespace artccel::core::util
