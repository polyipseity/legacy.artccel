# preconditions
if(NOT DEFINED ROOT_SOURCE_DIR)
	message(FATAL_ERROR "'ROOT_SOURCE_DIR' is undefined")
endif()

# tools
# clang-tidy
function(target_integrate_clang_tidy TARGET LANGUAGE LINE_FILTER_EXCLUDES)
	# workaround: make up for a lack of a usable filter
	set(LINE_FILTER "")

	foreach(LINE_FILTER_EXCLUDE IN LISTS LINE_FILTER_EXCLUDES)
		string(APPEND LINE_FILTER "{\"name\": \"${LINE_FILTER_EXCLUDE}\", \"lines\": [[2, 1]]},")
	endforeach()

	string(APPEND LINE_FILTER "{\"name\": \".h\"}, {\"name\": \".c\"},")

	if(LANGUAGE STREQUAL "CXX")
		string(APPEND LINE_FILTER "\
{\"name\": \".hpp\"}, {\"name\": \".cpp\"},\
{\"name\": \".hxx\"}, {\"name\": \".cxx\"},\
{\"name\": \".hh\"}, {\"name\": \".cc\"},\
{\"name\": \".h++\"}, {\"name\": \".c++\"},\
")
	endif()

	# actual work
	string(REPLACE "X" "+" LANGUAGE_PROPER_NAME "${LANGUAGE}")
	string(TOLOWER "${LANGUAGE_PROPER_NAME}" LANGUAGE_PROPER_NAME)
	set(INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>")
	set(CLANG_TIDY_OUTPUTS "")
	get_target_property(TARGET_SOURCES "${TARGET}" SOURCES)

	foreach(TARGET_SOURCE IN LISTS TARGET_SOURCES)
		get_filename_component(TARGET_SOURCE_REAL "${TARGET_SOURCE}" REALPATH BASE_DIR "${PROJECT_SOURCE_DIR}")
		string(SHA3_512 TARGET_SOURCE_HASH "${TARGET_SOURCE_REAL}")
		add_custom_command(
			OUTPUT "clang-tidy/${TARGET_SOURCE_HASH}.timestamp"
			COMMAND clang-tidy ARGS
			"--config-file=${ROOT_SOURCE_DIR}/.clang-tidy"
			"--line-filter=[${LINE_FILTER}]"
			"${TARGET_SOURCE_REAL}"
			--
			"$<$<BOOL:${INCLUDE_DIRECTORIES}>:-I$<JOIN:${INCLUDE_DIRECTORIES},;-I>>"
			"-std=${LANGUAGE_PROPER_NAME}${CMAKE_${LANGUAGE}_STANDARD}"
			-Wall -Wextra -Wpedantic -Werror # -Weverything has checks that crash clang-tidy
			-finput-charset=UTF-8 -fexec-charset=UTF-8

			# workaround: make clang-tidy include non-default system headers
			"$<$<BOOL:${CMAKE_${LANGUAGE}_IMPLICIT_INCLUDE_DIRECTORIES}>:-isystem$<JOIN:${CMAKE_${LANGUAGE}_IMPLICIT_INCLUDE_DIRECTORIES},;-isystem>>"
			COMMAND "${CMAKE_COMMAND}" ARGS -E touch "${PROJECT_BINARY_DIR}/clang-tidy/${TARGET_SOURCE_HASH}.timestamp"
			DEPENDS "${TARGET_SOURCE_REAL}" # MAIN_DEPENDENCY silently overwrites compilation
			IMPLICIT_DEPENDS "${LANGUAGE}" "${TARGET_SOURCE_REAL}"
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
			COMMENT "Running clang-tidy on '${TARGET_SOURCE}' of '${TARGET}'"
			VERBATIM COMMAND_EXPAND_LISTS # dumb generator expression with spaces is a time waster
		)
		list(APPEND CLANG_TIDY_OUTPUTS "clang-tidy/${TARGET_SOURCE_HASH}.timestamp")
	endforeach()

	add_custom_target("${TARGET}-clang-tidy"
		DEPENDS ${CLANG_TIDY_OUTPUTS}
		COMMENT "Running clang-tidy on '${TARGET}'"
		VERBATIM
	)

	if(ARTCCEL_RUN_CLANG_TIDY)
		add_dependencies("${TARGET}" "${TARGET}-clang-tidy")
	endif()
endfunction()
