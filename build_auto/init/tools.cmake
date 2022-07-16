# preconditions
if(NOT DEFINED ROOT_SOURCE_DIR)
	message(FATAL_ERROR "'ROOT_SOURCE_DIR' is undefined")
endif()

# tools
# clang-tidy
add_custom_target(clang-tidy
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
		string(APPEND _line_filters "\
{\"name\": \".hpp\"}, {\"name\": \".cpp\"},\
{\"name\": \".hxx\"}, {\"name\": \".cxx\"},\
{\"name\": \".hh\"}, {\"name\": \".cc\"},\
{\"name\": \".h++\"}, {\"name\": \".c++\"},\
")
	endif()

	# actual work
	set(_target_include_directories "$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>")
	set(_target_compile_definitions "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>")
	get_target_property(_target_compile_options "${target}" COMPILE_OPTIONS)

	if(NOT _target_compile_options)
		set(_target_compile_options "")
	endif()

	eval_incompatible_genexps(_target_compile_options "${_target_compile_options}")

	set(_lang_std "${language}")
	string(REPLACE "X" "+" _lang_std "${_lang_std}")
	string(TOLOWER "${_lang_std}" _lang_std)
	string(APPEND _lang_std "${CMAKE_${language}_STANDARD}")
	eval_incompatible_genexps(arguments "${arguments}")
	get_target_property(_target_sources "${target}" SOURCES)

	foreach(_source IN LISTS _target_sources)
		get_source_file_property(_source_include_directories "${_source}" INCLUDE_DIRECTORIES)

		if(NOT _source_include_directories)
			set(_source_include_directories "")
		endif()

		eval_incompatible_genexps(_source_include_directories "${_source_include_directories}")
		get_source_file_property(_source_compile_definitions "${_source}" COMPILE_DEFINITIONS)

		if(NOT _source_compile_definitions)
			set(_source_compile_definitions "")
		endif()

		eval_incompatible_genexps(_source_compile_definitions "${_source_compile_definitions}")
		get_source_file_property(_source_compile_options "${_source}" COMPILE_OPTIONS)

		if(NOT _source_compile_options)
			set(_source_compile_options "")
		endif()

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
			"$<$<BOOL:${_target_include_directories}>:-I$<JOIN:${_target_include_directories},;-I>>"
			"$<$<BOOL:${_target_compile_definitions}>:-D$<JOIN:${_target_compile_definitions},;-D>>"
			"${_target_compile_options}"
			"$<$<BOOL:${_source_include_directories}>:-I$<JOIN:${_source_include_directories},;-I>>"
			"$<$<BOOL:${_source_compile_definitions}>:-D$<JOIN:${_source_compile_definitions},;-D>>"
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
	add_dependencies(clang-tidy "${target}-clang-tidy")

	if(ARTCCEL_CLANG_TIDY)
		add_dependencies("${target}" "${target}-clang-tidy")
	endif()
endfunction()
