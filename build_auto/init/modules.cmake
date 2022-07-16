cmake_minimum_required(VERSION 3.16)

# preconditions
if(NOT DEFINED ROOT_SOURCE_DIR)
	message(FATAL_ERROR "'ROOT_SOURCE_DIR' is undefined")
endif()

# modules
list(APPEND CMAKE_MODULE_PATH "${ROOT_SOURCE_DIR}/externals/sanitizers-cmake/cmake")

# sanitizers
set(SANITIZE_ADDRESS "${ARTCCEL_SANITIZE_ADDRESS}")
set(SANITIZE_MEMORY "${ARTCCEL_SANITIZE_MEMORY}")
set(SANITIZE_THREAD "${ARTCCEL_SANITIZE_THREAD}")
set(SANITIZE_UNDEFINED "${ARTCCEL_SANITIZE_UNDEFINED}")
find_package(Sanitizers)
sanitizer_add_blacklist_file("${ROOT_SOURCE_DIR}/sanitize-ignorelist.txt")
