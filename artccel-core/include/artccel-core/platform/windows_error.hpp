#ifndef ARTCCEL_CORE_PLATFORM_WINDOWS_ERROR_HPP
#define ARTCCEL_CORE_PLATFORM_WINDOWS_ERROR_HPP
#pragma once
#ifdef _WIN32

namespace artccel::core::platform::windows::f {
void throw_last_error [[noreturn]] ();
void print_last_error();
} // namespace artccel::core::platform::windows::f

#endif
#endif
