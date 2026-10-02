#include "FoundationProcessor.h"

#include "ParameterLayout.h"
#include "RealtimeGuard.h"
#include "StateSerializer.h"

namespace foundation
{

namespace
{
const juce::Identifier parameterTreeType { "FOUNDATION_PARAMS" };
const juce::String productName { "FOUNDATION" };

#ifdef FOUNDATION_PRODUCT_VERSION
const juce::String productVersion { FOUNDATION_PRODUCT_VERSION };
#else
const juce::String productVersion { "0.0.0-dev" };
#endif
} // namespace

FoundationProcessor::FoundationProcessor()
    : juce::AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, parameterTreeType, params::createParameterLayout())
{
}

FoundationProcessor::~FoundationProcessor() = default;

const juce::String FoundationProcessor::getName() const
{
    return productName;
}

//==================================================================================================
// Audio
//==================================================================================================

void FoundationProcessor::prepareToPlay(double, int)
{
    // Phase 0: nothing to prepare. All future DSP allocation happens here, never in processBlock.
}

void FoundationProcessor::releaseResources()
{
}

bool FoundationProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (! layouts.inputBuses.isEmpty())
        return false;

    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::stereo() || output == juce::AudioChannelSet::mono();
}

void FoundationProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    const rt::ScopedRealtimeScope realtimeScope;
    const juce::ScopedNoDenormals noDenormals;

    // Phase 0 shell: MIDI is accepted and consumed; output is silence.
    buffer.clear();
    midiMessages.clear();
}

//==================================================================================================
// State
//==================================================================================================

void FoundationProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    const juce::ScopedLock lock(stateLock);
    const auto state = state::StateSerializer::capture(parameters, retainedUnknownState, productVersion);
    destData = state::StateSerializer::toBinary(state);
}

void FoundationProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return;

    const auto tree = state::StateSerializer::fromBinary(data, static_cast<size_t>(sizeInBytes));
    if (! tree.isValid())
    {
        DBG("FOUNDATION: ignoring unreadable state data");
        return;
    }

    const juce::ScopedLock lock(stateLock);
    auto result = state::StateSerializer::apply(tree, parameters);

    if (! result.ok)
    {
        DBG("FOUNDATION: state rejected — " << result.error);
        return;
    }

    retainedUnknownState = result.retainedUnknown;
}

} // namespace foundation

//==================================================================================================
// Plugin entry point — compiled only into the plugin's shared-code target.
//==================================================================================================
#if FOUNDATION_PLUGIN_ENTRY
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new foundation::FoundationProcessor();
}
#endif
