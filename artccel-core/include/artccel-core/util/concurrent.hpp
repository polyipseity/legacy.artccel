#ifndef ARTCCEL_CORE_UTIL_CONCURRENT_HPP
#define ARTCCEL_CORE_UTIL_CONCURRENT_HPP
#pragma once

#include <chrono>      // import std::chrono::duration, std::chrono::time_point
#include <concepts>    // import std::same_as
#include <memory>      // import std::make_unique, std::unique_ptr
#include <type_traits> // import std::is_nothrow_move_constructible_v
#include <utility>     // import std::move

namespace artccel::core::util {
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Null_lockable {
  consteval Null_lockable() noexcept = default;

  // named requirement: BasicLockable

  constexpr void lock() const noexcept {}
  constexpr void unlock() const noexcept {}

  // named requirement: BasicLockable <- Lockable

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  constexpr auto try_lock [[nodiscard]] () const noexcept -> bool {
    return true;
  }

  // named requirement: BasicLockable <- Lockable <- TimedLockable

  template <typename Rep, typename Period>
  constexpr auto try_lock_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time
                     [[maybe_unused]]) const noexcept -> bool {
    return true;
  }
  template <typename Clock, typename Duration>
  constexpr auto try_lock_until
      [[nodiscard]] (std::chrono::time_point<Clock, Duration> const &abs_time
                     [[maybe_unused]]) const noexcept -> bool {
    return true;
  }

  // named requirement: SharedLockable

  constexpr void lock_shared() const noexcept {}
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  constexpr auto try_lock_shared [[nodiscard]] () const noexcept -> bool {
    return true;
  }
  constexpr void unlock_shared() const noexcept {}

  // named requirement: SharedLockable <- SharedTimedLockable

  template <typename Rep, typename Period>
  constexpr auto try_lock_shared_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time
                     [[maybe_unused]]) const noexcept -> bool {
    return true;
  }
  template <typename Clock, typename Duration>
  constexpr auto try_lock_shared_until
      [[nodiscard]] (std::chrono::time_point<Clock, Duration> const &abs_time
                     [[maybe_unused]]) const noexcept -> bool {
    return true;
  }
};

template <typename L, typename NL = Null_lockable> class Nullable_lockable {
public:
  using lockable_type = L;
  using null_lockable_type = NL;

private:
  constexpr static NL null_lockable{};
  std::unique_ptr<L> lockable_{};

public:
  constexpr Nullable_lockable() noexcept(noexcept(decltype(lockable_){
      std::make_unique<L>()}))
      : lockable_{std::make_unique<L>()} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr Nullable_lockable(std::unique_ptr<L> lockable) noexcept
      : lockable_{std::move(lockable)} {}

  constexpr explicit operator bool() const noexcept {
    return lockable_.operator bool();
  }

  // named requirement: BasicLockable

  constexpr void lock() const
      noexcept(noexcept(lockable_->lock(),
                        null_lockable.lock())) requires requires {
    { lockable_->lock() } -> std::same_as<void>;
    { null_lockable.lock() } -> std::same_as<void>;
  }
  {
    if (lockable_) {
      lockable_->lock();
    } else {
      null_lockable.lock();
    }
  }
  constexpr void unlock() const
      noexcept(noexcept(lockable_->unlock(),
                        null_lockable.unlock())) requires requires {
    { lockable_->unlock() } -> std::same_as<void>;
    { null_lockable.unlock() } -> std::same_as<void>;
  }
  {
    if (lockable_) {
      lockable_->unlock();
    } else {
      null_lockable.unlock();
    }
  }

  // named requirement: BasicLockable <- Lockable

  constexpr auto try_lock [[nodiscard]] () const
      noexcept(noexcept(bool{lockable_->try_lock()},
                        bool{null_lockable.try_lock()}) &&
               std::is_nothrow_move_constructible_v<bool>)
          -> bool requires requires {
    { lockable_->try_lock() } -> std::same_as<bool>;
    { null_lockable.try_lock() } -> std::same_as<bool>;
  }
  {
    if (lockable_) {
      return lockable_->try_lock();
    }
    return null_lockable.try_lock();
  }

  // named requirement: BasicLockable <- Lockable <- TimedLockable

  template <typename Rep, typename Period>
  constexpr auto try_lock_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time) const
      noexcept(noexcept(bool{lockable_->try_lock_for(rel_time)},
                        bool{null_lockable.try_lock_for(rel_time)}) &&
               std::is_nothrow_move_constructible_v<bool>)
          -> bool requires requires {
    { lockable_->try_lock_for(rel_time) } -> std::same_as<bool>;
    { null_lockable.try_lock_for(rel_time) } -> std::same_as<bool>;
  }
  {
    if (lockable_) {
      return lockable_->try_lock_for(rel_time);
    }
    return null_lockable.try_lock_for(rel_time);
  }
  template <typename Clock, typename Duration>
  constexpr auto try_lock_until [[nodiscard]] (
      std::chrono::time_point<Clock, Duration> const &abs_time) const
      noexcept(noexcept(bool{lockable_->try_lock_until(abs_time)},
                        bool{null_lockable.try_lock_until(abs_time)}) &&
               std::is_nothrow_move_constructible_v<bool>)
          -> bool requires requires {
    { lockable_->try_lock_until(abs_time) } -> std::same_as<bool>;
    { null_lockable.try_lock_until(abs_time) } -> std::same_as<bool>;
  }
  {
    if (lockable_) {
      return lockable_->try_lock_until(abs_time);
    }
    return null_lockable.try_lock_until(abs_time);
  }

  // named requirement: SharedLockable

  constexpr void lock_shared() const
      noexcept(noexcept(lockable_->lock_shared(),
                        null_lockable.lock_shared())) requires requires {
    { lockable_->lock_shared() } -> std::same_as<void>;
    { null_lockable.lock_shared() } -> std::same_as<void>;
  }
  {
    if (lockable_) {
      lockable_->lock_shared();
    } else {
      null_lockable.lock_shared();
    }
  }
  constexpr auto try_lock_shared [[nodiscard]] () const
      noexcept(noexcept(bool{lockable_->try_lock_shared()},
                        bool{null_lockable.try_lock_shared()}) &&
               std::is_nothrow_move_constructible_v<bool>)
          -> bool requires requires {
    { lockable_->try_lock_shared() } -> std::same_as<bool>;
    { null_lockable.try_lock_shared() } -> std::same_as<bool>;
  }
  {
    if (lockable_) {
      return lockable_->try_lock_shared();
    }
    return null_lockable.try_lock_shared();
  }
  constexpr void unlock_shared() const
      noexcept(noexcept(lockable_->unlock_shared(),
                        null_lockable.unlock_shared())) requires requires {
    { lockable_->unlock_shared() } -> std::same_as<void>;
    { null_lockable.unlock_shared() } -> std::same_as<void>;
  }
  {
    if (lockable_) {
      lockable_->unlock_shared();
    } else {
      null_lockable.unlock_shared();
    }
  }

  // named requirement: SharedLockable <- SharedTimedLockable

  template <typename Rep, typename Period>
  constexpr auto try_lock_shared_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time) const
      noexcept(noexcept(bool{lockable_->try_lock_shared_for(rel_time)},
                        bool{null_lockable.try_lock_shared_for(rel_time)}) &&
               std::is_nothrow_move_constructible_v<bool>) -> bool {
    if (lockable_) {
      return lockable_->try_lock_shared_for(rel_time);
    }
    return null_lockable.try_lock_shared_for(rel_time);
  }
  template <typename Clock, typename Duration>
  constexpr auto try_lock_shared_until [[nodiscard]] (
      std::chrono::time_point<Clock, Duration> const &abs_time) const
      noexcept(noexcept(bool{lockable_->try_lock_shared_until(abs_time)},
                        bool{null_lockable.try_lock_shared_until(abs_time)}) &&
               std::is_nothrow_move_constructible_v<bool>) -> bool {
    if (lockable_) {
      return lockable_->try_lock_shared_until(abs_time);
    }
    return null_lockable.try_lock_shared_until(abs_time);
  }
};
} // namespace artccel::core::util

#endif
