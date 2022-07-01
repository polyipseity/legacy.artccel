#include <artccel-core/util/clone.hpp> // interface, import Clone_auto_deleter_t, Clone_auto_element_t
#include <artccel-core/util/concurrent.hpp> // interface, import Nullable_lockable

#include <artccel-core/export.h>      // import ARTCCEL_CORE_EXPORT_DEFINITION
#include <artccel-core/util/meta.hpp> // import Replace_target
#include <mutex> // import std::mutex, std::recursive_mutex, std::recursive_timed_mutex, std::timed_mutex
#include <shared_mutex> // import std::shared_mutex, std::shared_timed_mutex

namespace artccel::core::util {
template struct ARTCCEL_CORE_EXPORT_DEFINITION
    Clone_auto_deleter_t<Replace_target::self>;
template struct ARTCCEL_CORE_EXPORT_DEFINITION
    Clone_auto_deleter_t<Replace_target::container>;
template struct ARTCCEL_CORE_EXPORT_DEFINITION
    Clone_auto_element_t<Replace_target::self>;
template struct ARTCCEL_CORE_EXPORT_DEFINITION
    Clone_auto_element_t<Replace_target::container>;

template class ARTCCEL_CORE_EXPORT_DEFINITION Nullable_lockable<std::mutex>;
template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::timed_mutex>;
template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::recursive_mutex>;
template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::recursive_timed_mutex>;
template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::shared_mutex>;
template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::shared_timed_mutex>;
} // namespace artccel::core::util
