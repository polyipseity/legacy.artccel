#include "artccel-core/concurrent.hpp" // interface
#include <chrono> // import std::chrono::duration, std::chrono::time_point

namespace artccel::core::util {
constexpr void Null_lockable::lock() noexcept {}
constexpr void Null_lockable::unlock() noexcept {}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
constexpr auto Null_lockable::try_lock [[nodiscard]] () noexcept -> bool {
  return true;
}
constexpr void Null_lockable::lock_shared() noexcept {}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
constexpr auto Null_lockable::try_lock_shared [[nodiscard]] () noexcept
    -> bool {
  return true;
}
constexpr void Null_lockable::unlock_shared() noexcept {}
} // namespace artccel::core::util
