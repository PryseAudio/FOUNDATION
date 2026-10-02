#include "ParameterLayout.h"
#include "ParameterRegistry.h"

#include <catch2/catch_test_macros.hpp>

#include <juce_core/juce_core.h>

// =================================================================================================
// Parameter ID stability (ADR-0003)
//
// Fails when any pinned parameter disappears, moves, or changes its definition — and when a new
// parameter is registered without being deliberately added to the golden file. Hosts store
// automation and sessions against these definitions, so every change here must be intentional.
// =================================================================================================

using namespace foundation::params;

namespace
{

juce::var loadGolden()
{
    const juce::File golden { juce::String(FOUNDATION_GOLDEN_DIR) + "/parameter_ids.golden.json" };
    REQUIRE(golden.existsAsFile());

    const auto parsed = juce::JSON::parse(golden);
    REQUIRE(parsed.isObject());
    return parsed;
}

juce::String kindName(ParameterKind kind)
{
    return kind == ParameterKind::choice ? "choice" : "continuous";
}

juce::String mappingName(ValueMapping mapping)
{
    switch (mapping)
    {
        case ValueMapping::linear:
            return "linear";
    }
    return "unknown";
}

} // namespace

TEST_CASE("Parameter IDs: every registered parameter matches the golden contract, in order", "[params][golden]")
{
    const auto golden = loadGolden();
    const auto* entries = golden["parameters"].getArray();
    REQUIRE(entries != nullptr);

    const auto registry = allParameters();

    INFO("If this change is intentional and pre-release, update 09_TESTS/Golden/parameter_ids.golden.json");
    REQUIRE(static_cast<size_t>(entries->size()) == registry.size());

    for (int i = 0; i < entries->size(); ++i)
    {
        const auto& entry = entries->getReference(i);
        const auto& spec = registry[static_cast<size_t>(i)];
        const auto goldenId = entry["id"].toString();

        INFO("golden index " << i << ", id " << goldenId);
        CHECK(toJuceString(spec.id) == goldenId);
        CHECK(spec.versionHint == static_cast<int>(entry["versionHint"]));
        CHECK(kindName(spec.kind) == entry["kind"].toString());
        CHECK(mappingName(spec.mapping) == entry["mapping"].toString());
        CHECK(spec.minValue == static_cast<float>(static_cast<double>(entry["min"])));
        CHECK(spec.maxValue == static_cast<float>(static_cast<double>(entry["max"])));
        CHECK(spec.defaultValue == static_cast<float>(static_cast<double>(entry["default"])));
        CHECK(spec.automatable == static_cast<bool>(entry["automatable"]));

        const auto* choices = entry["choices"].getArray();
        REQUIRE(choices != nullptr);
        REQUIRE(static_cast<size_t>(choices->size()) == spec.choices.size());

        for (int c = 0; c < choices->size(); ++c)
            CHECK(toJuceString(spec.choices[static_cast<size_t>(c)]) == choices->getReference(c).toString());
    }
}
