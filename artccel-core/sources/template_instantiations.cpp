#include <artccel-core/util/clone.hpp> // import Clone_auto_deleter_t, Clone_auto_element_t
#include <artccel-core/util/meta.hpp> // import Find_and_replace_target

namespace artccel::core::util {
template struct Clone_auto_deleter_t<Find_and_replace_target::self>;
template struct Clone_auto_deleter_t<Find_and_replace_target::container>;
template struct Clone_auto_element_t<Find_and_replace_target::self>;
template struct Clone_auto_element_t<Find_and_replace_target::container>;
} // namespace artccel::core::util
