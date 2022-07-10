cmake_minimum_required(VERSION 3.14)
project(artccel VERSION 1.0.0 LANGUAGES C CXX)
set(ROOT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/")

include("${ROOT_SOURCE_DIR}/build_auto/init.cmake")
include("${ROOT_SOURCE_DIR}/build_auto/dependencies.cmake")

add_library(artccel INTERFACE)
add_library("${ARTCCEL_NAMESPACE}artccel" ALIAS artccel)
configure_file("in/config.h" "include/artccel/config.h")
target_include_directories(artccel INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> $<INSTALL_INTERFACE:include>)

add_subdirectory(artccel-core)

if(ARTCCEL_INSTALL)
	include(GNUInstallDirs)
	install(TARGETS artccel EXPORT artccel-targets)
	install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/include/" TYPE INCLUDE)
	install(EXPORT artccel-targets DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")
	export(EXPORT artccel-targets NAMESPACE "${ARTCCEL_NAMESPACE}" FILE "Targets.cmake")

	include(CMakePackageConfigHelpers)
	configure_package_config_file(
		"in/config.cmake"
		"${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
		INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake"
	)
	write_basic_package_version_file(
		"${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
		COMPATIBILITY AnyNewerVersion
	)
	install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake" "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")
endif()

if(ARTCCEL_STANDALONE)
	include(InstallRequiredSystemLibraries)
	set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING.COMBINED.txt")
	set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
	set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
	set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
	set(CPACK_SOURCE_GENERATOR "TGZ")
	include(CPack)
endif()
