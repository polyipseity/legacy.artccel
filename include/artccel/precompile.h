#ifndef GUARD_0849ED38_CAF3_47D7_B5FC_61B09E09BDEB
#define GUARD_0849ED38_CAF3_47D7_B5FC_61B09E09BDEB
#pragma once

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <climits>
#include <cstring>
#include <ios>
#include <iostream>
#include <locale>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <span>
#include <string_view>

#pragma warning(push)
#pragma warning(disable : 4626 4820)
#include <gsl/gsl>
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable : 4582 4583 4625 4626 4820 5026 5027)
#include <tl/expected.hpp>
#pragma warning(pop)
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4668 5039)
#include <windows.h>
#pragma warning(pop)
#endif

#endif
