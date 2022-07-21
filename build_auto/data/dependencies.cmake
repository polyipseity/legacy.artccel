cmake_minimum_required(VERSION 3.16)

include(FetchContent)

# dependencies

# Microsoft.GSL
FetchContent_Declare(Microsoft.GSL
	GIT_REPOSITORY "https://github.com/microsoft/GSL.git"
	GIT_TAG "a3534567187d2edc428efd3f13466ff75fe5805c" # tag v4.0.0
	GIT_SHALLOW true
	GIT_PROGRESS true
	FIND_PACKAGE_ARGS
)
find_package_or_fetch_content(Microsoft.GSL GSL
	"GSL_INSTALL;${ARTCCEL_INSTALL};GSL_TEST;${ARTCCEL_TEST}"
	"4.0.0;EXACT;REQUIRED;CONFIG")

# ofats-any_invocable
FetchContent_Declare(ofats-any_invocable
	GIT_REPOSITORY "https://github.com/ofats/any_invocable.git"
	GIT_TAG "853a90ba1b573a957ea2a452404e0cc68ee0b6cc" # tag v1.0.0
	GIT_SHALLOW true
	GIT_PROGRESS true
	FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(ofats-any_invocable)
add_library(ofats::any_invocable INTERFACE IMPORTED)
target_include_directories(ofats::any_invocable INTERFACE "${ofats-any_invocable_SOURCE_DIR}/include")

# tl-expected
FetchContent_Declare(tl-expected
	GIT_REPOSITORY "https://github.com/TartanLlama/expected.git"
	GIT_TAG "3d741708b967b83ca1e2888239196c4a67f9f9b0" # tag v1.0.0
	GIT_SHALLOW true
	GIT_PROGRESS true
	FIND_PACKAGE_ARGS
)
find_package_or_fetch_content(tl-expected expected
	"EXPECTED_BUILD_TESTS;${ARTCCEL_TEST}"
	"1.0.0;EXACT;REQUIRED;CONFIG")
