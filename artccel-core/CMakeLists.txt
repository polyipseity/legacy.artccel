cmake_minimum_required(VERSION 3.10)
project(artccel-core VERSION 1.0.0 LANGUAGES C CXX)
set(ROOT_DIR "${PROJECT_SOURCE_DIR}/../")

include("${ROOT_DIR}/build_auto/init.cmake")

add_library(artccel-core SHARED
	"sources/cerrno_extras.cpp"
	"sources/compute.cpp"
	"sources/encoding.cpp"
	"sources/main_hooks.cpp"
	"sources/template_instantiations.cpp")
configure_file("in/config.h" "include/artccel-core/config.h")
generate_preset_export_header(artccel-core)
target_include_directories(artccel-core PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_include_directories(artccel-core SYSTEM PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_link_libraries(artccel-core PUBLIC artccel Microsoft.GSL::GSL PRIVATE Threads::Threads)

add_executable(artccel-core-exe "sources/exe/main.cpp")
target_link_libraries(artccel-core-exe artccel-core)

include(GNUInstallDirs)
install(TARGETS artccel-core artccel-core-exe EXPORT artccel-core-targets)
install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/include/" TYPE INCLUDE)
install(EXPORT artccel-core-targets DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")
export(EXPORT artccel-core-targets FILE "Targets.cmake")
