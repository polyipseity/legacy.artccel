cmake_minimum_required(VERSION 3.10)
project(artccel VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_CLANG_TIDY clang-tidy --checks=*,-fuchsia-trailing-return --warnings-as-errors=* --header-filter=.)

find_package(Microsoft.GSL 4.0.0 EXACT REQUIRED CONFIG)

add_library(artccel INTERFACE)
configure_file("in/config.h" "include/artccel/config.h")
target_include_directories(artccel INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> $<INSTALL_INTERFACE:include>)

add_subdirectory(artccel-core)

include(GNUInstallDirs)
install(TARGETS artccel EXPORT artccel-targets)
install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/include/" TYPE INCLUDE)
install(EXPORT artccel-targets DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")
export(EXPORT artccel-targets FILE "Targets.cmake")

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING.COMBINED.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${artccel_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${artccel_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${artccel_VERSION_PATCH}")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(CPack)

include(CMakePackageConfigHelpers)
configure_package_config_file(in/Config.cmake
  "${PROJECT_BINARY_DIR}/Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/ConfigVersion.cmake"
  VERSION "${artccel_VERSION}"
  COMPATIBILITY AnyNewerVersion
)
install(FILES "${PROJECT_BINARY_DIR}/Config.cmake" "${PROJECT_BINARY_DIR}/ConfigVersion.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")
