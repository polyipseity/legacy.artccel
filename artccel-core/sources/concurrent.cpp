#include <artccel-core/util/concurrent.hpp> // interface

#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT_DEFINITION
#include <mutex> // import std::mutex, std::recursive_mutex, std::recursive_timed_mutex, std::timed_mutex
#include <shared_mutex> // import std::shared_mutex, std::shared_timed_mutex

namespace artccel::core::util {
#pragma warning(push)
#pragma warning(disable : 4251)
template class ARTCCEL_CORE_EXPORT_DEFINITION Nullable_lockable<std::mutex>;
ARTCCEL_CORE_EXPORT_DEFINITION constexpr
    typename Nullable_lockable<std::mutex>::null_lockable_type
        Nullable_lockable<std::mutex>::null_lockable_;

template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::timed_mutex>;
ARTCCEL_CORE_EXPORT_DEFINITION constexpr
    typename Nullable_lockable<std::timed_mutex>::null_lockable_type
        Nullable_lockable<std::timed_mutex>::null_lockable_;

template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::recursive_mutex>;
ARTCCEL_CORE_EXPORT_DEFINITION constexpr
    typename Nullable_lockable<std::recursive_mutex>::null_lockable_type
        Nullable_lockable<std::recursive_mutex>::null_lockable_;

template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::recursive_timed_mutex>;
ARTCCEL_CORE_EXPORT_DEFINITION constexpr
    typename Nullable_lockable<std::recursive_timed_mutex>::null_lockable_type
        Nullable_lockable<std::recursive_timed_mutex>::null_lockable_;

template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::shared_mutex>;
ARTCCEL_CORE_EXPORT_DEFINITION constexpr
    typename Nullable_lockable<std::shared_mutex>::null_lockable_type
        Nullable_lockable<std::shared_mutex>::null_lockable_;

template class ARTCCEL_CORE_EXPORT_DEFINITION
    Nullable_lockable<std::shared_timed_mutex>;
ARTCCEL_CORE_EXPORT_DEFINITION constexpr
    typename Nullable_lockable<std::shared_timed_mutex>::null_lockable_type
        Nullable_lockable<std::shared_timed_mutex>::null_lockable_;
#pragma warning(pop)
} // namespace artccel::core::util
