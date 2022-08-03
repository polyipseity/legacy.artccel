#pragma once
#ifndef GUARD_F1F2A7DF_A99E_484B_9A2A_ECA764B219D9
#define GUARD_F1F2A7DF_A99E_484B_9A2A_ECA764B219D9

#ifdef _WIN32

namespace artccel::core::platform::windows::f {
void throw_last_error [[noreturn]] ();
void print_last_error();
} // namespace artccel::core::platform::windows::f

#endif

#endif
