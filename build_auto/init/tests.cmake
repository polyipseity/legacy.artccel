cmake_minimum_required(VERSION 3.16)

# tests
include(CTest)

if(NOT BUILD_TESTING)
	set(ARTCCEL_TEST false)
endif()

if(ARTCCEL_TEST)
	set(ARTCCEL_TEST_EXCLUDE_FROM_ALL "")
else()
	set(ARTCCEL_TEST_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
endif()

add_custom_target("${ARTCCEL_TARGET_NAMESPACE}-tests"
	COMMENT "Building all tests"
	VERBATIM)

function(target_as_test target)
	if(NOT TARGET "${target}")
		message(FATAL_ERROR "Not a target: ${target}")
	endif()

	if(ARTCCEL_TEST)
		set(_target_exclude_from_all false)
	else()
		set(_target_exclude_from_all true)
	endif()

	set_target_properties("${target}" PROPERTIES EXCLUDE_FROM_ALL "${_target_exclude_from_all}")
	add_dependencies("${ARTCCEL_TARGET_NAMESPACE}-tests" "${target}")
	add_test(NAME "${target}" COMMAND "${target}")
endfunction()
