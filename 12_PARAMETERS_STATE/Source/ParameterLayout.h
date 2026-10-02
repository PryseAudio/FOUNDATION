#pragma once

#include "ParameterRegistry.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <string_view>

// =================================================================================================
// FOUNDATION — host parameter layout, generated from the ParameterRegistry.
//
// Continuous parameters use FoundationFloatParameter rather than juce::AudioParameterFloat because
// exact state recall is a hard requirement (01_NON_NEGOTIABLE.md #9, ADR-0006):
//
//   - The plain value is the stored truth. setPlainValueExact() restores a saved plain value
//     bit-for-bit. (Routing a plain value through the normalised 0..1 domain and back is not
//     guaranteed to reproduce the same float.)
//   - setValue() ignores a normalised value equal to the one it currently reports, so a host or
//     wrapper echoing the current value back cannot perturb an exactly-restored plain value.
//   - The audio thread reads the plain value through plainValuePointer(): one atomic load, no
//     lookups, no allocation.
//
// Choice parameters use juce::AudioParameterChoice: integer indices round-trip exactly.
// =================================================================================================

namespace foundation::params
{

class FoundationFloatParameter final : public juce::RangedAudioParameter
{
public:
    explicit FoundationFloatParameter(const ParameterSpec& spec);

    [[nodiscard]] const ParameterSpec& getSpec() const noexcept { return spec; }

    /** Current plain value. Safe on any thread. */
    [[nodiscard]] float getPlainValue() const noexcept { return plain.load(std::memory_order_relaxed); }

    /** Pointer for real-time reads of the plain value. Valid for the parameter's lifetime. */
    [[nodiscard]] const std::atomic<float>* plainValuePointer() const noexcept { return &plain; }

    /** Restores an exact plain value (clamped to the legal range) and notifies listeners/host.
        Used by state recall. Not for the audio thread. */
    void setPlainValueExact(float newPlainValue);

    // juce::RangedAudioParameter
    [[nodiscard]] const juce::NormalisableRange<float>& getNormalisableRange() const override { return range; }

    // juce::AudioProcessorParameter
    [[nodiscard]] float getValue() const override;
    void setValue(float newNormalisedValue) override;
    [[nodiscard]] float getDefaultValue() const override;
    [[nodiscard]] juce::String getName(int maximumStringLength) const override;
    [[nodiscard]] juce::String getLabel() const override;
    [[nodiscard]] juce::String getText(float normalisedValue, int maximumStringLength) const override;
    [[nodiscard]] float getValueForText(const juce::String& text) const override;
    [[nodiscard]] bool isAutomatable() const override { return spec.automatable; }

private:
    const ParameterSpec& spec;
    juce::NormalisableRange<float> range;
    std::atomic<float> plain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FoundationFloatParameter)
};

/** Builds the complete host parameter layout, with groups following the registry's group paths. */
[[nodiscard]] juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

/** Returns the FOUNDATION float parameter with the given ID, or nullptr. */
[[nodiscard]] FoundationFloatParameter* findFloatParameter(juce::AudioProcessorValueTreeState& state,
                                                           std::string_view id);

/** Returns the choice parameter with the given ID, or nullptr. */
[[nodiscard]] juce::AudioParameterChoice* findChoiceParameter(juce::AudioProcessorValueTreeState& state,
                                                              std::string_view id);

/** Converts a registry ID to a juce::String. */
[[nodiscard]] juce::String toJuceString(std::string_view text);

} // namespace foundation::params
