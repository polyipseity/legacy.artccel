cmake_minimum_required(VERSION 3.16)
project(artccel VERSION 1.0.0 LANGUAGES C CXX)
set(ROOT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")

include("${ROOT_SOURCE_DIR}/build_auto/init.cmake")

add_library("${ARTCCEL_TARGET_NAMESPACE}" INTERFACE)
add_library("${ARTCCEL_EXPORT_NAMESPACE}${ARTCCEL_TARGET_NAMESPACE}" ALIAS "${ARTCCEL_TARGET_NAMESPACE}")
configure_file("in/config.h" "include/artccel/config.h" @ONLY)
target_include_directories("${ARTCCEL_TARGET_NAMESPACE}" INTERFACE
	"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
	"$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
	"$<INSTALL_INTERFACE:include>")
target_link_libraries("${ARTCCEL_TARGET_NAMESPACE}" INTERFACE
	Microsoft.GSL::GSL
	Threads::Threads
	ofats::any_invocable
	tl-expected::expected)

add_library("${ARTCCEL_TARGET_NAMESPACE}-private" INTERFACE)
target_precompile_headers("${ARTCCEL_TARGET_NAMESPACE}-private" INTERFACE "include/artccel/precompile.h")

add_subdirectory(artccel-core)

if(ARTCCEL_INSTALL)
	include(GNUInstallDirs)
	install(TARGETS "${ARTCCEL_TARGET_NAMESPACE}" EXPORT "${ARTCCEL_TARGET_NAMESPACE}-targets")
	install(DIRECTORY "include/" "${PROJECT_BINARY_DIR}/include/" TYPE INCLUDE)
	install(EXPORT "${ARTCCEL_TARGET_NAMESPACE}-targets" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" NAMESPACE "${ARTCCEL_NAMESPACE}")
	export(EXPORT "${ARTCCEL_TARGET_NAMESPACE}-targets" NAMESPACE "${ARTCCEL_EXPORT_NAMESPACE}")

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
