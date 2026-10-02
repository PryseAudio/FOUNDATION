# ==================================================================================================
# FOUNDATION — compiler settings
#
#   - C++20 for all FOUNDATION code (JUCE itself only requires C++17).
#   - Static MSVC runtime (/MT) so the plugin carries no VC++ redistributable dependency.
#     Set globally, before any target exists, so JUCE, Catch2 and our code all agree.
#   - Never fast-math: it breaks NaN/Inf guards and cross-build determinism.
#   - Strict warnings (and warnings-as-errors) apply to FOUNDATION's own sources only. JUCE and
#     Catch2 are third-party code compiled inside our targets and are not held to our warning level.
# ==================================================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Group the many JUCE/Catch2 targets in IDE solution views.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# --------------------------------------------------------------------------------------------------
# Applied to every final target that compiles JUCE (plugin shared code, tests, tools), so every
# translation unit in a binary sees identical JUCE configuration.
# --------------------------------------------------------------------------------------------------
function(foundation_configure_juce_target target)
    target_compile_definitions(${target} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        FOUNDATION_PRODUCT_VERSION="${PROJECT_VERSION}")

    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8 /Zc:__cplusplus /permissive- /bigobj>)

    target_link_libraries(${target} PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

    set_property(GLOBAL APPEND PROPERTY FOUNDATION_CONSUMER_TARGETS ${target})
endfunction()

# --------------------------------------------------------------------------------------------------
# Strict warnings on FOUNDATION-owned source files only.
#
# Module sources are compiled inside each consumer target (see FoundationModule.cmake), so the
# options are attached as source-file properties in each consumer target's directory scope.
# Angle-bracket includes (<juce_*/...>, <catch2/...>, the standard library) are treated as external
# headers and do not produce warnings in our translation units.
# --------------------------------------------------------------------------------------------------
function(foundation_apply_strict_warnings)
    if(MSVC)
        set(_strict /W4 /external:anglebrackets /external:W0)
        if(FOUNDATION_WARNINGS_AS_ERRORS)
            list(APPEND _strict /WX)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(_strict -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor
                    --system-header-prefix=juce_ --system-header-prefix=catch2/)
        if(FOUNDATION_WARNINGS_AS_ERRORS)
            list(APPEND _strict -Werror)
        endif()
    else()
        set(_strict -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor)
        if(FOUNDATION_WARNINGS_AS_ERRORS)
            list(APPEND _strict -Werror)
        endif()
    endif()

    get_property(_own_module_sources GLOBAL PROPERTY FOUNDATION_OWN_SOURCES)
    get_property(_consumers GLOBAL PROPERTY FOUNDATION_CONSUMER_TARGETS)

    foreach(_target IN LISTS _consumers)
        get_target_property(_target_dir ${_target} SOURCE_DIR)
        get_target_property(_target_sources ${_target} SOURCES)

        set(_files ${_own_module_sources})
        foreach(_src IN LISTS _target_sources)
            if(_src MATCHES "^\\$<")   # skip generator expressions
                continue()
            endif()
            cmake_path(ABSOLUTE_PATH _src BASE_DIRECTORY "${_target_dir}" OUTPUT_VARIABLE _abs)
            cmake_path(IS_PREFIX PROJECT_SOURCE_DIR "${_abs}" NORMALIZE _inside_project)
            cmake_path(IS_PREFIX CMAKE_BINARY_DIR "${_abs}" NORMALIZE _inside_build)
            string(FIND "${_abs}" "/External/" _external_pos)
            if(_inside_project AND NOT _inside_build AND _external_pos EQUAL -1)
                list(APPEND _files "${_abs}")
            endif()
        endforeach()

        list(REMOVE_DUPLICATES _files)
        foreach(_file IN LISTS _files)
            if(_file MATCHES "\\.(cpp|cc|cxx)$")
                set_property(SOURCE "${_file}" DIRECTORY "${_target_dir}"
                             APPEND PROPERTY COMPILE_OPTIONS ${_strict})
            endif()
        endforeach()
    endforeach()
endfunction()
