#include <artccel-core/util/exception_extras.hpp> // interface

#include <cstddef>   // import std::nullptr_t
#include <exception> // import std::exception_ptr, std::rethrow_exception
#include <utility>   // import std::exchange, std::move, std::swap

namespace artccel::core::util {
Rethrow_on_lexical_scope_exit::Rethrow_on_lexical_scope_exit() noexcept =
    default;
Rethrow_on_lexical_scope_exit::Rethrow_on_lexical_scope_exit(
    [[maybe_unused]] std::nullptr_t /*unused*/) noexcept {}
Rethrow_on_lexical_scope_exit::Rethrow_on_lexical_scope_exit(
    std::exception_ptr exc) noexcept
    : exc_{std::move(exc)} {}
auto Rethrow_on_lexical_scope_exit::write [[nodiscard]] () &noexcept
    -> std::exception_ptr & {
  return exc_;
}
auto Rethrow_on_lexical_scope_exit::read [[nodiscard]] () const &noexcept
    -> std::exception_ptr const & {
  return exc_;
}
auto Rethrow_on_lexical_scope_exit::release
    [[nodiscard]] () & -> std::exception_ptr {
  return std::exchange(exc_, nullptr);
}
auto Rethrow_on_lexical_scope_exit::operator=(std::nullptr_t right
                                              [[maybe_unused]]) &noexcept
    -> Rethrow_on_lexical_scope_exit & {
  exc_ = nullptr;
  return *this;
}
auto Rethrow_on_lexical_scope_exit::operator=(
    std::exception_ptr const &right) &noexcept
    -> Rethrow_on_lexical_scope_exit & {
  exc_ = right;
  return *this;
}
auto Rethrow_on_lexical_scope_exit::operator=(
    std::exception_ptr &&right) &noexcept -> Rethrow_on_lexical_scope_exit & {
  using std::swap;
  swap(exc_, right);
  return *this;
}
Rethrow_on_lexical_scope_exit::~Rethrow_on_lexical_scope_exit() noexcept(
    false) {
  if (exc_) {
    std::rethrow_exception(exc_);
  }
}
} // namespace artccel::core::util
