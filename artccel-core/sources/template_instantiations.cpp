#include <artccel-core/util/clone.hpp> // import Clone_auto_deleter_t, Clone_auto_element_t
#include <artccel-core/util/meta.hpp> // import Replace_target

namespace artccel::core::util {
template struct Clone_auto_deleter_t<Replace_target::self>;
template struct Clone_auto_deleter_t<Replace_target::container>;
template struct Clone_auto_element_t<Replace_target::self>;
template struct Clone_auto_element_t<Replace_target::container>;
} // namespace artccel::core::util
