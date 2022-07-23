cmake_minimum_required(VERSION 3.16)

# preconditions
if(NOT DEFINED ROOT_SOURCE_DIR)
	message(FATAL_ERROR "'ROOT_SOURCE_DIR' is undefined")
endif()

# tests
if(ARTCCEL_TEST)
	include(CTest)

	if(NOT BUILD_TESTING)
		set(ARTCCEL_TEST false)
	endif()
endif()

add_custom_target("${ARTCCEL_TARGET_NAMESPACE}tests"
	COMMENT "Building all tests"
	VERBATIM)

function(target_as_test target)
	if(NOT TARGET "${target}")
		message(FATAL_ERROR "Not a target: ${target}")
	endif()

	if(NOT ARTCCEL_TEST)
		set_target_properties("${target}" PROPERTIES EXCLUDE_FROM_ALL true)
	endif()

	add_dependencies("${ARTCCEL_TARGET_NAMESPACE}tests" "${target}")
	add_test(NAME "${target}" COMMAND "${target}")
endfunction()

configure_file("${ROOT_SOURCE_DIR}/in/CTestCustom.cmake" "${PROJECT_BINARY_DIR}" @ONLY)
