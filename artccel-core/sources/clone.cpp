#include <artccel-core/util/clone.hpp> // interface

#include <artccel-core/export.h>      // import ARTCCEL_CORE_EXPORT_DEFINITION
#include <artccel-core/util/meta.hpp> // import Replace_target

namespace artccel::core::util {
template struct ARTCCEL_CORE_EXPORT_DEFINITION
    Clone_auto_deleter_t<Replace_target::self>;
template struct ARTCCEL_CORE_EXPORT_DEFINITION
    Clone_auto_deleter_t<Replace_target::container>;
template struct ARTCCEL_CORE_EXPORT_DEFINITION
    Clone_auto_element_t<Replace_target::self>;
template struct ARTCCEL_CORE_EXPORT_DEFINITION
    Clone_auto_element_t<Replace_target::container>;
} // namespace artccel::core::util
