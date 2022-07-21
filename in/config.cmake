cmake_minimum_required(VERSION 3.16)

@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/artccel-targets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/artccel-core-targets.cmake")
check_required_components(artccel)
