#include "AllocationDetector.h"
#include "FoundationProcessor.h"
#include "RealtimeGuard.h"

#include <catch2/catch_test_macros.hpp>

#include <new>

// =================================================================================================
// Real-time safety: processBlock must perform zero heap operations (01_NON_NEGOTIABLE.md #8).
// =================================================================================================

using namespace foundation;

TEST_CASE("Allocation detector: catches heap operations inside a real-time scope", "[realtime][detector]")
{
    // Proves the detector is live, so a passing processBlock test is meaningful.
    // operator new is called directly: allocations from new-expressions may legally be elided.
    test::ScopedRealtimeHeapCheck check;
    {
        const rt::ScopedRealtimeScope scope;
        void* pointer = ::operator new(64);
        ::operator delete(pointer);
    }
    CHECK(check.allocations() == 1);
    CHECK(check.deallocations() == 1);

    // Outside a scope nothing is counted.
    test::ScopedRealtimeHeapCheck outside;
    void* pointer = ::operator new(64);
    ::operator delete(pointer);
    CHECK(outside.allocations() == 0);
    CHECK(outside.deallocations() == 0);
}

TEST_CASE("processBlock performs no heap operations", "[realtime]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;

    constexpr double sampleRate = 48000.0;
    constexpr int maxBlockSize = 512;

    FoundationProcessor processor;
    processor.setPlayConfigDetails(0, 2, sampleRate, maxBlockSize);
    processor.prepareToPlay(sampleRate, maxBlockSize);

    juce::AudioBuffer<float> buffer(2, maxBlockSize);
    juce::MidiBuffer midi;
    midi.ensureSize(4096);

    const int blockSizes[] = { 1, 7, 32, 64, 128, 333, 512 };

    test::ScopedRealtimeHeapCheck check;

    for (int round = 0; round < 50; ++round)
    {
        for (const int blockSize : blockSizes)
        {
            // Event preparation happens outside the real-time scope, as a host would do it.
            midi.clear();
            midi.addEvent(juce::MidiMessage::noteOn(1, 36 + (round % 24), static_cast<juce::uint8>(100)), 0);
            midi.addEvent(juce::MidiMessage::controllerEvent(1, 1, round % 128), blockSize / 2);
            midi.addEvent(juce::MidiMessage::pitchWheel(1, (round * 97) % 16384), blockSize - 1);
            midi.addEvent(juce::MidiMessage::noteOff(1, 36 + (round % 24)), blockSize - 1);

            // Host automation between blocks.
            for (auto* parameter : processor.getParameters())
                parameter->setValueNotifyingHost(static_cast<float>((round + blockSize) % 100) / 99.0f);

            buffer.setSize(2, blockSize, false, false, true);
            processor.processBlock(buffer, midi);
        }
    }

    CHECK(check.allocations() == 0);
    CHECK(check.deallocations() == 0);

    processor.releaseResources();
}

TEST_CASE("processBlock outputs silence in the Phase 0 shell", "[realtime][shell]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;

    FoundationProcessor processor;
    processor.setPlayConfigDetails(0, 2, 44100.0, 256);
    processor.prepareToPlay(44100.0, 256);

    juce::AudioBuffer<float> buffer(2, 256);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, 0.5f);

    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 40, static_cast<juce::uint8>(127)), 0);
    processor.processBlock(buffer, midi);

    CHECK(buffer.getMagnitude(0, buffer.getNumSamples()) == 0.0f);
    CHECK(midi.isEmpty());
}
