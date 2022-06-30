#include <artccel-core/util/cerrno_extras.hpp> // interface
#include <cerrno>                              // import errno

namespace artccel::core::util {
Errno_guard::Errno_guard() noexcept = default;
auto Errno_guard::err_no [[nodiscard]] () const noexcept -> Errno_t {
  return err_no_;
}
auto Errno_guard::release() noexcept -> Errno_t {
  released_ = true;
  return err_no_;
}
Errno_guard::~Errno_guard() noexcept {
  if (!released_) {
    errno = err_no_;
  }
}
} // namespace artccel::core::util
