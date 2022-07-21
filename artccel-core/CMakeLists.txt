cmake_minimum_required(VERSION 3.16)
project(artccel-core VERSION 1.0.0 LANGUAGES C CXX)
set(ROOT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")

include("${ROOT_SOURCE_DIR}/build_auto/init.cmake")

add_library("${ARTCCEL_TARGET_NAMESPACE}-core" SHARED
	"sources/cerrno_extras.cpp"
	"sources/clone.cpp"
	"sources/concurrent.cpp"
	"sources/encoding.cpp"
	"sources/enum_bitset.cpp"
	"sources/error_handling.cpp"
	"sources/geometry.cpp"
	"sources/main_hooks.cpp"
	"sources/polyfill.cpp"
	"sources/reflect.cpp"
	"sources/windows_error.cpp")
add_library("${ARTCCEL_EXPORT_NAMESPACE}${ARTCCEL_TARGET_NAMESPACE}-core" ALIAS "${ARTCCEL_TARGET_NAMESPACE}-core")
configure_file("in/config.h" "include/artccel/core/config.h" @ONLY)
generate_preset_export_header("${ARTCCEL_TARGET_NAMESPACE}-core" "include/artccel/core/export.h" "${ARTCCEL_UUID}")
string(MAKE_C_IDENTIFIER "${ARTCCEL_TARGET_NAMESPACE}-core_EXPORTS" core_EXPORT_DEFINITION)
target_include_directories("${ARTCCEL_TARGET_NAMESPACE}-core" PUBLIC
	"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
	"$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
	"$<INSTALL_INTERFACE:include>")
target_precompile_headers("${ARTCCEL_TARGET_NAMESPACE}-core" PRIVATE "include/artccel/core/precompile.h")
target_link_libraries("${ARTCCEL_TARGET_NAMESPACE}-core" PUBLIC "${ARTCCEL_TARGET_NAMESPACE}" PRIVATE "${ARTCCEL_TARGET_NAMESPACE}-private")
add_sanitizers("${ARTCCEL_TARGET_NAMESPACE}-core")
target_integrate_clang_tidy("${ARTCCEL_TARGET_NAMESPACE}-core" CXX "export.h" "-D${core_EXPORT_DEFINITION}")

get_target_property(core_PRECOMPILE_HEADERS "${ARTCCEL_TARGET_NAMESPACE}-core" PRECOMPILE_HEADERS)

if(NOT core_PRECOMPILE_HEADERS)
	set(core_PRECOMPILE_HEADERS "")
endif()

add_executable("${ARTCCEL_TARGET_NAMESPACE}-core-exe" "sources/exe/main.cpp")
add_executable("${ARTCCEL_EXPORT_NAMESPACE}${ARTCCEL_TARGET_NAMESPACE}-core-exe" ALIAS "${ARTCCEL_TARGET_NAMESPACE}-core-exe")
target_precompile_headers("${ARTCCEL_TARGET_NAMESPACE}-core-exe" PRIVATE ${core_PRECOMPILE_HEADERS})
target_link_libraries("${ARTCCEL_TARGET_NAMESPACE}-core-exe" "${ARTCCEL_TARGET_NAMESPACE}-core")
add_sanitizers("${ARTCCEL_TARGET_NAMESPACE}-core-exe")
target_integrate_clang_tidy("${ARTCCEL_TARGET_NAMESPACE}-core-exe" CXX "export.h" "")

if(ARTCCEL_INSTALL)
	include(GNUInstallDirs)
	install(TARGETS "${ARTCCEL_TARGET_NAMESPACE}-core" "${ARTCCEL_TARGET_NAMESPACE}-core-exe" EXPORT "${ARTCCEL_TARGET_NAMESPACE}-core-targets")
	install(DIRECTORY "include/" "${PROJECT_BINARY_DIR}/include/" TYPE INCLUDE)
	install(EXPORT "${ARTCCEL_TARGET_NAMESPACE}-core-targets" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" NAMESPACE "${ARTCCEL_EXPORT_NAMESPACE}")
	export(EXPORT "${ARTCCEL_TARGET_NAMESPACE}-core-targets" NAMESPACE "${ARTCCEL_EXPORT_NAMESPACE}")
endif()
