cmake_minimum_required(VERSION 3.16)

# generation
function(generate_preset_export_header target filename namespace)
	if(NOT TARGET "${target}")
		message(FATAL_ERROR "Not a target: ${target}")
	endif()

	string(UUID _include_guard_name NAMESPACE "${namespace}" NAME "${target}" TYPE SHA1 UPPER)
	string(MAKE_C_IDENTIFIER "${_include_guard_name}" _include_guard_name)
	string(PREPEND _include_guard_name "GUARD_")

	string(MAKE_C_IDENTIFIER "${target}" _target_macro_lowercase)
	string(TOUPPER "${_target_macro_lowercase}" _target_macro)
	set(CUSTOM_EXPORT_HEADER "
#ifdef ${_target_macro}_STATIC_DEFINE
#	define ${_target_macro}_EXPORT_DECLARATION
#	define ${_target_macro}_EXPORT_DEFINITION
#else
#	ifndef ${_target_macro}_EXPORT_DECLARATION
#		ifdef ${_target_macro_lowercase}_EXPORTS
#			ifdef _MSC_VER
#				define ${_target_macro}_EXPORT_DECLARATION
#			else
#				define ${_target_macro}_EXPORT_DECLARATION ${_target_macro}_EXPORT
#			endif
#		else
#			define ${_target_macro}_EXPORT_DECLARATION ${_target_macro}_EXPORT
#		endif
#	endif
#	ifndef ${_target_macro}_EXPORT_DEFINITION
#		ifdef ${_target_macro_lowercase}_EXPORTS
#			ifdef _MSC_VER
#				define ${_target_macro}_EXPORT_DEFINITION ${_target_macro}_EXPORT
#			else
#				define ${_target_macro}_EXPORT_DEFINITION
#			endif
#		else
#			define ${_target_macro}_EXPORT_DEFINITION
#		endif
#	endif
#endif
")

	include(GenerateExportHeader)
	generate_export_header("${target}"
		EXPORT_FILE_NAME "${filename}"
		INCLUDE_GUARD_NAME "${_include_guard_name}"
		CUSTOM_CONTENT_FROM_VARIABLE CUSTOM_EXPORT_HEADER)
endfunction()
