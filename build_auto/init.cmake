# preconditions:
# - required variables: ROOT_DIR

set(THREADS_PREFER_PTHREAD_FLAG true)
find_package(Threads REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_compile_options(
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wpedantic>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-error=unknown-pragmas>
	$<$<CXX_COMPILER_ID:MSVC>:/W4>
	$<$<CXX_COMPILER_ID:MSVC>:/WX>
)
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)

if(IPO_SUPPORTED)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION true)
	message(STATUS "Interprocedural optimization is enabled")
else()
	message(STATUS "Interprocedural optimization is not supported: ${IPO_OUTPUT}")
endif()

# Workaround: Make clang-tidy include non-default system headers
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES} ${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES})
set(CMAKE_CXX_CLANG_TIDY clang-tidy -p "${CMAKE_BINARY_DIR}/compile_commands.json" "--config-file=${ROOT_DIR}/.clang-tidy")
