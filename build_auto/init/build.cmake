# building
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CXX_CLANG_LIKE_COMPILER true)
else()
	set(CXX_CLANG_LIKE_COMPILER false)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR CXX_CLANG_LIKE_COMPILER)
	set(CXX_GCC_LIKE_COMPILER true)
else()
	set(CXX_GCC_LIKE_COMPILER false)
endif()

add_compile_definitions("$<$<BOOL:${WIN32}>:NOMINMAX>")
add_compile_options(
	"$<$<BOOL:${CXX_GCC_LIKE_COMPILER}>:-Wall>"
	"$<$<BOOL:${CXX_GCC_LIKE_COMPILER}>:-Wextra>"
	"$<$<BOOL:${CXX_GCC_LIKE_COMPILER}>:-Wpedantic>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Weverything>"
	"$<$<CXX_COMPILER_ID:MSVC>:/Wall>"
	"$<$<BOOL:${CXX_GCC_LIKE_COMPILER}>:-Werror>"
	"$<$<CXX_COMPILER_ID:MSVC>:/WX>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-c++20-compat>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-c++98-c++11-c++14-compat>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-c++98-compat>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-c++98-compat-pedantic>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-ctad-maybe-unsupported>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-missing-variable-declarations>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-return-std-move-in-c++11>"
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4068>" # unknown pragma
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4464>" # relative include path contains '..'
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4514>" # unreferenced inline function has been removed
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4710>" # function not inlined
	"$<$<CXX_COMPILER_ID:MSVC>:/wd5030>" # attribute is not recognized

	"$<$<BOOL:${CXX_GCC_LIKE_COMPILER}>:-Wno-error=attributes>"
	"$<$<BOOL:${CXX_GCC_LIKE_COMPILER}>:-Wno-error=unknown-pragmas>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-error=unknown-argument>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-error=unknown-attributes>"
	"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-Wno-error=unknown-warning-option>"
	"$<$<AND:$<BOOL:${CXX_CLANG_LIKE_COMPILER}>,$<BOOL:${WIN32}>>:-Wno-error=nonportable-system-include-path>"

	# #pragma warning(disable : XXXX) seems to be broken for below warnings
	"$<$<CXX_COMPILER_ID:MSVC>:/wd4868>" # compiler may not enforce left-to-right evaluation order in braced initializer list
	"$<$<CXX_COMPILER_ID:MSVC>:/wd5045>" # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified

	"$<$<BOOL:${CXX_GCC_LIKE_COMPILER}>:-finput-charset=UTF-8>"
	"$<$<CXX_COMPILER_ID:MSVC>:/source-charset:UTF-8>"
	"$<$<BOOL:${CXX_GCC_LIKE_COMPILER}>:-fexec-charset=UTF-8>"
	"$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:UTF-8>"
)

if(ARTCCEL_PROFILE_COMPILATION)
	add_compile_options(
		"$<$<BOOL:${CXX_GCC_LIKE_COMPILER}>:-ftime-report>"
		"$<$<BOOL:${CXX_CLANG_LIKE_COMPILER}>:-ftime-trace>"
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
