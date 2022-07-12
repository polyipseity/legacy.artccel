# options
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(ARTCCEL_STANDALONE true)
else()
	set(ARTCCEL_STANDALONE false)
endif()

# external
option(ARTCCEL_INSTALL "Generate and install targets" "${ARTCCEL_STANDALONE}")
option(ARTCCEL_TEST "Build and perform tests" "${ARTCCEL_STANDALONE}")
set(ARTCCEL_NAMESPACE "artccel::" CACHE STRING "Namespace of exported targets")

# build
option(ARTCCEL_PROFILE_COMPILATION "Profile compilation time" false)
option(ARTCCEL_SANITIZE_ADDRESS "Enable address sanitizer" false)
option(ARTCCEL_SANITIZE_MEMORY "Enable memory sanitizer" false)
option(ARTCCEL_SANITIZE_THREAD "Enable thread sanitizer" false)
option(ARTCCEL_SANITIZE_UNDEFINED "Enable undefined behavior sanitizer" false)
