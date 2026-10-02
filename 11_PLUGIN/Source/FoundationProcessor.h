#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// =================================================================================================
// FOUNDATION — audio processor (Phase 0 shell)
//
// Phase 0 scope: accepts MIDI, outputs silence, exposes the registered parameters to the host and
// saves/restores exact state. There is deliberately no editor: until the PLAY implementation
// (Phase 9) hosts show their generic parameter view, so no placeholder UI exists.
//
// Real-time contract: processBlock never allocates, locks, performs I/O or touches UI. It runs
// inside a foundation::rt::ScopedRealtimeScope so the test runner's allocation detector can prove it.
// =================================================================================================

namespace foundation
{

class FoundationProcessor final : public juce::AudioProcessor
{
public:
    FoundationProcessor();
    ~FoundationProcessor() override;

    // Audio
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    [[nodiscard]] bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    using juce::AudioProcessor::processBlock;

    // Editor — none until Phase 9.
    [[nodiscard]] juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    [[nodiscard]] bool hasEditor() const override { return false; }

    // Identity / capabilities
    [[nodiscard]] const juce::String getName() const override;
    [[nodiscard]] bool acceptsMidi() const override { return true; }
    [[nodiscard]] bool producesMidi() const override { return false; }
    [[nodiscard]] bool isMidiEffect() const override { return false; }
    [[nodiscard]] double getTailLengthSeconds() const override { return 0.0; }

    // Programs — presets are handled by the FOUNDATION preset system, not host programs.
    [[nodiscard]] int getNumPrograms() override { return 1; }
    [[nodiscard]] int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    [[nodiscard]] const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    // State
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    [[nodiscard]] juce::AudioProcessorValueTreeState& getParameterState() noexcept { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters;

    // Message-thread state. Guarded by stateLock; never touched by the audio thread.
    juce::CriticalSection stateLock;
    juce::ValueTree retainedUnknownState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FoundationProcessor)
};

} // namespace foundation
