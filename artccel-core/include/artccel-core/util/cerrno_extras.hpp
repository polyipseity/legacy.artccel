#ifndef ARTCCEL_CORE_UTIL_CERRNO_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CERRNO_EXTRAS_HPP
#pragma once

#include <cerrno> // import errno

namespace artccel::core::util {
namespace detail {
static auto get_errno() noexcept { return errno; }
} // namespace detail
using Errno_t = decltype(detail::get_errno());

class Errno_guard {
private:
  Errno_t err_no_{errno};
  bool released_{false};

public:
  Errno_guard() noexcept = default;
  ~Errno_guard() noexcept;
  auto err_no [[nodiscard]] () const noexcept -> Errno_t;
  auto release() noexcept -> Errno_t;

  Errno_guard(Errno_guard const &other) = delete;
  auto operator=(Errno_guard const &right) = delete;
  Errno_guard(Errno_guard &&other) = delete;
  auto operator=(Errno_guard &&right) = delete;
};
} // namespace artccel::core::util

#endif
