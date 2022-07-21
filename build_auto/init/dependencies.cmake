cmake_minimum_required(VERSION 3.16)

include(FetchContent)

# helpers
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE "OPT_IN" CACHE STRING "https://cmake.org/cmake/help/latest/module/FetchContent.html?highlight=fetchcontent_try_find_package_mode#variable:FETCHCONTENT_TRY_FIND_PACKAGE_MODE")

function(find_package_or_fetch_content package target target_options package_options ...)
	if(FETCHCONTENT_TRY_FIND_PACKAGE_MODE STREQUAL "OPT_IN")
		set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE "ALWAYS")
		message(WARNING "'FETCHCONTENT_TRY_FIND_PACKAGE_MODE' is treated as 'ALWAYS'")
	endif()

	if(FETCHCONTENT_TRY_FIND_PACKAGE_MODE STREQUAL "ALWAYS")
		set(_find_package true)
	elseif(FETCHCONTENT_TRY_FIND_PACKAGE_MODE STREQUAL "NEVER")
		set(_find_package false)
	else()
		message(FATAL_ERROR "'FETCHCONTENT_TRY_FIND_PACKAGE_MODE' has invalid value: ${FETCHCONTENT_TRY_FIND_PACKAGE_MODE}")
	endif()

	if(_find_package)
		list(REMOVE_ITEM package_options "REQUIRED")
		find_package("${package}" ${package_options})

		if("${package}_FOUND")
			return()
		endif()
	endif()

	list(LENGTH target_options _target_options_len)
	math(EXPR _target_options_len "${_target_options_len} - 1")

	foreach(_target_option_idx RANGE 0 "${_target_options_len}" 2)
		math(EXPR _target_option_val_idx "${_target_option_idx} + 1")
		list(GET target_options "${_target_option_idx}" _target_option)
		list(GET target_options "${_target_option_val_idx}" _target_option_val)
		set("${_target_option}" "${_target_option_val}")
	endforeach()

	FetchContent_GetProperties("${package}")
	FetchContent_MakeAvailable("${package}")
	list(FIND ARGN EXCLUDE_FROM_ALL _exclude_from_all)
	set_property(DIRECTORY "${${package}_SOURCE_DIR}" PROPERTY EXCLUDE_FROM_ALL "${_exclude_from_all}")
	add_library("${package}::${target}" ALIAS "${target}")
	export(TARGETS "${target}" NAMESPACE "${package}::" FILE "${target}-targets.cmake")
endfunction()

# dependencies
include("${CMAKE_CURRENT_LIST_DIR}/../data/dependencies.cmake")
