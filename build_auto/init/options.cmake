cmake_minimum_required(VERSION 3.16)

# options
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(ARTCCEL_STANDALONE true)
else()
	set(ARTCCEL_STANDALONE false)
endif()

# external
option(ARTCCEL_INSTALL "Generate and install targets" "${ARTCCEL_STANDALONE}")
option(ARTCCEL_TEST "Build and perform tests" "${ARTCCEL_STANDALONE}")
set(ARTCCEL_NAMESPACE "artccel::" CACHE STRING "Namespace of exported targets")

# build
option(ARTCCEL_INTERPROCEDURAL_OPTIMIZATION "Enable interprocedural optimization if available" true)
option(ARTCCEL_PROFILE_COMPILATION "Profile compilation time" false)
option(ARTCCEL_SANITIZE_ADDRESS "Enable address sanitizer" false)
option(ARTCCEL_SANITIZE_MEMORY "Enable memory sanitizer" false)
option(ARTCCEL_SANITIZE_THREAD "Enable thread sanitizer" false)
option(ARTCCEL_SANITIZE_UNDEFINED "Enable undefined behavior sanitizer" false)

# tools
option(ARTCCEL_CCACHE "Use ccache" true)
set(ARTCCEL_CCACHE_OPTIONS
	CCACHE_SLOPPINESS=include_file_ctime,include_file_mtime,pch_defines,time_macros
	CACHE STRING "Environment variables passed to ccache")
option(ARTCCEL_RUN_CLANG_TIDY "Run clang-tidy" false)
