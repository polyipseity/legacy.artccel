#ifdef _WIN32
#include <cassert>      // import assert
#include <iostream>     // import std::cerr, std::flush
#include <system_error> // import std::system_category, std::system_error

#pragma warning(push)
#pragma warning(disable : 4668 5039)
#include <Windows.h> // import ::GetLastError
#pragma warning(pop)

#include <artccel/core/platform/windows_error.hpp> // interface

#include <artccel/core/util/conversions.hpp> // import util::f::int_modulo_cast
#include <artccel/core/util/encoding.hpp> // import util::literals::encoding::operator""_as_utf8_compat

namespace artccel::core::platform::windows::f {
using util::literals::encoding::operator""_as_utf8_compat;

void throw_last_error [[noreturn]] () {
  auto const last_error{::GetLastError()};
  assert(last_error && u8"No last error");
  throw std::system_error{util::f::int_modulo_cast<int>(last_error),
                          std::system_category()};
}
void print_last_error() {
  auto const last_error{::GetLastError()};
  assert(last_error && u8"No last error");
  std::cerr << u8"Windows system error: "_as_utf8_compat << last_error
            << u8'\n'_as_utf8_compat << std::flush;
}
} // namespace artccel::core::platform::windows::f

#endif
