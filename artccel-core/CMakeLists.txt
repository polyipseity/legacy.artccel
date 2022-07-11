cmake_minimum_required(VERSION 3.14)
project(artccel-core VERSION 1.0.0 LANGUAGES C CXX)
set(ROOT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../")

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
configure_file("in/config.h" "include/artccel/core/config.h")
generate_preset_export_header(artccel-core "include/artccel/core/export.h")
target_include_directories(artccel-core PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> $<INSTALL_INTERFACE:include>)
target_link_libraries(artccel-core PUBLIC artccel Microsoft.GSL::GSL tl-expected::expected PRIVATE Threads::Threads)
add_sanitizers(artccel-core)

add_executable(artccel-core-exe "sources/exe/main.cpp")
add_executable("${ARTCCEL_NAMESPACE}artccel-core-exe" ALIAS artccel-core-exe)
target_link_libraries(artccel-core-exe artccel-core)
add_sanitizers(artccel-core-exe)

if(ARTCCEL_INSTALL)
	include(GNUInstallDirs)
	install(TARGETS artccel-core artccel-core-exe EXPORT artccel-core-targets)
	install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/include/" TYPE INCLUDE)
	install(EXPORT artccel-core-targets DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")
	export(EXPORT artccel-core-targets NAMESPACE "${ARTCCEL_NAMESPACE}" FILE "Targets.cmake")
endif()
