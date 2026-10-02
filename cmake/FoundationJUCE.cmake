# ==================================================================================================
# FOUNDATION — JUCE location and version pin (ADR-0007)
#
# JUCE lives in the git submodule External/JUCE, pinned to tag 9.0.3
# (commit be29c81492b6151c8ea8d14c840e1311963b3a83).
#
# FOUNDATION_JUCE_PATH may point at another checkout for local experiments, but the version check
# below still applies: a JUCE upgrade is a deliberate change, gated by the golden-render tests.
# ==================================================================================================

set(FOUNDATION_JUCE_EXPECTED_VERSION "9.0.3")
set(FOUNDATION_JUCE_PATH "${PROJECT_SOURCE_DIR}/External/JUCE" CACHE PATH "Path to the pinned JUCE checkout")

set(_foundation_juce_header "${FOUNDATION_JUCE_PATH}/modules/juce_core/system/juce_StandardHeader.h")

if(NOT EXISTS "${_foundation_juce_header}")
    message(FATAL_ERROR
        "JUCE not found at '${FOUNDATION_JUCE_PATH}'.\n"
        "Run: git submodule update --init --recursive")
endif()

file(STRINGS "${_foundation_juce_header}" _juce_major REGEX "^#define JUCE_MAJOR_VERSION[ \t]+[0-9]+")
file(STRINGS "${_foundation_juce_header}" _juce_minor REGEX "^#define JUCE_MINOR_VERSION[ \t]+[0-9]+")
file(STRINGS "${_foundation_juce_header}" _juce_build REGEX "^#define JUCE_BUILDNUMBER[ \t]+[0-9]+")
string(REGEX MATCH "[0-9]+$" _juce_major "${_juce_major}")
string(REGEX MATCH "[0-9]+$" _juce_minor "${_juce_minor}")
string(REGEX MATCH "[0-9]+$" _juce_build "${_juce_build}")
set(FOUNDATION_JUCE_FOUND_VERSION "${_juce_major}.${_juce_minor}.${_juce_build}")

if(NOT FOUNDATION_JUCE_FOUND_VERSION VERSION_EQUAL FOUNDATION_JUCE_EXPECTED_VERSION)
    message(FATAL_ERROR
        "JUCE version mismatch: found ${FOUNDATION_JUCE_FOUND_VERSION} at '${FOUNDATION_JUCE_PATH}', "
        "FOUNDATION is pinned to ${FOUNDATION_JUCE_EXPECTED_VERSION}.")
endif()

message(STATUS "FOUNDATION: using JUCE ${FOUNDATION_JUCE_FOUND_VERSION} from ${FOUNDATION_JUCE_PATH}")

add_subdirectory("${FOUNDATION_JUCE_PATH}" "${CMAKE_BINARY_DIR}/External/JUCE")
