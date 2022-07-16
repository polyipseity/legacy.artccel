cmake_minimum_required(VERSION 3.16)

# building
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)

set(CXX_CLANG_LIKE_COMPILER_GENEXP "$<BOOL:$<FILTER:$<CXX_COMPILER_ID>,INCLUDE,Clang>>")
set(CXX_GCC_LIKE_COMPILER_GENEXP "$<OR:$<BOOL:$<CXX_COMPILER_ID:GNU>>,${CXX_CLANG_LIKE_COMPILER_GENEXP}>")

add_compile_definitions("$<$<BOOL:${WIN32}>:NOMINMAX>")
add_compile_options(
	"$<${CXX_GCC_LIKE_COMPILER_GENEXP}:-Wall>"
	"$<${CXX_GCC_LIKE_COMPILER_GENEXP}:-Wextra>"
	"$<${CXX_GCC_LIKE_COMPILER_GENEXP}:-Wpedantic>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Weverything>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Wall>"
	"$<${CXX_GCC_LIKE_COMPILER_GENEXP}:-Werror>"
	"$<$<CXX_COMPILER_ID:MSVC>:/WX>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-c++20-compat>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-c++98-c++11-c++14-compat>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-c++98-compat>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-c++98-compat-pedantic>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-ctad-maybe-unsupported>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-missing-variable-declarations>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-return-std-move-in-c++11>"
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4068>" # unknown pragma
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4464>" # relative include path contains '..'
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4514>" # unreferenced inline function has been removed
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4710>" # function not inlined
	"$<$<CXX_COMPILER_ID:MSVC>:/wd5030>" # attribute is not recognized

	"$<${CXX_GCC_LIKE_COMPILER_GENEXP}:-Wno-error=attributes>"
	"$<${CXX_GCC_LIKE_COMPILER_GENEXP}:-Wno-error=unknown-pragmas>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-error=unknown-argument>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-error=unknown-attributes>"
	"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-Wno-error=unknown-warning-option>"
	"$<$<AND:${CXX_CLANG_LIKE_COMPILER_GENEXP},$<BOOL:${WIN32}>>:-Wno-error=nonportable-system-include-path>"

	# #pragma warning(disable : XXXX) seems to be broken for below warnings
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4868>" # compiler may not enforce left-to-right evaluation order in braced initializer list
	"$<$<CXX_COMPILER_ID:MSVC>:/wd5045>" # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified

	"$<${CXX_GCC_LIKE_COMPILER_GENEXP}:-finput-charset=UTF-8>"
	"$<$<CXX_COMPILER_ID:MSVC>:/source-charset:UTF-8>"
	"$<${CXX_GCC_LIKE_COMPILER_GENEXP}:-fexec-charset=UTF-8>"
	"$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:UTF-8>"
)

if(ARTCCEL_PROFILE_COMPILATION)
	add_compile_options(
		"$<${CXX_GCC_LIKE_COMPILER_GENEXP}:-ftime-report>"
		"$<${CXX_CLANG_LIKE_COMPILER_GENEXP}:-ftime-trace>"
	)
	message(STATUS "Compilation profiling is enabled")

	# MSVC: vcperf
endif()

add_link_options(
	"$<$<CXX_COMPILER_ID:MSVC>:/WX>"
	"$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/DEBUG>"
)

if(ARTCCEL_INTERPROCEDURAL_OPTIMIZATION)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)

	if(IPO_SUPPORTED)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION true)
		message(STATUS "Interprocedural optimization is enabled")
	else()
		message(WARNING "Interprocedural optimization is not supported: ${IPO_OUTPUT}")
	endif()
endif()
