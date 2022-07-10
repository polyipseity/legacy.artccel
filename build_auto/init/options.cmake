# preconditions
if(NOT DEFINED ROOT_SOURCE_DIR)
	message(FATAL_ERROR "'ROOT_SOURCE_DIR' is undefined")
endif()

# options
if(CMAKE_SOURCE_DIR STREQUAL ROOT_SOURCE_DIR)
	set(ARTCCEL_STANDALONE true)
else()
	set(ARTCCEL_STANDALONE false)
endif()

option(ARTCCEL_INSTALL "Generate and install targets" "${ARTCCEL_STANDALONE}")
option(ARTCCEL_TEST "Build and perform tests" "${ARTCCEL_STANDALONE}")
set(ARTCCEL_NAMESPACE "artccel::" CACHE STRING "Namespace of exported targets")
