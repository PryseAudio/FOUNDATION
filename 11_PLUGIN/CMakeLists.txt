# 11_PLUGIN — plugin shell: processor (foundation_processor) and the Foundation plugin target.
#
# The processor is a module so the test runner and RenderCLI exercise the exact same code the
# plugin ships. Only the plugin target defines FOUNDATION_PLUGIN_ENTRY (createPluginFilter) and sees
# the JucePlugin_* identity macros; no other module may depend on them.

foundation_add_module(foundation_processor
    SOURCES
        Source/FoundationProcessor.cpp
    DEPENDS
        foundation_params_state
        foundation_dsp_core)

set(_foundation_formats VST3 Standalone)
if(APPLE)
    list(APPEND _foundation_formats AU)
endif()

if(FOUNDATION_COPY_PLUGIN)
    set(_foundation_copy_plugin TRUE)
else()
    set(_foundation_copy_plugin FALSE)
endif()

juce_add_plugin(Foundation
    PRODUCT_NAME                "${FOUNDATION_PRODUCT_NAME}"
    PLUGIN_NAME                 "${FOUNDATION_PLUGIN_NAME}"
    COMPANY_NAME                "${FOUNDATION_COMPANY_NAME}"
    BUNDLE_ID                   "${FOUNDATION_BUNDLE_ID}"
    PLUGIN_MANUFACTURER_CODE    "${FOUNDATION_PLUGIN_MANUFACTURER_CODE}"
    PLUGIN_CODE                 "${FOUNDATION_PLUGIN_CODE}"
    DESCRIPTION                 "Bass sound-design instrument"
    FORMATS                     ${_foundation_formats}
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     ${_foundation_copy_plugin}
    VST3_CATEGORIES             Instrument Synth
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice
    HARDENED_RUNTIME_ENABLED    TRUE)

target_compile_definitions(Foundation PRIVATE FOUNDATION_PLUGIN_ENTRY=1)

target_link_libraries(Foundation PRIVATE
    foundation_processor
    juce::juce_audio_utils)

foundation_configure_juce_target(Foundation)
