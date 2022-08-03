#pragma once
#ifndef GUARD_A00D9886_2A47_4517_970B_9387D1B2FD94
#define GUARD_A00D9886_2A47_4517_970B_9387D1B2FD94

#include <cerrno>      // import errno
#include <type_traits> // import std::decay_t

#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core::util {
class ARTCCEL_CORE_EXPORT Errno_guard;

using Errno_t = std::decay_t<decltype(errno)>;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
class Errno_guard {
#pragma clang diagnostic pop
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
