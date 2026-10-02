#pragma once

#include <span>
#include <string_view>

// =================================================================================================
// FOUNDATION — ParameterRegistry (ADR-0003)
//
// The single source of truth for every parameter. The host parameter layout (ParameterLayout), the
// state format (StateSerializer), the ID-stability golden test and — in later phases — modulation
// destinations, UI bindings, randomisation rules and generated documentation all derive from this
// table, so engine, host, presets and UI can never disagree about a parameter.
//
// Values are expressed in plain (user-facing) units. Choice parameters use their integer index.
// =================================================================================================

namespace foundation::params
{

enum class ParameterKind
{
    continuous,
    choice
};

/** How plain values map onto the host's normalised 0..1 range. Extended (e.g. exact logarithmic
    mappings for frequency/time) as parameters that need them are introduced. */
enum class ValueMapping
{
    linear
};

/** How values are displayed to hosts and users. */
enum class ValueFormat
{
    plain,
    percent,   // 0..1 shown as 0..100 %
    decibels,  // dB; the minimum of the range displays as "-inf dB"
    choice
};

enum class ParameterScope
{
    global,
    voice
};

struct ParameterSpec
{
    std::string_view id;
    int versionHint = 1;        // JUCE ParameterID version hint (AU parameter ordering)
    int sinceSchemaVersion = 1; // state schema version that introduced this parameter
    std::string_view name;
    std::string_view groupPath; // '/'-separated host group path, e.g. "Performance/Macros"
    ParameterKind kind = ParameterKind::continuous;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float defaultValue = 0.0f;
    ValueMapping mapping = ValueMapping::linear;
    ValueFormat format = ValueFormat::plain;
    std::string_view unit;
    std::span<const std::string_view> choices;
    bool automatable = true;
    bool modulatable = false;
    bool bipolar = false;
    ParameterScope scope = ParameterScope::global;
};

/** Every registered parameter, in host order. */
[[nodiscard]] std::span<const ParameterSpec> allParameters() noexcept;

/** Looks up a parameter by ID; returns nullptr if the ID is not registered. */
[[nodiscard]] const ParameterSpec* findParameter(std::string_view id) noexcept;

} // namespace foundation::params
