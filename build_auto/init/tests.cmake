cmake_minimum_required(VERSION 3.16)

# tests
include(CTest)

if(NOT BUILD_TESTING)
	set(ARTCCEL_TEST false)
endif()

add_custom_target("${ARTCCEL_TARGET_NAMESPACE}-tests"
	COMMENT "Building all tests"
	VERBATIM)

function(target_as_test target)
	if(NOT TARGET "${target}")
		message(FATAL_ERROR "Not a target: ${target}")
	endif()

	if(NOT ARTCCEL_TEST)
		set_target_properties("${target}" PROPERTIES EXCLUDE_FROM_ALL true)
	endif()

	add_dependencies("${ARTCCEL_TARGET_NAMESPACE}-tests" "${target}")
	add_test(NAME "${target}" COMMAND "${target}")
endfunction()
