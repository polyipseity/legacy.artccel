cmake_minimum_required(VERSION 3.16)

# preconditions
if(NOT DEFINED ROOT_SOURCE_DIR)
	message(FATAL_ERROR "'ROOT_SOURCE_DIR' is undefined")
endif()

# tools
function(prepend_env_to_compiler_launchers)
	get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)

	foreach(_language IN LISTS _languages)
		list(PREPEND "CMAKE_${_language}_COMPILER_LAUNCHER"
			"${CMAKE_COMMAND}" -E env
			${${_language}_COMPILER_LAUNCHER_ENV})
		set("CMAKE_${_language}_COMPILER_LAUNCHER" ${CMAKE_${_language}_COMPILER_LAUNCHER} PARENT_SCOPE)
	endforeach()
endfunction()

# https://crascit.com/2016/04/09/using-ccache-with-cmake/
function(port_compiler_launchers_to_xcode)
	if(CMAKE_GENERATOR STREQUAL "Xcode")
		get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)

		foreach(_language IN LISTS _languages)
			list(JOIN _command_line " " "CMAKE_${_language}_COMPILER_LAUNCHER")

			if(_language STREQUAL "C")
				set(CMAKE_XCODE_ATTRIBUTE_CC "${_command_line} ${CMAKE_C_COMPILER}" PARENT_SCOPE)
				set(CMAKE_XCODE_ATTRIBUTE_LD "${_command_line} ${CMAKE_C_COMPILER}" PARENT_SCOPE)
			elseif(_language STREQUAL "CXX")
				set(CMAKE_XCODE_ATTRIBUTE_CXX "${_command_line} ${CMAKE_CXX_COMPILER}" PARENT_SCOPE)
				set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS "${_command_line} ${CMAKE_CXX_COMPILER}" PARENT_SCOPE)
			else()
				set("CMAKE_XCODE_ATTRIBUTE_${_language}" "${_command_line} ${CMAKE_${_language}_COMPILER}" PARENT_SCOPE)
			endif()
		endforeach()

		message(STATUS "Ported compiler launchers to Xcode, they may not work; check Xcode attributes")
	endif()
endfunction()

# ccache
if(ARTCCEL_CCACHE)
	find_program(CCACHE_PROGRAM ccache)

	if(CCACHE_PROGRAM)
		set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
		set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
		list(APPEND C_COMPILER_LAUNCHER_ENV ${ARTCCEL_CCACHE_OPTIONS})
		list(APPEND CXX_COMPILER_LAUNCHER_ENV ${ARTCCEL_CCACHE_OPTIONS})
		message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
	else()
		message(WARNING "Cannot find ccache")
	endif()
endif()

# clang-tidy
add_custom_target("${ARTCCEL_TARGET_NAMESPACE}clang-tidy"
	COMMENT "Running clang-tidy on all clang-tidy-integrated targets"
	VERBATIM)

function(target_integrate_clang_tidy target language link_filter_excludes arguments)
	if(NOT TARGET "${target}")
		message(FATAL_ERROR "Not a target: ${target}")
	endif()

	set(_valid_languages "C" "CXX")

	if(NOT language IN_LIST _valid_languages)
		message(FATAL_ERROR "clang-tidy does not support language: ${language}")
	endif()

	# evaluate incompatible generator expressions
	function(eval_incompatible_genexps out_var str)
		set(_result "${str}")
		string(REPLACE "$<C_COMPILER_ID>" "Clang" _result "${_result}")
		string(REPLACE "$<C_COMPILER_ID:Clang>" "1" _result "${_result}")
		string(REGEX REPLACE "\\$<C_COMPILER_ID:[^>]*>" "0" _result "${_result}")

		if(language STREQUAL "CXX")
			string(REPLACE "$<CXX_COMPILER_ID>" "Clang" _result "${_result}")
			string(REPLACE "$<CXX_COMPILER_ID:Clang>" "1" _result "${_result}")
			string(REGEX REPLACE "\\$<CXX_COMPILER_ID:[^>]*>" "0" _result "${_result}")
		endif()

		set("${out_var}" "${_result}" PARENT_SCOPE)
	endfunction()

	# workaround: make up for a lack of a usable filter
	eval_incompatible_genexps(link_filter_excludes "${link_filter_excludes}")

	foreach(_exclude IN LISTS link_filter_excludes)
		string(APPEND _line_filters "{\"name\": \"${_exclude}\", \"lines\": [[2, 1]]},")
	endforeach()

	string(APPEND _line_filters "{\"name\": \".h\"}, {\"name\": \".c\"},")

	if(language STREQUAL "CXX")
		string(APPEND _line_filters [[
{"name": ".hpp"}, {"name": ".cpp"},
{"name": ".hxx"}, {"name": ".cxx"},
{"name": ".hh" }, {"name": ".cc" },
{"name": ".h++"}, {"name": ".c++"},
]])
	endif()

	# actual work
	set(_target_compile_definitions "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>")
	set(_target_include_directories "$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>")
	get_target_property(_target_compile_options "${target}" COMPILE_OPTIONS)

	if(NOT _target_compile_options)
		set(_target_compile_options "")
	endif()

	eval_incompatible_genexps(_target_compile_options "${_target_compile_options}")

	string(REPLACE "X" "+" _lang_std "${language}")
	get_target_property(_target_lang_std "${target}" "${language}_STANDARD")
	string(APPEND _lang_std "${_target_lang_std}")
	string(TOLOWER "${_lang_std}" _lang_std)
	eval_incompatible_genexps(arguments "${arguments}")
	get_target_property(_target_sources "${target}" SOURCES)

	foreach(_source IN LISTS _target_sources)
		get_source_file_property(_source_compile_definitions "${_source}" COMPILE_DEFINITIONS)
		get_source_file_property(_source_include_directories "${_source}" INCLUDE_DIRECTORIES)
		get_source_file_property(_source_compile_options "${_source}" COMPILE_OPTIONS)

		if(NOT _source_compile_definitions)
			set(_source_compile_definitions "")
		endif()

		if(NOT _source_include_directories)
			set(_source_include_directories "")
		endif()

		if(NOT _source_compile_options)
			set(_source_compile_options "")
		endif()

		eval_incompatible_genexps(_source_compile_definitions "${_source_compile_definitions}")
		eval_incompatible_genexps(_source_include_directories "${_source_include_directories}")
		eval_incompatible_genexps(_source_compile_options "${_source_compile_options}")

		get_filename_component(_source_real "${_source}" REALPATH BASE_DIR "${PROJECT_SOURCE_DIR}")
		string(SHA3_512 _source_hash "${_source_real}")
		add_custom_command(
			OUTPUT "clang-tidy/${_source_hash}.timestamp"
			COMMAND clang-tidy ARGS
			"--config-file=${ROOT_SOURCE_DIR}/.clang-tidy"
			"--line-filter=[${_line_filters}]"
			"${_source_real}"
			--
			"$<$<BOOL:${_target_compile_definitions}>:-D$<JOIN:${_target_compile_definitions},;-D>>"
			"$<$<BOOL:${_target_include_directories}>:-I$<JOIN:${_target_include_directories},;-I>>"
			"${_target_compile_options}"
			"$<$<BOOL:${_source_compile_definitions}>:-D$<JOIN:${_source_compile_definitions},;-D>>"
			"$<$<BOOL:${_source_include_directories}>:-I$<JOIN:${_source_include_directories},;-I>>"
			"${_source_compile_options}"

			# workaround: make clang-tidy include non-default system headers
			"$<$<BOOL:${CMAKE_${language}_IMPLICIT_INCLUDE_DIRECTORIES}>:-isystem$<JOIN:${CMAKE_${language}_IMPLICIT_INCLUDE_DIRECTORIES},;-isystem>>"

			"-std=${_lang_std}" "${arguments}"
			COMMAND "${CMAKE_COMMAND}" ARGS -E touch "${PROJECT_BINARY_DIR}/clang-tidy/${_source_hash}.timestamp"
			DEPENDS "${_source_real}" "${ROOT_SOURCE_DIR}/.clang-tidy" # MAIN_DEPENDENCY silently overwrites compilation
			IMPLICIT_DEPENDS "${language}" "${_source_real}"
			COMMENT "Running clang-tidy on '${_source}' of '${target}'"
			VERBATIM COMMAND_EXPAND_LISTS # dumb generator expression with spaces is a time waster
		)
		list(APPEND _clang_tidy_outputs "clang-tidy/${_source_hash}.timestamp")
	endforeach()

	add_custom_target("${target}-clang-tidy"
		DEPENDS ${_clang_tidy_outputs}
		COMMENT "Running clang-tidy on '${target}'"
		VERBATIM
	)
	add_dependencies("${ARTCCEL_TARGET_NAMESPACE}clang-tidy" "${target}-clang-tidy")

	if(ARTCCEL_CLANG_TIDY)
		add_dependencies("${target}" "${target}-clang-tidy")
	endif()
endfunction()

# deferred
prepend_env_to_compiler_launchers()
port_compiler_launchers_to_xcode()
