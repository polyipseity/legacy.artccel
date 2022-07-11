#include <artccel/core/util/error_handling.hpp> // interface

#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT_DEFINITION

namespace artccel::core::util {
#pragma warning(suppress : 4251)
template struct ARTCCEL_CORE_EXPORT_DEFINITION Error_with_exception<>;
} // namespace artccel::core::util
