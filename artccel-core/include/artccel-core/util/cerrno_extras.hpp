#ifndef ARTCCEL_CORE_UTIL_CERRNO_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CERRNO_EXTRAS_HPP
#pragma once

#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT
#include <cerrno>                // import errno

namespace artccel::core::util {
class ARTCCEL_CORE_EXPORT Errno_guard;

namespace detail {
static inline auto get_errno() noexcept { return errno; }
} // namespace detail
using Errno_t = decltype(detail::get_errno());

class Errno_guard {
private:
  Errno_t err_no_{errno};
  bool released_{false};

public:
  explicit Errno_guard() noexcept;
  ~Errno_guard() noexcept;
  auto err_no [[nodiscard]] () const noexcept -> Errno_t;
  auto release() noexcept -> Errno_t;

  Errno_guard(Errno_guard const &other) = delete;
  auto operator=(Errno_guard const &right) = delete;
  Errno_guard(Errno_guard &&other) = delete;
  auto operator=(Errno_guard &&right) = delete;
#pragma warning(suppress : 4820)
};
} // namespace artccel::core::util

#endif
