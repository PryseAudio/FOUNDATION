# 09_TESTS — Catch2 test runner (foundation_tests).
#
# Tests link the FOUNDATION modules directly (not the plugin binary). The allocation detector
# replaces global operator new/delete for this executable only.

juce_add_console_app(foundation_tests
    PRODUCT_NAME "foundation_tests")

target_sources(foundation_tests PRIVATE
    Support/AllocationDetector.cpp
    Unit/ParameterRegistryTests.cpp
    Unit/ParameterIdStabilityTests.cpp
    State/StateRoundTripTests.cpp
    Realtime/ProcessBlockAllocationTests.cpp)

target_include_directories(foundation_tests PRIVATE Support)

target_compile_definitions(foundation_tests PRIVATE
    FOUNDATION_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Golden")

target_link_libraries(foundation_tests PRIVATE
    foundation_processor
    juce::juce_audio_processors
    Catch2::Catch2WithMain)

foundation_configure_juce_target(foundation_tests)

catch_discover_tests(foundation_tests
    DISCOVERY_MODE PRE_TEST
    TEST_PREFIX "foundation.")
