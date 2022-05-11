cmake_minimum_required(VERSION 3.10)
project(artccel-core VERSION 1.0.0 LANGUAGES C CXX)

add_library(artccel-core SHARED "src/placeholder.cpp")
configure_file("in/config.h" "include/artccel-core/config.h")
target_include_directories(artccel-core PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> $<INSTALL_INTERFACE:include>)
target_link_libraries(artccel-core artccel)

add_executable(artccel-core-exe "src/exe/main.cpp")
target_link_libraries(artccel-core-exe artccel-core)

include(GNUInstallDirs)
install(TARGETS artccel-core artccel-core-exe EXPORT artccel-core-targets)
install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/include/" TYPE INCLUDE)
install(EXPORT artccel-core-targets DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake")
export(EXPORT artccel-core-targets FILE "Targets.cmake")
