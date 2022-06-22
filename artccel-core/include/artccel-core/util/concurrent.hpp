#ifndef ARTCCEL_CORE_UTIL_CONCURRENT_HPP
#define ARTCCEL_CORE_UTIL_CONCURRENT_HPP
#pragma once

#include "utility_extras.hpp" // import Delegate
#include <chrono>      // import std::chrono::duration, std::chrono::time_point
#include <concepts>    // import std::semiregular, std::same_as
#include <memory>      // import std::make_unique, std::unique_ptr
#include <mutex>       // import std::call_once, std::once_flag
#include <type_traits> // import std::is_nothrow_move_constructible_v
#include <utility>     // import std::forward, std::move, std::swap

namespace artccel::core::util {
class Semiregular_once_flag {
private:
  std::unique_ptr</* mutable */ std::once_flag> value_{
      std::make_unique<std::once_flag>()};
  bool flag_{false};

public:
  constexpr Semiregular_once_flag() noexcept = default;

  template <typename Callable, typename... Args>
  void call_once(Callable &&func, Args &&...args) {
    std::call_once(*value_, [this, &func, &args...]() noexcept(noexcept(
                                std::invoke(std::forward<Callable>(func),
                                            std::forward<Args>(args)...))) {
      std::invoke(std::forward<Callable>(func), std::forward<Args>(args)...);
      flag_ = true; // after invocation because std::invoke may throw
    });
  }

  ~Semiregular_once_flag() noexcept = default;
  constexpr void swap(Semiregular_once_flag &other) noexcept {
    using std::swap;
    swap(value_, other.value_);
    swap(flag_, other.flag_);
  }
  Semiregular_once_flag(Semiregular_once_flag const &other)
      : flag_{other.flag_} {
    if (flag_) {
      std::call_once(*value_, []() noexcept {});
    }
  }
  auto operator=(Semiregular_once_flag const &right) noexcept(
      noexcept(Semiregular_once_flag{right}.swap(*this), *this))
      -> Semiregular_once_flag & {
    Semiregular_once_flag{right}.swap(*this);
    return *this;
  }
  Semiregular_once_flag(Semiregular_once_flag &&other) noexcept
      : value_{std::move(other.value_)}, flag_{other.flag_} {}
  auto operator=(Semiregular_once_flag &&right) noexcept
      -> Semiregular_once_flag & {
    Semiregular_once_flag{std::move(right)}.swap(*this);
    return *this;
  }
};
static_assert(std::semiregular<Semiregular_once_flag>);
constexpr void swap(Semiregular_once_flag &left,
                    Semiregular_once_flag &right) noexcept {
  left.swap(right);
}

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

template <typename L, typename NL = Null_lockable>
class Nullable_lockable : public Delegate<std::unique_ptr</* mutable */ L>> {
public:
  using type = typename Nullable_lockable::type;
  using lockable_type = L;
  using null_lockable_type = NL;

private:
  constexpr static NL null_lockable{};

public:
  constexpr Nullable_lockable() noexcept
      : Nullable_lockable{std::make_unique<L>()} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr Nullable_lockable(type value) noexcept
      : Nullable_lockable::Delegate{std::move(value)} {}

  // named requirement: BasicLockable

  constexpr void lock() const
      noexcept(noexcept(this->value_->lock(),
                        null_lockable.lock())) requires requires {
    { this->value_->lock() } -> std::same_as<void>;
    { null_lockable.lock() } -> std::same_as<void>;
  }
  {
    if (this->value_) {
      this->value_->lock();
    } else {
      null_lockable.lock();
    }
  }
  constexpr void unlock() const
      noexcept(noexcept(this->value_->unlock(),
                        null_lockable.unlock())) requires requires {
    { this->value_->unlock() } -> std::same_as<void>;
    { null_lockable.unlock() } -> std::same_as<void>;
  }
  {
    if (this->value_) {
      this->value_->unlock();
    } else {
      null_lockable.unlock();
    }
  }

  // named requirement: BasicLockable <- Lockable

  constexpr auto try_lock [[nodiscard]] () const
      noexcept(noexcept(bool{this->value_->try_lock()},
                        bool{null_lockable.try_lock()}) &&
               std::is_nothrow_move_constructible_v<bool>)
          -> bool requires requires {
    { this->value_->try_lock() } -> std::same_as<bool>;
    { null_lockable.try_lock() } -> std::same_as<bool>;
  }
  {
    if (this->value_) {
      return this->value_->try_lock();
    }
    return null_lockable.try_lock();
  }

  // named requirement: BasicLockable <- Lockable <- TimedLockable

  template <typename Rep, typename Period>
  constexpr auto try_lock_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time) const
      noexcept(noexcept(bool{this->value_->try_lock_for(rel_time)},
                        bool{null_lockable.try_lock_for(rel_time)}) &&
               std::is_nothrow_move_constructible_v<bool>)
          -> bool requires requires {
    { this->value_->try_lock_for(rel_time) } -> std::same_as<bool>;
    { null_lockable.try_lock_for(rel_time) } -> std::same_as<bool>;
  }
  {
    if (this->value_) {
      return this->value_->try_lock_for(rel_time);
    }
    return null_lockable.try_lock_for(rel_time);
  }
  template <typename Clock, typename Duration>
  constexpr auto try_lock_until [[nodiscard]] (
      std::chrono::time_point<Clock, Duration> const &abs_time) const
      noexcept(noexcept(bool{this->value_->try_lock_until(abs_time)},
                        bool{null_lockable.try_lock_until(abs_time)}) &&
               std::is_nothrow_move_constructible_v<bool>)
          -> bool requires requires {
    { this->value_->try_lock_until(abs_time) } -> std::same_as<bool>;
    { null_lockable.try_lock_until(abs_time) } -> std::same_as<bool>;
  }
  {
    if (this->value_) {
      return this->value_->try_lock_until(abs_time);
    }
    return null_lockable.try_lock_until(abs_time);
  }

  // named requirement: SharedLockable

  constexpr void lock_shared() const
      noexcept(noexcept(this->value_->lock_shared(),
                        null_lockable.lock_shared())) requires requires {
    { this->value_->lock_shared() } -> std::same_as<void>;
    { null_lockable.lock_shared() } -> std::same_as<void>;
  }
  {
    if (this->value_) {
      this->value_->lock_shared();
    } else {
      null_lockable.lock_shared();
    }
  }
  constexpr auto try_lock_shared [[nodiscard]] () const
      noexcept(noexcept(bool{this->value_->try_lock_shared()},
                        bool{null_lockable.try_lock_shared()}) &&
               std::is_nothrow_move_constructible_v<bool>)
          -> bool requires requires {
    { this->value_->try_lock_shared() } -> std::same_as<bool>;
    { null_lockable.try_lock_shared() } -> std::same_as<bool>;
  }
  {
    if (this->value_) {
      return this->value_->try_lock_shared();
    }
    return null_lockable.try_lock_shared();
  }
  constexpr void unlock_shared() const
      noexcept(noexcept(this->value_->unlock_shared(),
                        null_lockable.unlock_shared())) requires requires {
    { this->value_->unlock_shared() } -> std::same_as<void>;
    { null_lockable.unlock_shared() } -> std::same_as<void>;
  }
  {
    if (this->value_) {
      this->value_->unlock_shared();
    } else {
      null_lockable.unlock_shared();
    }
  }

  // named requirement: SharedLockable <- SharedTimedLockable

  template <typename Rep, typename Period>
  constexpr auto try_lock_shared_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time) const
      noexcept(noexcept(bool{this->value_->try_lock_shared_for(rel_time)},
                        bool{null_lockable.try_lock_shared_for(rel_time)}) &&
               std::is_nothrow_move_constructible_v<bool>) -> bool {
    if (this->value_) {
      return this->value_->try_lock_shared_for(rel_time);
    }
    return null_lockable.try_lock_shared_for(rel_time);
  }
  template <typename Clock, typename Duration>
  constexpr auto try_lock_shared_until [[nodiscard]] (
      std::chrono::time_point<Clock, Duration> const &abs_time) const
      noexcept(noexcept(bool{this->value_->try_lock_shared_until(abs_time)},
                        bool{null_lockable.try_lock_shared_until(abs_time)}) &&
               std::is_nothrow_move_constructible_v<bool>) -> bool {
    if (this->value_) {
      return this->value_->try_lock_shared_until(abs_time);
    }
    return null_lockable.try_lock_shared_until(abs_time);
  }
};
} // namespace artccel::core::util

#endif
