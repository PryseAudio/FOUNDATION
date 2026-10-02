# ==================================================================================================
# FOUNDATION — numbered-module helper (ADR-0008)
#
# Each numbered folder that contains code declares exactly one module library:
#
#   foundation_add_module(<name>
#       [SOURCES <files...>]          # .cpp files compiled into every consumer
#       [DEPENDS <foundation modules...>])
#
# Modules are CMake INTERFACE "source libraries": their .cpp files are compiled inside each final
# target that links them (plugin shared code, test runner, tools).
#
# WHY not separately compiled static libraries: juce_add_plugin always compiles the JUCE modules
# into the plugin's own shared-code target. A separately compiled JUCE (or a static library compiled
# against different JUCE configuration macros) would put two copies of JUCE, or two inconsistent
# definitions of JUCE classes, into one binary — duplicate symbols at best, silent ODR violations at
# worst. Compiling module sources inside each consumer guarantees every translation unit in a binary
# sees identical JUCE configuration, while the dependency graph between modules is still enforced:
# a module can only include headers from modules it DEPENDS on.
#
# As with JUCE's own modules, link module libraries with PRIVATE visibility only.
# ==================================================================================================

function(foundation_add_module name)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDS" ${ARGN})

    add_library(${name} INTERFACE)
    add_library(foundation::${name} ALIAS ${name})

    target_include_directories(${name} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Source")

    set(_abs_sources)
    foreach(_src IN LISTS ARG_SOURCES)
        cmake_path(ABSOLUTE_PATH _src BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE _abs)
        list(APPEND _abs_sources "${_abs}")
    endforeach()

    if(_abs_sources)
        target_sources(${name} INTERFACE ${_abs_sources})
        set_property(GLOBAL APPEND PROPERTY FOUNDATION_OWN_SOURCES ${_abs_sources})
    endif()

    if(ARG_DEPENDS)
        target_link_libraries(${name} INTERFACE ${ARG_DEPENDS})
    endif()
endfunction()
