# building
if(WIN32)
	add_definitions(-DNOMINMAX)
endif()

add_compile_options(
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wpedantic>
	$<$<CXX_COMPILER_ID:MSVC>:/Wall>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
	$<$<CXX_COMPILER_ID:MSVC>:/WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-error=unknown-pragmas>
	$<$<CXX_COMPILER_ID:MSVC>:/wd4068> # unknown pragma
	$<$<CXX_COMPILER_ID:MSVC>:/wd4464> # relative include path contains '..'
	$<$<CXX_COMPILER_ID:MSVC>:/wd4514> # unreferenced inline function has been removed
	$<$<CXX_COMPILER_ID:MSVC>:/wd4710> # function not inlined
	$<$<CXX_COMPILER_ID:MSVC>:/wd5030> # attribute is not recognized

	# pragma warning seems to be broken for below warnings
	$<$<CXX_COMPILER_ID:MSVC>:/wd4868> # compiler may not enforce left-to-right evaluation order in braced initializer list
	$<$<CXX_COMPILER_ID:MSVC>:/wd5045> # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified

	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-finput-charset=UTF-8>
	$<$<CXX_COMPILER_ID:MSVC>:/source-charset:UTF-8>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fexec-charset=UTF-8>
	$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:UTF-8>

	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fvisibility=hidden>
)
add_link_options(
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/DEBUG>
	$<$<CXX_COMPILER_ID:MSVC>:/WX>
)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	add_compile_options(
		-Weverything
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-c++98-c++11-c++14-compat
		-Wno-c++20-compat
		-Wno-return-std-move-in-c++11
		-Wno-missing-variable-declarations
		-Wno-ctad-maybe-unsupported
		-Wno-error=unknown-argument
		-Wno-error=unknown-warning-option
		-Wno-error=unknown-attributes
	)
endif()

include(CheckIPOSupported)

function(enable_ipo_if_supported)
	check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)

	if(IPO_SUPPORTED)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION true)
		message(STATUS "Interprocedural optimization is enabled")
	else()
		message(STATUS "Interprocedural optimization is not supported: ${IPO_OUTPUT}")
	endif()
endfunction()

enable_ipo_if_supported()