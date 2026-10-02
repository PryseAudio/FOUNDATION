#include "ParameterLayout.h"

#include <memory>
#include <vector>

namespace foundation::params
{

juce::String toJuceString(std::string_view text)
{
    return juce::String::fromUTF8(text.data(), static_cast<int>(text.size()));
}

//==================================================================================================
// FoundationFloatParameter
//==================================================================================================

FoundationFloatParameter::FoundationFloatParameter(const ParameterSpec& specToUse)
    : juce::RangedAudioParameter(juce::ParameterID { toJuceString(specToUse.id), specToUse.versionHint },
                                 toJuceString(specToUse.name),
                                 juce::AudioProcessorParameterWithIDAttributes()
                                     .withLabel(toJuceString(specToUse.unit))
                                     .withAutomatable(specToUse.automatable))
    , spec(specToUse)
    , range(specToUse.minValue, specToUse.maxValue)
    , plain(specToUse.defaultValue)
{
    jassert(spec.kind == ParameterKind::continuous);
    jassert(spec.mapping == ValueMapping::linear);
}

void FoundationFloatParameter::setPlainValueExact(float newPlainValue)
{
    const auto clamped = juce::jlimit(spec.minValue, spec.maxValue, newPlainValue);

    if (clamped == plain.load(std::memory_order_relaxed))
        return;

    plain.store(clamped, std::memory_order_relaxed);
    sendValueChangedMessageToListeners(getValue());
}

float FoundationFloatParameter::getValue() const
{
    return range.convertTo0to1(plain.load(std::memory_order_relaxed));
}

void FoundationFloatParameter::setValue(float newNormalisedValue)
{
    const auto normalised = juce::jlimit(0.0f, 1.0f, newNormalisedValue);

    // Echo guard: a value equal to what we already report must not perturb an exact plain value.
    if (normalised == getValue())
        return;

    plain.store(range.convertFrom0to1(normalised), std::memory_order_relaxed);
}

float FoundationFloatParameter::getDefaultValue() const
{
    return range.convertTo0to1(spec.defaultValue);
}

juce::String FoundationFloatParameter::getName(int maximumStringLength) const
{
    return toJuceString(spec.name).substring(0, maximumStringLength);
}

juce::String FoundationFloatParameter::getLabel() const
{
    return toJuceString(spec.unit);
}

juce::String FoundationFloatParameter::getText(float normalisedValue, int maximumStringLength) const
{
    const auto value = range.convertFrom0to1(juce::jlimit(0.0f, 1.0f, normalisedValue));
    juce::String text;

    switch (spec.format)
    {
        case ValueFormat::percent:
            text = juce::String(value * 100.0f, 1);
            break;
        case ValueFormat::decibels:
            text = value <= spec.minValue ? juce::String("-inf") : juce::String(value, 1);
            break;
        case ValueFormat::plain:
        case ValueFormat::choice:
            text = juce::String(value, 3);
            break;
    }

    return maximumStringLength > 0 ? text.substring(0, maximumStringLength) : text;
}

float FoundationFloatParameter::getValueForText(const juce::String& text) const
{
    const auto trimmed = text.trim();
    float value = spec.defaultValue;

    switch (spec.format)
    {
        case ValueFormat::percent:
            value = trimmed.getFloatValue() / 100.0f;
            break;
        case ValueFormat::decibels:
            value = trimmed.startsWithIgnoreCase("-inf") ? spec.minValue : trimmed.getFloatValue();
            break;
        case ValueFormat::plain:
        case ValueFormat::choice:
            value = trimmed.getFloatValue();
            break;
    }

    return range.convertTo0to1(juce::jlimit(spec.minValue, spec.maxValue, value));
}

//==================================================================================================
// Layout
//==================================================================================================

namespace
{

std::unique_ptr<juce::RangedAudioParameter> createParameter(const ParameterSpec& spec)
{
    if (spec.kind == ParameterKind::choice)
    {
        juce::StringArray choices;
        for (auto choice : spec.choices)
            choices.add(toJuceString(choice));

        return std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { toJuceString(spec.id), spec.versionHint },
            toJuceString(spec.name),
            choices,
            static_cast<int>(spec.defaultValue),
            juce::AudioParameterChoiceAttributes().withAutomatable(spec.automatable));
    }

    return std::make_unique<FoundationFloatParameter>(spec);
}

// Group tree built in registry order, then converted to juce parameter groups.
struct GroupNode
{
    juce::String segment;
    juce::String path;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    std::vector<std::unique_ptr<GroupNode>> children;

    GroupNode& child(const juce::String& childSegment)
    {
        for (auto& existing : children)
            if (existing->segment == childSegment)
                return *existing;

        auto node = std::make_unique<GroupNode>();
        node->segment = childSegment;
        node->path = path.isEmpty() ? childSegment : path + "/" + childSegment;
        children.push_back(std::move(node));
        return *children.back();
    }
};

juce::String groupIdForPath(const juce::String& path)
{
    // "Performance/Macros" -> "performance.macros"
    return path.toLowerCase().replaceCharacter('/', '.').removeCharacters(" ");
}

std::unique_ptr<juce::AudioProcessorParameterGroup> toJuceGroup(GroupNode& node)
{
    auto group = std::make_unique<juce::AudioProcessorParameterGroup>(groupIdForPath(node.path), node.segment, "|");

    for (auto& parameter : node.parameters)
        group->addChild(std::move(parameter));

    for (auto& childNode : node.children)
        group->addChild(toJuceGroup(*childNode));

    return group;
}

} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    GroupNode root;

    for (const auto& spec : allParameters())
    {
        auto* node = &root;
        for (const auto& segment : juce::StringArray::fromTokens(toJuceString(spec.groupPath), "/", ""))
            if (segment.isNotEmpty())
                node = &node->child(segment);

        node->parameters.push_back(createParameter(spec));
    }

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (auto& parameter : root.parameters)
        layout.add(std::move(parameter));

    for (auto& childNode : root.children)
        layout.add(toJuceGroup(*childNode));

    return layout;
}

FoundationFloatParameter* findFloatParameter(juce::AudioProcessorValueTreeState& state, std::string_view id)
{
    return dynamic_cast<FoundationFloatParameter*>(state.getParameter(toJuceString(id)));
}

juce::AudioParameterChoice* findChoiceParameter(juce::AudioProcessorValueTreeState& state, std::string_view id)
{
    return dynamic_cast<juce::AudioParameterChoice*>(state.getParameter(toJuceString(id)));
}

} // namespace foundation::params
