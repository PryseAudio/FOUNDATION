# ==================================================================================================
# FOUNDATION — Catch2 + CTest wiring (ADR-0007)
#
# Catch2 lives in the git submodule External/Catch2, pinned to tag v3.16.0
# (commit 317ac1ed4c0bb6e6b91eafc817e05c488feffcb3). Test-only; never shipped.
# ==================================================================================================

set(FOUNDATION_CATCH2_EXPECTED_VERSION "3.16.0")
set(_foundation_catch2_dir "${PROJECT_SOURCE_DIR}/External/Catch2")

if(NOT EXISTS "${_foundation_catch2_dir}/CMakeLists.txt")
    message(FATAL_ERROR
        "Catch2 not found at '${_foundation_catch2_dir}'.\n"
        "Run: git submodule update --init --recursive")
endif()

file(STRINGS "${_foundation_catch2_dir}/CMakeLists.txt" _catch2_project_line
     REGEX "^[ \t]*VERSION[ \t]+[0-9]+\\.[0-9]+\\.[0-9]+")
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" _catch2_version "${_catch2_project_line}")

if(NOT _catch2_version VERSION_EQUAL FOUNDATION_CATCH2_EXPECTED_VERSION)
    message(FATAL_ERROR
        "Catch2 version mismatch: found '${_catch2_version}', "
        "FOUNDATION is pinned to ${FOUNDATION_CATCH2_EXPECTED_VERSION}.")
endif()

message(STATUS "FOUNDATION: using Catch2 ${_catch2_version}")

add_subdirectory("${_foundation_catch2_dir}" "${CMAKE_BINARY_DIR}/External/Catch2" EXCLUDE_FROM_ALL)
list(APPEND CMAKE_MODULE_PATH "${_foundation_catch2_dir}/extras")
include(Catch)

enable_testing()
