#ifndef ARTCCEL_CORE_UTIL_CONCURRENT_HPP
#define ARTCCEL_CORE_UTIL_CONCURRENT_HPP
#pragma once

#include <chrono> // import std::chrono::duration, std::chrono::time_point

namespace artccel::core::util {
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Null_lockable {
  consteval Null_lockable() noexcept = default;

  // named requirement: BasicLockable

  constexpr void lock() noexcept;
  constexpr void unlock() noexcept;

  // named requirement: BasicLockable <- Lockable

  constexpr auto try_lock [[nodiscard]] () noexcept -> bool;

  // named requirement: BasicLockable <- Lockable <- TimedLockable

  template <typename Rep, typename Period>
  constexpr auto try_lock_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time
                     [[maybe_unused]]) noexcept {
    return true;
  }
  template <typename Clock, typename Duration>
  constexpr auto try_lock_until
      [[nodiscard]] (std::chrono::time_point<Clock, Duration> const &abs_time
                     [[maybe_unused]]) noexcept {
    return true;
  }

  // named requirement: SharedLockable

  constexpr void lock_shared() noexcept;
  constexpr auto try_lock_shared [[nodiscard]] () noexcept -> bool;
  constexpr void unlock_shared() noexcept;

  // named requirement: SharedLockable <- SharedTimedLockable

  template <typename Rep, typename Period>
  constexpr auto try_lock_shared_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time
                     [[maybe_unused]]) noexcept {
    return true;
  }
  template <typename Clock, typename Duration>
  constexpr auto try_lock_shared_until
      [[nodiscard]] (std::chrono::time_point<Clock, Duration> const &abs_time
                     [[maybe_unused]]) noexcept {
    return true;
  }
};
} // namespace artccel::core::util

#endif
