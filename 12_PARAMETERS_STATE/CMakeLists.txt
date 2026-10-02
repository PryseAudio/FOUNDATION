# 12_PARAMETERS_STATE — parameter registry, host parameter layout and state model (foundation_params_state).
#
# Uses juce::AudioProcessorValueTreeState, which lives in juce_audio_processors (that module depends
# on juce_gui_extra). Consumers must therefore link juce::juce_audio_processors. DSP modules
# (Phase 1+) depend only on juce_audio_processors_headless / juce_dsp and never on GUI modules.

foundation_add_module(foundation_params_state
    SOURCES
        Source/ParameterRegistry.cpp
        Source/ParameterLayout.cpp
        Source/StateSerializer.cpp
    DEPENDS
        foundation_dsp_core)
