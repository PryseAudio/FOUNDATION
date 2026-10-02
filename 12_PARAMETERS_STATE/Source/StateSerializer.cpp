#include "StateSerializer.h"

#include "ParameterLayout.h"
#include "ParameterRegistry.h"

#include <charconv>
#include <cmath>
#include <vector>

namespace foundation::state
{

using params::ParameterKind;
using params::ParameterSpec;

//==================================================================================================
// Float formatting
//==================================================================================================

juce::String StateSerializer::formatFloat(float value)
{
    char buffer[64] {};
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    jassert(result.ec == std::errc());
    return juce::String::fromUTF8(buffer, static_cast<int>(result.ptr - buffer));
}

std::optional<float> StateSerializer::parseFloat(const juce::String& text)
{
    const auto utf8 = text.trim().toStdString();
    if (utf8.empty())
        return std::nullopt;

    float value = 0.0f;
    const auto* begin = utf8.data();
    const auto* end = utf8.data() + utf8.size();
    const auto result = std::from_chars(begin, end, value);

    if (result.ec != std::errc() || result.ptr != end || ! std::isfinite(value))
        return std::nullopt;

    return value;
}

//==================================================================================================
// Capture
//==================================================================================================

juce::ValueTree StateSerializer::capture(juce::AudioProcessorValueTreeState& parameters,
                                         const juce::ValueTree& retainedUnknown,
                                         const juce::String& productVersion)
{
    juce::ValueTree root(tags::root);
    root.setProperty(tags::schemaVersion, currentSchemaVersion, nullptr);
    root.setProperty(tags::engineVersion, currentEngineVersion, nullptr);
    root.setProperty(tags::productVersion, productVersion, nullptr);

    juce::ValueTree paramsTree(tags::params);

    for (const auto& spec : params::allParameters())
    {
        float plainValue = spec.defaultValue;

        if (spec.kind == ParameterKind::choice)
        {
            if (auto* choice = params::findChoiceParameter(parameters, spec.id))
                plainValue = static_cast<float>(choice->getIndex());
            else
                jassertfalse;
        }
        else
        {
            if (auto* floatParam = params::findFloatParameter(parameters, spec.id))
                plainValue = floatParam->getPlainValue();
            else
                jassertfalse;
        }

        juce::ValueTree entry(tags::param);
        entry.setProperty(tags::id, params::toJuceString(spec.id), nullptr);
        entry.setProperty(tags::value, formatFloat(plainValue), nullptr);
        paramsTree.appendChild(entry, nullptr);
    }

    root.appendChild(paramsTree, nullptr);

    if (retainedUnknown.isValid() && retainedUnknown.getNumChildren() > 0)
        root.appendChild(retainedUnknown.createCopy(), nullptr);

    return root;
}

//==================================================================================================
// Migration
//==================================================================================================

juce::ValueTree StateSerializer::migrate(const juce::ValueTree& state, juce::String& error)
{
    if (! state.isValid() || ! state.hasType(tags::root))
    {
        error = "Not a FOUNDATION state";
        return {};
    }

    if (! state.hasProperty(tags::schemaVersion))
    {
        error = "State has no schemaVersion";
        return {};
    }

    const int schema = static_cast<int>(state.getProperty(tags::schemaVersion));

    if (schema > currentSchemaVersion)
    {
        error = "State was saved by a newer FOUNDATION (schema " + juce::String(schema) + ", this build understands "
                + juce::String(currentSchemaVersion) + ")";
        return {};
    }

    if (schema < 1)
    {
        error = "Invalid schemaVersion " + juce::String(schema);
        return {};
    }

    auto migrated = state.createCopy();

    // Migration chain: one step per schema version, applied in order (v1 -> v2 -> ...).
    // Schema 1 is the first version; there is nothing to migrate yet.

    migrated.setProperty(tags::schemaVersion, currentSchemaVersion, nullptr);
    return migrated;
}

//==================================================================================================
// Apply
//==================================================================================================

ApplyResult StateSerializer::apply(const juce::ValueTree& state, juce::AudioProcessorValueTreeState& parameters)
{
    ApplyResult result;

    const auto migrated = migrate(state, result.error);
    if (! migrated.isValid())
        return result;

    const auto paramsTree = migrated.getChildWithName(tags::params);
    if (! paramsTree.isValid())
    {
        result.error = "State has no PARAMS";
        return result;
    }

    // Pass 1: validate and resolve every value. Nothing is changed unless the whole state is valid.
    struct Resolved
    {
        const ParameterSpec* spec;
        float plainValue;
    };

    std::vector<Resolved> resolved;
    resolved.reserve(params::allParameters().size());

    for (const auto& spec : params::allParameters())
    {
        const auto entry = paramsTree.getChildWithProperty(tags::id, params::toJuceString(spec.id));

        if (! entry.isValid())
        {
            resolved.push_back({ &spec, spec.defaultValue });
            result.resetToDefault.add(params::toJuceString(spec.id));
            continue;
        }

        const auto parsed = parseFloat(entry.getProperty(tags::value).toString());
        if (! parsed.has_value())
        {
            result.error = "Malformed value for parameter '" + params::toJuceString(spec.id) + "'";
            return result;
        }

        resolved.push_back({ &spec, *parsed });
    }

    juce::ValueTree unknown(tags::retainedUnknown);

    for (const auto& entry : paramsTree)
        if (params::findParameter(entry.getProperty(tags::id).toString().toStdString()) == nullptr)
            unknown.appendChild(entry.createCopy(), nullptr);

    if (const auto previouslyRetained = migrated.getChildWithName(tags::retainedUnknown); previouslyRetained.isValid())
        for (const auto& entry : previouslyRetained)
            if (! unknown.getChildWithProperty(tags::id, entry.getProperty(tags::id)).isValid())
                unknown.appendChild(entry.createCopy(), nullptr);

    // Pass 2: apply.
    for (const auto& [spec, plainValue] : resolved)
    {
        if (spec->kind == ParameterKind::choice)
        {
            if (auto* choice = params::findChoiceParameter(parameters, spec->id))
            {
                const auto index = juce::jlimit(0, choice->choices.size() - 1, juce::roundToInt(plainValue));
                choice->setValueNotifyingHost(choice->convertTo0to1(static_cast<float>(index)));
                ++result.appliedCount;
            }
        }
        else if (auto* floatParam = params::findFloatParameter(parameters, spec->id))
        {
            floatParam->setPlainValueExact(plainValue);
            ++result.appliedCount;
        }
    }

    result.retainedUnknown = unknown;
    result.ok = true;
    return result;
}

//==================================================================================================
// Binary session format
//==================================================================================================

juce::MemoryBlock StateSerializer::toBinary(const juce::ValueTree& state)
{
    juce::MemoryBlock block;
    {
        juce::MemoryOutputStream memory(block, false);
        juce::GZIPCompressorOutputStream gzip(memory);
        state.writeToStream(gzip);
    }
    return block;
}

juce::ValueTree StateSerializer::fromBinary(const void* data, size_t sizeInBytes)
{
    if (data == nullptr || sizeInBytes == 0)
        return {};

    auto tree = juce::ValueTree::readFromGZIPData(data, sizeInBytes);
    return tree.hasType(tags::root) ? tree : juce::ValueTree {};
}

} // namespace foundation::state
