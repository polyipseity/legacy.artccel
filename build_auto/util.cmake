cmake_minimum_required(VERSION 3.16)

# utilities
function(get_subdirectories_recursively out_var directory)
	get_directory_property(_subdirectories DIRECTORY "${directory}" SUBDIRECTORIES)

	if(NOT _subdirectories)
		set(_subdirectories "")
	endif()

	set(_ret "${_subdirectories}")

	foreach(_subdirectory IN LISTS _subdirectories)
		get_subdirectories_recursively(_subsubdirectories "${_subdirectory}")
		list(APPEND _ret ${_subsubdirectories})
	endforeach()

	set("${out_var}" "${_ret}" PARENT_SCOPE)
endfunction()
