#include <artccel-core/util/exception_extras.hpp> // interface

#include <cstddef>   // import std::nullptr_t
#include <exception> // import std::exception_ptr, std::rethrow_exception
#include <utility>   // import std::move

namespace artccel::core::util {
Rethrow_on_destruct::Rethrow_on_destruct() noexcept = default;
Rethrow_on_destruct::Rethrow_on_destruct(
    [[maybe_unused]] std::nullptr_t /*unused*/) noexcept {
  /* explicit so that we do not get
  Rethrow_on_destruct rod{exception};
  rod = nullptr; // rethrow exception
  the right way is
  rod.ptr() = nullptr; // disable rethrow
  */
}
Rethrow_on_destruct::Rethrow_on_destruct(std::exception_ptr exc) noexcept
    : exc_{std::move(exc)} {}
auto Rethrow_on_destruct::ptr [[nodiscard]] () &noexcept
    -> std::exception_ptr & {
  return exc_;
}
auto Rethrow_on_destruct::ptr [[nodiscard]] () const &noexcept
    -> std::exception_ptr const & {
  return exc_;
}
auto Rethrow_on_destruct::ptr [[nodiscard]] () &&noexcept
    -> std::exception_ptr {
  return std::move(exc_);
}
auto Rethrow_on_destruct::ptr [[nodiscard]] () const && -> std::exception_ptr {
  return exc_;
}
Rethrow_on_destruct::Rethrow_on_destruct(Rethrow_on_destruct &&other) noexcept =
    default;
auto Rethrow_on_destruct::operator=(Rethrow_on_destruct &&right) noexcept
    -> Rethrow_on_destruct & = default;
Rethrow_on_destruct::~Rethrow_on_destruct() noexcept(false) {
  if (exc_) {
    std::rethrow_exception(exc_);
  }
}
} // namespace artccel::core::util
