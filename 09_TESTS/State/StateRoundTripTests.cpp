#include "FoundationProcessor.h"
#include "ParameterIDs.h"
#include "ParameterLayout.h"
#include "ParameterRegistry.h"
#include "StateSerializer.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <vector>

using namespace foundation;
using namespace foundation::params;
using foundation::state::StateSerializer;
namespace tags = foundation::state::tags;

namespace
{

juce::MemoryBlock saveState(FoundationProcessor& processor)
{
    juce::MemoryBlock block;
    processor.getStateInformation(block);
    return block;
}

void loadState(FoundationProcessor& processor, const juce::MemoryBlock& block)
{
    processor.setStateInformation(block.getData(), static_cast<int>(block.getSize()));
}

bool sameBits(float a, float b)
{
    return std::memcmp(&a, &b, sizeof(float)) == 0;
}

/** Plain value of every registered parameter, in registry order. */
std::vector<float> plainValues(FoundationProcessor& processor)
{
    std::vector<float> values;
    auto& state = processor.getParameterState();

    for (const auto& spec : allParameters())
    {
        if (spec.kind == ParameterKind::choice)
            values.push_back(static_cast<float>(findChoiceParameter(state, spec.id)->getIndex()));
        else
            values.push_back(findFloatParameter(state, spec.id)->getPlainValue());
    }

    return values;
}

void requireBitIdentical(const std::vector<float>& a, const std::vector<float>& b)
{
    REQUIRE(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i)
    {
        INFO("parameter " << std::string { allParameters()[i].id } << ": " << a[i] << " vs " << b[i]);
        CHECK(sameBits(a[i], b[i]));
    }
}

/** Sets every parameter to a deterministic, deliberately awkward non-default value. */
void scramble(FoundationProcessor& processor, std::uint32_t seed)
{
    std::mt19937 rng { seed };
    auto& state = processor.getParameterState();

    for (const auto& spec : allParameters())
    {
        if (spec.kind == ParameterKind::choice)
        {
            auto* choice = findChoiceParameter(state, spec.id);
            const auto index = static_cast<int>(rng() % static_cast<std::uint32_t>(choice->choices.size()));
            *choice = index;
        }
        else
        {
            std::uniform_real_distribution<float> dist { spec.minValue, spec.maxValue };
            findFloatParameter(state, spec.id)->setPlainValueExact(dist(rng));
        }
    }
}

juce::MemoryBlock binaryFromTree(const juce::ValueTree& tree)
{
    return StateSerializer::toBinary(tree);
}

} // namespace

TEST_CASE("State: default state round-trips byte-identically", "[state]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;
    FoundationProcessor source;
    const auto saved = saveState(source);

    FoundationProcessor target;
    loadState(target, saved);
    const auto resaved = saveState(target);

    CHECK(saved == resaved);
    requireBitIdentical(plainValues(source), plainValues(target));
}

TEST_CASE("State: arbitrary values recall bit-exactly", "[state][exact]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;

    for (std::uint32_t seed = 1; seed <= 25; ++seed)
    {
        INFO("seed " << seed);
        FoundationProcessor source;
        scramble(source, seed);
        const auto saved = saveState(source);

        FoundationProcessor target;
        loadState(target, saved);

        requireBitIdentical(plainValues(source), plainValues(target));
        CHECK(saveState(target) == saved);
    }
}

TEST_CASE("State: values set by host automation recall bit-exactly", "[state][exact]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;
    std::mt19937 rng { 1234 };
    std::uniform_real_distribution<float> normalised { 0.0f, 1.0f };

    FoundationProcessor source;
    for (auto* parameter : source.getParameters())
        parameter->setValueNotifyingHost(normalised(rng));

    const auto saved = saveState(source);
    FoundationProcessor target;
    loadState(target, saved);

    requireBitIdentical(plainValues(source), plainValues(target));

    // A host echoing current values back after recall must not change anything.
    for (auto* parameter : target.getParameters())
        parameter->setValue(parameter->getValue());

    requireBitIdentical(plainValues(source), plainValues(target));
}

TEST_CASE("State: parameters missing from a state are reset to their defaults", "[state][migration]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;
    FoundationProcessor processor;
    scramble(processor, 99);

    juce::ValueTree root(tags::root);
    root.setProperty(tags::schemaVersion, 1, nullptr);
    juce::ValueTree paramsTree(tags::params);
    juce::ValueTree entry(tags::param);
    entry.setProperty(tags::id, toJuceString(ids::outputGain), nullptr);
    entry.setProperty(tags::value, "-12.5", nullptr);
    paramsTree.appendChild(entry, nullptr);
    root.appendChild(paramsTree, nullptr);

    const auto result = StateSerializer::apply(root, processor.getParameterState());
    REQUIRE(result.ok);
    CHECK(result.resetToDefault.size() == static_cast<int>(allParameters().size()) - 1);

    const auto values = plainValues(processor);
    const auto registry = allParameters();

    for (size_t i = 0; i < registry.size(); ++i)
    {
        INFO("parameter " << std::string { registry[i].id });
        if (registry[i].id == ids::outputGain)
            CHECK(values[i] == -12.5f);
        else
            CHECK(values[i] == registry[i].defaultValue);
    }
}

TEST_CASE("State: entries for unknown parameter IDs are preserved and written back", "[state][forward]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;
    FoundationProcessor source;
    auto tree = StateSerializer::capture(source.getParameterState(), {}, "test");

    juce::ValueTree future(tags::param);
    future.setProperty(tags::id, "future.parameter", nullptr);
    future.setProperty(tags::value, "0.75", nullptr);
    tree.getChildWithName(tags::params).appendChild(future, nullptr);

    FoundationProcessor target;
    loadState(target, binaryFromTree(tree));

    const auto resaved = saveState(target);
    const auto resavedTree = StateSerializer::fromBinary(resaved.getData(), resaved.getSize());
    REQUIRE(resavedTree.isValid());

    const auto retained = resavedTree.getChildWithName(tags::retainedUnknown);
    REQUIRE(retained.isValid());
    const auto kept = retained.getChildWithProperty(tags::id, "future.parameter");
    REQUIRE(kept.isValid());
    CHECK(kept.getProperty(tags::value).toString() == "0.75");
}

TEST_CASE("State: invalid or newer states are rejected without changing anything", "[state][robustness]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;
    FoundationProcessor processor;
    scramble(processor, 7);
    const auto before = plainValues(processor);

    SECTION("newer schema")
    {
        auto tree = StateSerializer::capture(processor.getParameterState(), {}, "test");
        tree.setProperty(tags::schemaVersion, state::currentSchemaVersion + 1, nullptr);
        tree.getChildWithName(tags::params).getChild(0).setProperty(tags::value, "0.123", nullptr);

        const auto result = StateSerializer::apply(tree, processor.getParameterState());
        CHECK_FALSE(result.ok);
        loadState(processor, binaryFromTree(tree));
    }

    SECTION("malformed value")
    {
        FoundationProcessor other;
        auto tree = StateSerializer::capture(other.getParameterState(), {}, "test");
        tree.getChildWithName(tags::params).getChild(3).setProperty(tags::value, "not-a-number", nullptr);

        const auto result = StateSerializer::apply(tree, processor.getParameterState());
        CHECK_FALSE(result.ok);
        loadState(processor, binaryFromTree(tree));
    }

    SECTION("missing schema version")
    {
        FoundationProcessor other;
        auto tree = StateSerializer::capture(other.getParameterState(), {}, "test");
        tree.removeProperty(tags::schemaVersion, nullptr);
        CHECK_FALSE(StateSerializer::apply(tree, processor.getParameterState()).ok);
    }

    SECTION("garbage bytes")
    {
        const char garbage[] = "this is not a FOUNDATION state at all";
        processor.setStateInformation(garbage, static_cast<int>(sizeof(garbage)));
        processor.setStateInformation(nullptr, 0);
    }

    requireBitIdentical(before, plainValues(processor));
}

TEST_CASE("State: float text encoding round-trips exactly", "[state][exact]")
{
    std::mt19937 rng { 42 };
    int tested = 0;

    while (tested < 100000)
    {
        const auto bits = static_cast<std::uint32_t>(rng());
        float value = 0.0f;
        std::memcpy(&value, &bits, sizeof(value));

        if (! std::isfinite(value))
            continue;

        const auto parsed = StateSerializer::parseFloat(StateSerializer::formatFloat(value));
        REQUIRE(parsed.has_value());
        REQUIRE(sameBits(*parsed, value));
        ++tested;
    }

    CHECK_FALSE(StateSerializer::parseFloat("").has_value());
    CHECK_FALSE(StateSerializer::parseFloat("1.5x").has_value());
    CHECK_FALSE(StateSerializer::parseFloat("inf").has_value());
    CHECK_FALSE(StateSerializer::parseFloat("nan").has_value());
}
