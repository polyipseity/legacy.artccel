#pragma once
#ifndef GUARD_E4462344_3D02_4011_8109_D2998F468F32
#define GUARD_E4462344_3D02_4011_8109_D2998F468F32

#include <chrono>   // import std::chrono::duration, std::chrono::time_point
#include <concepts> // import std::invocable, std::semiregular, std::same_as
#include <memory>   // import std::make_unique, std::unique_ptr
#include <mutex> // import std::call_once, std::mutex, std::once_flag, std::recursive_mutex, std::recursive_timed_mutex, std::timed_mutex
#include <shared_mutex> // import std::shared_mutex, std::shared_timed_mutex
#include <utility> // import std::declval, std::forward, std::move, std::swap

#include "utility_extras.hpp" // import Delegate, Initialize_t
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT, ARTCCEL_CORE_EXPORT_DECLARATION

namespace artccel::core::util {
class ARTCCEL_CORE_EXPORT Semiregular_once_flag;
struct ARTCCEL_CORE_EXPORT Null_lockable;
template <typename Lock, typename NullLock = Null_lockable>
class Nullable_lockable;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
class Semiregular_once_flag {
#pragma clang diagnostic pop
private:
#pragma warning(suppress : 4251)
  std::unique_ptr</* mutable */ std::once_flag> value_{
      std::make_unique<std::once_flag>()};
  bool flag_{false};

public:
  constexpr Semiregular_once_flag() noexcept = default;
  template <typename... Args, std::invocable<Args...> Func>
  void call_once(Func &&func, Args &&...args) {
    std::call_once(
        *value_, [this, &func, &args...]() noexcept(noexcept(std::invoke(
                     std::forward<Func>(func), std::forward<Args>(args)...))) {
          std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
          flag_ = true; // after invocation because std::invoke may throw
        });
  }

  ~Semiregular_once_flag() noexcept = default;
  void swap(Semiregular_once_flag &other) noexcept {
    using std::swap;
    swap(value_, other.value_);
    swap(flag_, other.flag_);
  }
  friend void swap(Semiregular_once_flag &left,
                   Semiregular_once_flag &right) noexcept {
    left.swap(right);
  }
  Semiregular_once_flag(Semiregular_once_flag const &other)
      : flag_{other.flag_} {
    if (flag_) {
      std::call_once(*value_, []() noexcept {});
    }
  }
  auto operator=(Semiregular_once_flag const &right) noexcept(noexcept(
      static_cast<void>(Semiregular_once_flag{right}.swap(*this)), *this))
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
#pragma warning(suppress : 4820)
};
static_assert(std::semiregular<Semiregular_once_flag>,
              u8"Implementation error");

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

template <typename Lock, typename NullLock>
#pragma warning(suppress : 4251)
class Nullable_lockable : public Delegate<std::unique_ptr</* mutable */ Lock>> {
public:
  using type = typename Nullable_lockable::type;
  using lockable_type = Lock;
  using null_lockable_type = NullLock;

private:
  constexpr static NullLock null_lockable_{};

protected:
  explicit constexpr Nullable_lockable(Initialize_t tag [[maybe_unused]],
                                       type &&value) noexcept
      : Nullable_lockable::Delegate{Initialize_t{}, std::move(value)} {}

public:
  constexpr Nullable_lockable() noexcept
      : Nullable_lockable{Initialize_t{}, std::make_unique<Lock>()} {}
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  constexpr Nullable_lockable(type value) noexcept
      : Nullable_lockable{Initialize_t{}, std::move(value)} {}

  Nullable_lockable(Nullable_lockable const &) = delete;
  auto operator=(Nullable_lockable const &) = delete;
  constexpr Nullable_lockable(Nullable_lockable &&) noexcept = default;
  constexpr auto operator=(Nullable_lockable &&) noexcept
      -> Nullable_lockable & = default;
  constexpr ~Nullable_lockable() noexcept = default;

  // named requirement: BasicLockable

  template <typename = void>
  requires requires {
    { std::declval<type const &>()->lock() } -> std::same_as<void>;
    { null_lockable_.lock() } -> std::same_as<void>;
  }
  constexpr void lock() const
      noexcept(noexcept(this->value_->lock(), void(), null_lockable_.lock())) {
    if (this->value_) {
      this->value_->lock();
    } else {
      null_lockable_.lock();
    }
  }
  template <typename = void>
  requires requires {
    { std::declval<type const &>()->unlock() } -> std::same_as<void>;
    { null_lockable_.unlock() } -> std::same_as<void>;
  }
  constexpr void unlock() const
      noexcept(noexcept(this->value_->unlock(), void(),
                        null_lockable_.unlock())) {
    if (this->value_) {
      this->value_->unlock();
    } else {
      null_lockable_.unlock();
    }
  }

  // named requirement: BasicLockable <- Lockable

  template <typename = void>
  requires requires {
    { std::declval<type const &>()->try_lock() } -> std::same_as<bool>;
    { null_lockable_.try_lock() } -> std::same_as<bool>;
  }
  constexpr auto try_lock [[nodiscard]] () const
      noexcept(noexcept(static_cast<void>(bool{this->value_->try_lock()}),
                        bool{null_lockable_.try_lock()})) -> bool {
    if (this->value_) {
      return this->value_->try_lock();
    }
    return null_lockable_.try_lock();
  }

  // named requirement: BasicLockable <- Lockable <- TimedLockable

  template <typename Rep, typename Period>
  requires requires {
    {
      std::declval<type const &>()->try_lock_for(
          std::declval<std::chrono::duration<Rep, Period> const &>())
      } -> std::same_as<bool>;
    {
      null_lockable_.try_lock_for(
          std::declval<std::chrono::duration<Rep, Period> const &>())
      } -> std::same_as<bool>;
  }
  constexpr auto try_lock_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time) const
      noexcept(noexcept(static_cast<void>(bool{
                            this->value_->try_lock_for(rel_time)}),
                        bool{null_lockable_.try_lock_for(rel_time)})) -> bool {
    if (this->value_) {
      return this->value_->try_lock_for(rel_time);
    }
    return null_lockable_.try_lock_for(rel_time);
  }
  template <typename Clock, typename Duration>
  requires requires {
    {
      std::declval<type const &>()->try_lock_until(
          std::declval<std::chrono::time_point<Clock, Duration> const &>())
      } -> std::same_as<bool>;
    {
      null_lockable_.try_lock_until(
          std::declval<std::chrono::time_point<Clock, Duration> const &>())
      } -> std::same_as<bool>;
  }
  constexpr auto try_lock_until [[nodiscard]] (
      std::chrono::time_point<Clock, Duration> const &abs_time) const
      noexcept(noexcept(static_cast<void>(bool{
                            this->value_->try_lock_until(abs_time)}),
                        bool{null_lockable_.try_lock_until(abs_time)}))
          -> bool {
    if (this->value_) {
      return this->value_->try_lock_until(abs_time);
    }
    return null_lockable_.try_lock_until(abs_time);
  }

  // named requirement: SharedLockable

  template <typename = void>
  requires requires {
    { std::declval<type const &>()->lock_shared() } -> std::same_as<void>;
    { null_lockable_.lock_shared() } -> std::same_as<void>;
  }
  constexpr void lock_shared() const
      noexcept(noexcept(this->value_->lock_shared(), void(),
                        null_lockable_.lock_shared())) {
    if (this->value_) {
      this->value_->lock_shared();
    } else {
      null_lockable_.lock_shared();
    }
  }
  template <typename = void>
  requires requires {
    { std::declval<type const &>()->try_lock_shared() } -> std::same_as<bool>;
    { null_lockable_.try_lock_shared() } -> std::same_as<bool>;
  }
  constexpr auto try_lock_shared [[nodiscard]] () const noexcept(
      noexcept(static_cast<void>(bool{this->value_->try_lock_shared()}),
               bool{null_lockable_.try_lock_shared()})) -> bool {
    if (this->value_) {
      return this->value_->try_lock_shared();
    }
    return null_lockable_.try_lock_shared();
  }
  template <typename = void>
  requires requires {
    { std::declval<type const &>()->unlock_shared() } -> std::same_as<void>;
    { null_lockable_.unlock_shared() } -> std::same_as<void>;
  }
  constexpr void unlock_shared() const
      noexcept(noexcept(this->value_->unlock_shared(), void(),
                        null_lockable_.unlock_shared())) {
    if (this->value_) {
      this->value_->unlock_shared();
    } else {
      null_lockable_.unlock_shared();
    }
  }

  // named requirement: SharedLockable <- SharedTimedLockable

  template <typename Rep, typename Period>
  requires requires {
    {
      std::declval<type const &>()->try_lock_shared_for(
          std::declval<std::chrono::duration<Rep, Period> const &>())
      } -> std::same_as<bool>;
    {
      null_lockable_.try_lock_shared_for(
          std::declval<std::chrono::duration<Rep, Period> const &>())
      } -> std::same_as<bool>;
  }
  constexpr auto try_lock_shared_for
      [[nodiscard]] (std::chrono::duration<Rep, Period> const &rel_time) const
      noexcept(noexcept(static_cast<void>(bool{
                            this->value_->try_lock_shared_for(rel_time)}),
                        bool{null_lockable_.try_lock_shared_for(rel_time)}))
          -> bool {
    if (this->value_) {
      return this->value_->try_lock_shared_for(rel_time);
    }
    return null_lockable_.try_lock_shared_for(rel_time);
  }
  template <typename Clock, typename Duration>
  requires requires {
    {
      std::declval<type const &>()->try_lock_shared_until(
          std::declval<std::chrono::time_point<Clock, Duration> const &>())
      } -> std::same_as<bool>;
    {
      null_lockable_.try_lock_shared_until(
          std::declval<std::chrono::time_point<Clock, Duration> const &>())
      } -> std::same_as<bool>;
  }
  constexpr auto try_lock_shared_until [[nodiscard]] (
      std::chrono::time_point<Clock, Duration> const &abs_time) const
      noexcept(noexcept(static_cast<void>(bool{
                            this->value_->try_lock_shared_until(abs_time)}),
                        bool{null_lockable_.try_lock_shared_until(abs_time)}))
          -> bool {
    if (this->value_) {
      return this->value_->try_lock_shared_until(abs_time);
    }
    return null_lockable_.try_lock_shared_until(abs_time);
  }
};
extern template class ARTCCEL_CORE_EXPORT_DECLARATION
    Nullable_lockable<std::mutex>;
extern template class ARTCCEL_CORE_EXPORT_DECLARATION
    Nullable_lockable<std::timed_mutex>;
extern template class ARTCCEL_CORE_EXPORT_DECLARATION
    Nullable_lockable<std::recursive_mutex>;
extern template class ARTCCEL_CORE_EXPORT_DECLARATION
    Nullable_lockable<std::recursive_timed_mutex>;
extern template class ARTCCEL_CORE_EXPORT_DECLARATION
    Nullable_lockable<std::shared_mutex>;
extern template class ARTCCEL_CORE_EXPORT_DECLARATION
    Nullable_lockable<std::shared_timed_mutex>;
} // namespace artccel::core::util

#endif
