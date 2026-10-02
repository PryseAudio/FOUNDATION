#include "FoundationProcessor.h"
#include "ParameterIDs.h"
#include "ParameterLayout.h"
#include "ParameterRegistry.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <regex>
#include <set>
#include <string>

using namespace foundation;
using namespace foundation::params;

TEST_CASE("Registry: parameter IDs are unique and well-formed", "[params][registry]")
{
    const std::regex idFormat { R"(^[a-z][a-zA-Z0-9]*(\.[a-z][a-zA-Z0-9]*)+$)" };
    std::set<std::string> seen;

    for (const auto& spec : allParameters())
    {
        const std::string id { spec.id };
        INFO("id: " << id);
        CHECK(std::regex_match(id, idFormat));
        CHECK(seen.insert(id).second);
        CHECK_FALSE(spec.name.empty());
        CHECK_FALSE(spec.groupPath.empty());
        CHECK(spec.versionHint >= 1);
        CHECK(spec.sinceSchemaVersion >= 1);
    }
}

TEST_CASE("Registry: ranges and defaults are valid", "[params][registry]")
{
    for (const auto& spec : allParameters())
    {
        INFO("id: " << std::string { spec.id });
        CHECK(std::isfinite(spec.minValue));
        CHECK(std::isfinite(spec.maxValue));
        CHECK(spec.minValue < spec.maxValue);
        CHECK(spec.defaultValue >= spec.minValue);
        CHECK(spec.defaultValue <= spec.maxValue);

        if (spec.kind == ParameterKind::choice)
        {
            REQUIRE(spec.choices.size() >= 2);
            CHECK(spec.minValue == 0.0f);
            CHECK(spec.maxValue == static_cast<float>(spec.choices.size() - 1));
            CHECK(spec.defaultValue == std::floor(spec.defaultValue));
        }
    }
}

TEST_CASE("Registry: lookup", "[params][registry]")
{
    for (const auto& spec : allParameters())
        CHECK(findParameter(spec.id) == &spec);

    CHECK(findParameter("does.not.exist") == nullptr);
}

TEST_CASE("Registry: eight macros; PLAY knobs are macros 1-6", "[params][registry][macros]")
{
    STATIC_REQUIRE(ids::numMacros == 8);

    const char* playNames[] = { "SUB", "BODY", "TEXTURE", "MOVEMENT", "WIDTH", "OUTPUT" };

    for (int i = 0; i < ids::numMacros; ++i)
    {
        const auto* spec = findParameter(ids::macros[static_cast<size_t>(i)]);
        REQUIRE(spec != nullptr);
        CHECK(spec->kind == ParameterKind::continuous);
        CHECK(spec->minValue == 0.0f);
        CHECK(spec->maxValue == 1.0f);
        CHECK(spec->modulatable);

        if (i < 6)
            CHECK(std::string { spec->name }.find(playNames[i]) != std::string::npos);
    }
}

TEST_CASE("Registry: PLAY mode groups default to CLEAN / STEREO / SMOOTH", "[params][registry][modes]")
{
    const auto* character = findParameter(ids::perfCharacter);
    const auto* stereoMode = findParameter(ids::perfStereoMode);
    const auto* response = findParameter(ids::perfResponse);

    REQUIRE(character != nullptr);
    REQUIRE(stereoMode != nullptr);
    REQUIRE(response != nullptr);

    CHECK(character->choices.size() == 3);
    CHECK(stereoMode->choices.size() == 2);
    CHECK(response->choices.size() == 2);

    CHECK(static_cast<int>(character->defaultValue) == static_cast<int>(Character::clean));
    CHECK(static_cast<int>(stereoMode->defaultValue) == static_cast<int>(StereoMode::stereo));
    CHECK(static_cast<int>(response->defaultValue) == static_cast<int>(Response::smooth));
}

TEST_CASE("Layout: the processor exposes exactly the registry, in registry order", "[params][layout]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;
    FoundationProcessor processor;

    const auto& hostParameters = processor.getParameters();
    const auto registry = allParameters();
    REQUIRE(static_cast<size_t>(hostParameters.size()) == registry.size());

    for (size_t i = 0; i < registry.size(); ++i)
    {
        auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*>(hostParameters[static_cast<int>(i)]);
        REQUIRE(withId != nullptr);
        CHECK(withId->paramID == toJuceString(registry[i].id));
        CHECK(withId->isAutomatable() == registry[i].automatable);

        const auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(withId);
        REQUIRE(ranged != nullptr);
        CHECK(ranged->convertFrom0to1(ranged->getDefaultValue()) == registry[i].defaultValue);
    }
}

TEST_CASE("FoundationFloatParameter: exact plain values survive host echo", "[params][exact]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;
    FoundationProcessor processor;

    auto* gain = findFloatParameter(processor.getParameterState(), ids::outputGain);
    REQUIRE(gain != nullptr);

    // A value that does not survive a plain -> normalised -> plain round trip in general.
    const float awkward = std::nextafter(-17.3f, 0.0f);
    gain->setPlainValueExact(awkward);
    CHECK(gain->getPlainValue() == awkward);

    // A host/wrapper echoing back the value we report must not perturb it.
    gain->setValue(gain->getValue());
    CHECK(gain->getPlainValue() == awkward);

    // A genuinely different host value is applied.
    gain->setValueNotifyingHost(1.0f);
    CHECK(gain->getPlainValue() == 6.0f);

    // Clamping.
    gain->setPlainValueExact(100.0f);
    CHECK(gain->getPlainValue() == 6.0f);
}

TEST_CASE("FoundationFloatParameter: text conversion", "[params][text]")
{
    const juce::ScopedJuceInitialiser_GUI juceInit;
    FoundationProcessor processor;
    auto& state = processor.getParameterState();

    auto* gain = findFloatParameter(state, ids::outputGain);
    REQUIRE(gain != nullptr);
    CHECK(gain->getText(0.0f, 0) == "-inf");
    CHECK(gain->getLabel() == "dB");
    CHECK(gain->getValueForText("-inf") == 0.0f);
    CHECK(gain->convertFrom0to1(gain->getValueForText("0")) == 0.0f);

    auto* macro = findFloatParameter(state, ids::macro1);
    REQUIRE(macro != nullptr);
    CHECK(macro->getText(0.5f, 0) == "50.0");
    CHECK(macro->getLabel() == "%");
    CHECK(macro->getValueForText("25") == 0.25f);
}
