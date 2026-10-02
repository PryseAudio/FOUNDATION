# ==================================================================================================
# FOUNDATION — root build orchestration
#
# Architecture: 02_DOCUMENTATION/04_ARCHITECTURE.md
# Decisions:    02_DOCUMENTATION/ADR/
#
# Configure with a preset (see CMakePresets.json), e.g.:
#   cmake --preset windows-vs2026
#   cmake --build --preset windows-debug
#   ctest --preset windows-debug
# Build output always goes outside the source tree: ../FOUNDATION_build/<preset>.
# ==================================================================================================

cmake_minimum_required(VERSION 3.25...4.4)

project(FOUNDATION
    VERSION 0.0.1
    DESCRIPTION "FOUNDATION — bass sound-design instrument"
    LANGUAGES C CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Use a CMake preset (see CMakePresets.json).")
endif()

# --------------------------------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------------------------------
option(FOUNDATION_BUILD_TESTS         "Build the Catch2 test runner"                       ON)
option(FOUNDATION_BUILD_TOOLS         "Build developer tools (RenderCLI)"                  ON)
option(FOUNDATION_WARNINGS_AS_ERRORS  "Treat warnings in FOUNDATION sources as errors"     ON)
option(FOUNDATION_ENABLE_ASAN         "Build with AddressSanitizer"                        OFF)
option(FOUNDATION_COPY_PLUGIN         "Copy plugin binaries to the system plugin folders"  OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(FoundationCompilerSettings)
include(FoundationIdentity)
include(FoundationModule)
include(FoundationJUCE)

if(FOUNDATION_BUILD_TESTS)
    include(FoundationTesting)
endif()

if(FOUNDATION_ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

# --------------------------------------------------------------------------------------------------
# Modules (dependency order). Folders without Phase 0 code are added in the phase that creates them.
# --------------------------------------------------------------------------------------------------
add_subdirectory(04_DSP/Core)            # foundation_dsp_core
add_subdirectory(12_PARAMETERS_STATE)    # foundation_params_state
add_subdirectory(11_PLUGIN)              # foundation_processor + Foundation plugin

if(FOUNDATION_BUILD_TESTS)
    add_subdirectory(09_TESTS)
endif()

if(FOUNDATION_BUILD_TOOLS)
    add_subdirectory(09_TESTS/Tools/RenderCLI)
endif()

# Must run after every consumer target has been created.
foundation_apply_strict_warnings()
