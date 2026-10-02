# 09_TESTS/Tools/RenderCLI — offline renderer (foundation_render_cli).
#
# Renders the FOUNDATION processor to a WAV file from an optional MIDI file and optional saved
# state. Used for golden-render regression tests and for sound design before the UI exists.

juce_add_console_app(foundation_render_cli
    PRODUCT_NAME "foundation_render_cli")

target_sources(foundation_render_cli PRIVATE Main.cpp)

target_link_libraries(foundation_render_cli PRIVATE
    foundation_processor
    juce::juce_audio_processors
    juce::juce_audio_formats)

foundation_configure_juce_target(foundation_render_cli)

if(FOUNDATION_BUILD_TESTS)
    add_test(NAME foundation.render_cli.smoke
             COMMAND foundation_render_cli
                     --out "${CMAKE_CURRENT_BINARY_DIR}/render_cli_smoke.wav"
                     --seconds 1 --sample-rate 48000 --block 480 --note 36)
endif()
