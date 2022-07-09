# preconditions:
# - required variables: ROOT_DIR

set(THREADS_PREFER_PTHREAD_FLAG true)
find_package(Threads REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

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
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-error=unknown-warning-option>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-error=unknown-pragmas>
	$<$<CXX_COMPILER_ID:MSVC>:/wd4068> # unknown pragma
	$<$<CXX_COMPILER_ID:MSVC>:/wd4464> # relative include path contains '..'
	$<$<CXX_COMPILER_ID:MSVC>:/wd4514> # unreferenced inline function has been removed
	$<$<CXX_COMPILER_ID:MSVC>:/wd4710> # function not inlined
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-error=unknown-attributes>
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
	)
endif()

include(GenerateExportHeader)

function(generate_preset_export_header LIBRARY_TARGET)
	string(REPLACE "-" "_" LIBRARY_TARGET_CODE "${LIBRARY_TARGET}")
	string(TOUPPER "${LIBRARY_TARGET_CODE}" LIBRARY_TARGET_CODE_UPPER)
	set(CUSTOM_EXPORT_HEADER "
#ifdef ${LIBRARY_TARGET_CODE_UPPER}_STATIC_DEFINE
#	define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION
#	define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DEFINITION
#else
#	ifndef ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION
#		ifdef ${LIBRARY_TARGET_CODE}_EXPORTS
#			ifdef _MSC_VER
#				define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION
#			else
#				define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION ${LIBRARY_TARGET_CODE_UPPER}_EXPORT
#			endif
#		else
#			define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION ${LIBRARY_TARGET_CODE_UPPER}_EXPORT
#		endif
#	endif
#	ifndef ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DEFINITION
#		ifdef ${LIBRARY_TARGET_CODE}_EXPORTS
#			ifdef _MSC_VER
#				define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DEFINITION ${LIBRARY_TARGET_CODE_UPPER}_EXPORT
#			else
#				define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DEFINITION
#			endif
#		else
#			define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DEFINITION
#		endif
#	endif
#endif
")
	generate_export_header("${LIBRARY_TARGET}"
		EXPORT_FILE_NAME "include/${LIBRARY_TARGET}/export.h"
		CUSTOM_CONTENT_FROM_VARIABLE CUSTOM_EXPORT_HEADER)
endfunction()

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

# Workaround: Make clang-tidy include non-default system headers
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES} ${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES})
set(CMAKE_CXX_CLANG_TIDY clang-tidy
	-p "${CMAKE_BINARY_DIR}/compile_commands.json"
	"-line-filter=[\
{\"name\": \"config.h\", \"lines\": [[65535, 1]]},\
{\"name\": \"export.h\", \"lines\": [[65535, 1]]},\
{\"name\": \".h\"},\
{\"name\": \".hpp\"},\
{\"name\": \".c\"},\
{\"name\": \".cpp\"}\
]"
	"--config-file=${ROOT_DIR}/.clang-tidy")
