cmake_minimum_required(VERSION 3.16)
project(artccel-core VERSION 1.0.0 LANGUAGES C CXX)
set(ROOT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")

include("${ROOT_SOURCE_DIR}/build_auto/init.cmake")

add_library(artccel-core SHARED
	"sources/cerrno_extras.cpp"
	"sources/clone.cpp"
	"sources/compute.cpp"
	"sources/concurrent.cpp"
	"sources/encoding.cpp"
	"sources/enum_bitset.cpp"
	"sources/error_handling.cpp"
	"sources/geometry.cpp"
	"sources/interval.cpp"
	"sources/main_hooks.cpp"
	"sources/polyfill.cpp"
	"sources/reflect.cpp"
	"sources/windows_error.cpp")
add_library("${ARTCCEL_NAMESPACE}artccel-core" ALIAS artccel-core)
configure_file("in/config.h" "include/artccel/core/config.h" @ONLY)
generate_preset_export_header(artccel-core "include/artccel/core/export.h")
target_include_directories(artccel-core PUBLIC
	"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
	"$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
	"$<INSTALL_INTERFACE:include>")
target_precompile_headers(artccel-core PRIVATE "include/artccel/core/precompile.h")
target_link_libraries(artccel-core PUBLIC artccel PRIVATE artccel-private)
add_sanitizers(artccel-core)
target_integrate_clang_tidy(artccel-core CXX "" "-Dartccel_core_EXPORTS")

add_executable(artccel-core-exe "sources/exe/main.cpp")
add_executable("${ARTCCEL_NAMESPACE}artccel-core-exe" ALIAS artccel-core-exe)
get_target_property(artccel-core_PRECOMPILE_HEADERS artccel-core PRECOMPILE_HEADERS)

if(NOT artccel-core_PRECOMPILE_HEADERS)
	set(artccel-core_PRECOMPILE_HEADERS "")
endif()

target_precompile_headers(artccel-core-exe PRIVATE ${artccel-core_PRECOMPILE_HEADERS})
target_link_libraries(artccel-core-exe artccel-core)
add_sanitizers(artccel-core-exe)
target_integrate_clang_tidy(artccel-core-exe CXX "" "")

if(ARTCCEL_INSTALL)
	include(GNUInstallDirs)
	install(TARGETS artccel-core artccel-core-exe EXPORT artccel-core-targets)
	install(DIRECTORY "include/" "${PROJECT_BINARY_DIR}/include/" TYPE INCLUDE)
	install(EXPORT artccel-core-targets DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" NAMESPACE "${ARTCCEL_NAMESPACE}")
	export(EXPORT artccel-core-targets NAMESPACE "${ARTCCEL_NAMESPACE}")
endif()
