# preconditions
if(NOT DEFINED ROOT_SOURCE_DIR)
	message(FATAL_ERROR "'ROOT_SOURCE_DIR' is undefined")
endif()

# tools
# clang-tidy
add_custom_target(clang-tidy
	COMMENT "Running clang-tidy on all clang-tidy-integrated targets"
	VERBATIM)

function(target_integrate_clang_tidy TARGET LANGUAGE LINE_FILTER_EXCLUDES ARGUMENTS)
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

	# evaluate incompatible generator expressions
	function(eval_incompatible_genexps out_var str)
		set(_result "${str}")
		string(REPLACE "$<C_COMPILER_ID:Clang>" "1" _result "${_result}")
		string(REPLACE "$<CXX_COMPILER_ID:Clang>" "1" _result "${_result}")
		string(REGEX REPLACE "\\$<C_COMPILER_ID:[^>]*>" "0" _result "${_result}")
		string(REGEX REPLACE "\\$<CXX_COMPILER_ID:[^>]*>" "0" _result "${_result}")
		set("${out_var}" "${result}" PARENT_SCOPE)
	endfunction()

	# actual work
	string(REPLACE "X" "+" LANGUAGE_PROPER_NAME "${LANGUAGE}")
	string(TOLOWER "${LANGUAGE_PROPER_NAME}" LANGUAGE_PROPER_NAME)
	set(TARGET_INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>")
	get_target_property(TARGET_COMPILE_OPTIONS "${TARGET}" COMPILE_OPTIONS)
	eval_incompatible_genexps(TARGET_COMPILE_OPTIONS "${TARGET_COMPILE_OPTIONS}")
	get_target_property(TARGET_COMPILE_DEFINITIONS "${TARGET}" COMPILE_DEFINITIONS)
	eval_incompatible_genexps(TARGET_COMPILE_DEFINITIONS "${TARGET_COMPILE_DEFINITIONS}")
	set(CLANG_TIDY_OUTPUTS "")
	get_target_property(TARGET_SOURCES "${TARGET}" SOURCES)

	foreach(TARGET_SOURCE IN LISTS TARGET_SOURCES)
		get_filename_component(TARGET_SOURCE_REAL "${TARGET_SOURCE}" REALPATH BASE_DIR "${PROJECT_SOURCE_DIR}")
		string(SHA3_512 TARGET_SOURCE_HASH "${TARGET_SOURCE_REAL}")
		get_source_file_property(SOURCE_COMPILE_OPTIONS "${TARGET_SOURCE}" COMPILE_OPTIONS)
		eval_incompatible_genexps(SOURCE_COMPILE_OPTIONS "${SOURCE_COMPILE_OPTIONS}")
		get_source_file_property(SOURCE_COMPILE_DEFINITIONS "${TARGET_SOURCE}" COMPILE_DEFINITIONS)
		eval_incompatible_genexps(SOURCE_COMPILE_DEFINITIONS "${SOURCE_COMPILE_DEFINITIONS}")
		get_source_file_property(SOURCE_INCLUDE_DIRECTORIES "${TARGET_SOURCE}" INCLUDE_DIRECTORIES)
		eval_incompatible_genexps(SOURCE_INCLUDE_DIRECTORIES "${SOURCE_INCLUDE_DIRECTORIES}")
		add_custom_command(
			OUTPUT "clang-tidy/${TARGET_SOURCE_HASH}.timestamp"
			COMMAND clang-tidy ARGS
			"--config-file=${ROOT_SOURCE_DIR}/.clang-tidy"
			"--line-filter=[${LINE_FILTER}]"
			"${TARGET_SOURCE_REAL}"
			--
			"$<$<BOOL:${TARGET_INCLUDE_DIRECTORIES}>:-I$<JOIN:${TARGET_INCLUDE_DIRECTORIES},;-I>>"
			"${TARGET_COMPILE_OPTIONS}" "${TARGET_COMPILE_DEFINITIONS}"
			"$<$<BOOL:${SOURCE_INCLUDE_DIRECTORIES}>:-I$<JOIN:${SOURCE_INCLUDE_DIRECTORIES},;-I>>"
			"${SOURCE_COMPILE_OPTIONS}" "${SOURCE_COMPILE_DEFINITIONS}"

			# workaround: make clang-tidy include non-default system headers
			"$<$<BOOL:${CMAKE_${LANGUAGE}_IMPLICIT_INCLUDE_DIRECTORIES}>:-isystem$<JOIN:${CMAKE_${LANGUAGE}_IMPLICIT_INCLUDE_DIRECTORIES},;-isystem>>"

			"-std=${LANGUAGE_PROPER_NAME}${CMAKE_${LANGUAGE}_STANDARD}"
			"${ARGUMENTS}"
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
	add_dependencies(clang-tidy "${TARGET}-clang-tidy")

	if(ARTCCEL_RUN_CLANG_TIDY)
		add_dependencies("${TARGET}" "${TARGET}-clang-tidy")
	endif()
endfunction()
