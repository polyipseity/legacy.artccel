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
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fvisibility=hidden>
	$<$<CXX_COMPILER_ID:MSVC>:/W4>
	$<$<CXX_COMPILER_ID:MSVC>:/WX>
	$<$<CXX_COMPILER_ID:MSVC>:/wd4068> # unknown pragma
)

include(GenerateExportHeader)

function(generate_preset_export_header LIBRARY_TARGET)
	string(REPLACE "-" "_" LIBRARY_TARGET_CODE "${LIBRARY_TARGET}")
	string(TOUPPER "${LIBRARY_TARGET_CODE}" LIBRARY_TARGET_CODE_UPPER)
	set(CUSTOM_EXPORT_HEADER "
#ifdef ${LIBRARY_TARGET_CODE_UPPER}_STATIC_DEFINE
#	define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION
#	define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DEFINITION
#else
#	ifdef ${LIBRARY_TARGET_CODE}_EXPORTS
#		ifdef _MSC_VER
#			define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION
#			define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DEFINITION ${LIBRARY_TARGET_CODE_UPPER}_EXPORT
#		else
#			define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION ${LIBRARY_TARGET_CODE_UPPER}_EXPORT
#			define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DEFINITION
#		endif
#	else
#		ifdef _MSC_VER
#			define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION ${LIBRARY_TARGET_CODE_UPPER}_EXPORT
#			define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DEFINITION
#		else
#			define ${LIBRARY_TARGET_CODE_UPPER}_EXPORT_DECLARATION ${LIBRARY_TARGET_CODE_UPPER}_EXPORT
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
set(CMAKE_CXX_CLANG_TIDY clang-tidy -p "${CMAKE_BINARY_DIR}/compile_commands.json" "--config-file=${ROOT_DIR}/.clang-tidy")
