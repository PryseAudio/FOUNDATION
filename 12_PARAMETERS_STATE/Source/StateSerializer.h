#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <optional>

// =================================================================================================
// FOUNDATION — state serialisation (ADR-0006)
//
// State tree (Phase 0 subset of the architecture §G schema):
//
//   FOUNDATION_STATE  schemaVersion · engineVersion · productVersion
//     PARAMS
//       P  id="<parameter id>"  v="<plain value>"
//     RETAINED_UNKNOWN            entries for IDs this build does not know (written by a newer
//       P  id=...  v=...          build) are preserved and written back unchanged
//
// Rules:
//   - Plain values are stored as the shortest decimal string that round-trips the float exactly
//     (std::to_chars), so recall is bit-exact.
//   - Every registered parameter missing from a loaded state is reset to its default (the
//     sound-neutral value), so a load never inherits leftovers from the previous sound.
//   - States are migrated forward through migrate() before being applied. A state with a newer
//     schemaVersion than this build understands is rejected and the current state is left untouched.
//   - DAW session state is the tree as GZIP-compressed binary ValueTree data. (Human-readable JSON
//     preset files arrive with the preset library, Phase 7.)
//
// Not for the audio thread: all functions here allocate.
// =================================================================================================

namespace foundation::state
{

inline constexpr int currentSchemaVersion = 1;
inline constexpr int currentEngineVersion = 1;

namespace tags
{
inline const juce::Identifier root { "FOUNDATION_STATE" };
inline const juce::Identifier params { "PARAMS" };
inline const juce::Identifier param { "P" };
inline const juce::Identifier retainedUnknown { "RETAINED_UNKNOWN" };
inline const juce::Identifier schemaVersion { "schemaVersion" };
inline const juce::Identifier engineVersion { "engineVersion" };
inline const juce::Identifier productVersion { "productVersion" };
inline const juce::Identifier id { "id" };
inline const juce::Identifier value { "v" };
} // namespace tags

struct ApplyResult
{
    bool ok = false;
    juce::String error;
    int appliedCount = 0;
    juce::StringArray resetToDefault; // registered IDs absent from the state
    juce::ValueTree retainedUnknown;  // entries for IDs this build does not know
};

class StateSerializer
{
public:
    /** Captures the current parameter state. Safe to call from any non-audio thread. */
    [[nodiscard]] static juce::ValueTree capture(juce::AudioProcessorValueTreeState& parameters,
                                                 const juce::ValueTree& retainedUnknown,
                                                 const juce::String& productVersion);

    /** Migrates and applies a state tree. On failure nothing is changed. */
    [[nodiscard]] static ApplyResult apply(const juce::ValueTree& state,
                                           juce::AudioProcessorValueTreeState& parameters);

    /** Migrates a state tree to currentSchemaVersion. Returns an invalid tree and sets error on failure. */
    [[nodiscard]] static juce::ValueTree migrate(const juce::ValueTree& state, juce::String& error);

    [[nodiscard]] static juce::MemoryBlock toBinary(const juce::ValueTree& state);
    [[nodiscard]] static juce::ValueTree fromBinary(const void* data, size_t sizeInBytes);

    /** Shortest decimal representation that parses back to exactly the same float. */
    [[nodiscard]] static juce::String formatFloat(float value);
    [[nodiscard]] static std::optional<float> parseFloat(const juce::String& text);
};

} // namespace foundation::state
