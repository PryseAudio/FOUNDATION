#include "ParameterRegistry.h"

#include "ParameterIDs.h"

#include <array>

namespace foundation::params
{

namespace
{

constexpr std::array<std::string_view, 3> characterChoices { "Clean", "Warm", "Drive" };
constexpr std::array<std::string_view, 2> stereoModeChoices { "Mono", "Stereo" };
constexpr std::array<std::string_view, 2> responseChoices { "Smooth", "Aggressive" };

constexpr ParameterSpec makeMacro(std::string_view id, std::string_view name)
{
    ParameterSpec spec;
    spec.id = id;
    spec.name = name;
    spec.groupPath = "Performance/Macros";
    spec.kind = ParameterKind::continuous;
    spec.minValue = 0.0f;
    spec.maxValue = 1.0f;
    spec.defaultValue = 0.0f;
    spec.format = ValueFormat::percent;
    spec.unit = "%";
    spec.modulatable = true; // macros accept global, non-macro sources (architecture §F)
    return spec;
}

constexpr ParameterSpec makeChoice(std::string_view id,
                                   std::string_view name,
                                   std::span<const std::string_view> choices,
                                   int defaultIndex)
{
    ParameterSpec spec;
    spec.id = id;
    spec.name = name;
    spec.groupPath = "Performance/Modes";
    spec.kind = ParameterKind::choice;
    spec.minValue = 0.0f;
    spec.maxValue = static_cast<float>(choices.size() - 1);
    spec.defaultValue = static_cast<float>(defaultIndex);
    spec.format = ValueFormat::choice;
    spec.choices = choices;
    return spec;
}

constexpr ParameterSpec makeOutputGain()
{
    ParameterSpec spec;
    spec.id = ids::outputGain;
    spec.name = "Output Gain";
    spec.groupPath = "Output";
    spec.kind = ParameterKind::continuous;
    spec.minValue = -60.0f; // displayed as -inf dB
    spec.maxValue = 6.0f;
    spec.defaultValue = 0.0f;
    spec.mapping = ValueMapping::linear; // linear in dB
    spec.format = ValueFormat::decibels;
    spec.unit = "dB";
    spec.modulatable = true;
    return spec;
}

// Host order. Append new parameters at the end of their section; never reorder released entries.
constexpr std::array<ParameterSpec, 12> registry {
    makeMacro(ids::macro1, "Macro 1 SUB"),
    makeMacro(ids::macro2, "Macro 2 BODY"),
    makeMacro(ids::macro3, "Macro 3 TEXTURE"),
    makeMacro(ids::macro4, "Macro 4 MOVEMENT"),
    makeMacro(ids::macro5, "Macro 5 WIDTH"),
    makeMacro(ids::macro6, "Macro 6 OUTPUT"),
    makeMacro(ids::macro7, "Macro 7"),
    makeMacro(ids::macro8, "Macro 8"),
    makeChoice(ids::perfCharacter, "Character", characterChoices, static_cast<int>(Character::clean)),
    makeChoice(ids::perfStereoMode, "Stereo Mode", stereoModeChoices, static_cast<int>(StereoMode::stereo)),
    makeChoice(ids::perfResponse, "Response", responseChoices, static_cast<int>(Response::smooth)),
    makeOutputGain(),
};

} // namespace

std::span<const ParameterSpec> allParameters() noexcept
{
    return registry;
}

const ParameterSpec* findParameter(std::string_view id) noexcept
{
    for (const auto& spec : registry)
        if (spec.id == id)
            return &spec;

    return nullptr;
}

} // namespace foundation::params
