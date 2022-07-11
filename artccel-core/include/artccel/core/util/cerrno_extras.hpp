#ifndef ARTCCEL_CORE_UTIL_CERRNO_EXTRAS_HPP
#define ARTCCEL_CORE_UTIL_CERRNO_EXTRAS_HPP
#pragma once

#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT
#include <cerrno>                // import errno
#include <type_traits>           // import std::decay_t

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
