// =================================================================================================
// FOUNDATION — RenderCLI
//
// Renders the FOUNDATION processor offline to a 32-bit float WAV file.
//
//   foundation_render_cli --out <file.wav> [--seconds <s>] [--sample-rate <hz>] [--block <samples>]
//                         [--midi <file.mid>] [--state <file>] [--note <midi note>]
//
//   --midi   plays all tracks of a standard MIDI file (tempo map honoured)
//   --state  loads binary state as written by the plugin (getStateInformation)
//   --note   plays one note (velocity 100) from 0 s to 75 % of the render length
//
// After writing, the file is re-opened and its format/length verified.
// Exit codes: 0 ok, 1 usage error, 2 I/O error, 3 verification failure.
// =================================================================================================

#include "FoundationProcessor.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

namespace
{

struct Options
{
    juce::File out;
    juce::File midi;
    juce::File state;
    double seconds = 2.0;
    double sampleRate = 48000.0;
    int blockSize = 512;
    int note = -1;
};

void printUsage()
{
    std::cout << "Usage: foundation_render_cli --out <file.wav> [--seconds <s>] [--sample-rate <hz>]\n"
                 "                             [--block <samples>] [--midi <file.mid>] [--state <file>]\n"
                 "                             [--note <midi note>]\n";
}

bool parseOptions(const juce::StringArray& args, Options& options)
{
    for (int i = 1; i < args.size(); ++i)
    {
        const auto& arg = args[i];
        const bool hasValue = i + 1 < args.size();
        const auto value = hasValue ? args[i + 1] : juce::String();

        auto file = [&] { return juce::File::getCurrentWorkingDirectory().getChildFile(value); };

        if (arg == "--out" && hasValue)                options.out = file();
        else if (arg == "--midi" && hasValue)          options.midi = file();
        else if (arg == "--state" && hasValue)         options.state = file();
        else if (arg == "--seconds" && hasValue)       options.seconds = value.getDoubleValue();
        else if (arg == "--sample-rate" && hasValue)   options.sampleRate = value.getDoubleValue();
        else if (arg == "--block" && hasValue)         options.blockSize = value.getIntValue();
        else if (arg == "--note" && hasValue)          options.note = value.getIntValue();
        else
        {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            return false;
        }

        ++i;
    }

    if (options.out == juce::File())
    {
        std::cerr << "--out is required\n";
        return false;
    }

    if (options.seconds <= 0.0 || options.seconds > 3600.0 || options.sampleRate < 8000.0
        || options.sampleRate > 768000.0 || options.blockSize < 1 || options.blockSize > 65536
        || options.note > 127)
    {
        std::cerr << "Argument out of range\n";
        return false;
    }

    return true;
}

bool buildEvents(const Options& options, juce::MidiMessageSequence& events)
{
    if (options.midi != juce::File())
    {
        juce::FileInputStream stream(options.midi);
        juce::MidiFile midiFile;

        if (! stream.openedOk() || ! midiFile.readFrom(stream))
        {
            std::cerr << "Could not read MIDI file: " << options.midi.getFullPathName() << "\n";
            return false;
        }

        midiFile.convertTimestampTicksToSeconds();
        for (int track = 0; track < midiFile.getNumTracks(); ++track)
            events.addSequence(*midiFile.getTrack(track), 0.0);
    }

    if (options.note >= 0)
    {
        events.addEvent(juce::MidiMessage::noteOn(1, options.note, static_cast<juce::uint8>(100)), 0.0);
        events.addEvent(juce::MidiMessage::noteOff(1, options.note), options.seconds * 0.75);
    }

    events.sort();
    events.updateMatchedPairs();
    return true;
}

} // namespace

int main(int argc, char* argv[])
{
    juce::StringArray args;
    for (int i = 0; i < argc; ++i)
        args.add(juce::String::fromUTF8(argv[i]));

    Options options;
    if (! parseOptions(args, options))
    {
        printUsage();
        return 1;
    }

    const juce::ScopedJuceInitialiser_GUI juceInit;

    juce::MidiMessageSequence events;
    if (! buildEvents(options, events))
        return 2;

    foundation::FoundationProcessor processor;
    processor.setPlayConfigDetails(0, 2, options.sampleRate, options.blockSize);

    if (options.state != juce::File())
    {
        juce::MemoryBlock stateData;
        if (! options.state.loadFileAsData(stateData))
        {
            std::cerr << "Could not read state file: " << options.state.getFullPathName() << "\n";
            return 2;
        }
        processor.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
    }

    processor.prepareToPlay(options.sampleRate, options.blockSize);

    const auto totalSamples = static_cast<int>(std::llround(options.seconds * options.sampleRate));
    juce::AudioBuffer<float> rendered(2, totalSamples);
    juce::AudioBuffer<float> block(2, options.blockSize);
    juce::MidiBuffer midi;

    int nextEvent = 0;
    for (int start = 0; start < totalSamples; start += options.blockSize)
    {
        const int numSamples = std::min(options.blockSize, totalSamples - start);
        block.setSize(2, numSamples, false, false, true);
        midi.clear();

        while (nextEvent < events.getNumEvents())
        {
            const auto& message = events.getEventPointer(nextEvent)->message;
            const auto samplePosition = static_cast<int>(std::llround(message.getTimeStamp() * options.sampleRate));

            if (samplePosition >= start + numSamples)
                break;

            midi.addEvent(message, std::max(0, samplePosition - start));
            ++nextEvent;
        }

        processor.processBlock(block, midi);

        for (int channel = 0; channel < 2; ++channel)
            rendered.copyFrom(channel, start, block, channel, 0, numSamples);
    }

    processor.releaseResources();

    // Write.
    {
        options.out.getParentDirectory().createDirectory();
        options.out.deleteFile();

        std::unique_ptr<juce::OutputStream> stream = std::make_unique<juce::FileOutputStream>(options.out);
        if (! static_cast<juce::FileOutputStream*>(stream.get())->openedOk())
        {
            std::cerr << "Could not open output file: " << options.out.getFullPathName() << "\n";
            return 2;
        }

        juce::WavAudioFormat wav;
        const auto writerOptions = juce::AudioFormatWriterOptions()
                                       .withSampleRate(options.sampleRate)
                                       .withNumChannels(2)
                                       .withBitsPerSample(32)
                                       .withSampleFormat(juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);

        auto writer = wav.createWriterFor(stream, writerOptions);
        if (writer == nullptr || ! writer->writeFromAudioSampleBuffer(rendered, 0, totalSamples))
        {
            std::cerr << "Could not write WAV data\n";
            return 2;
        }
    }

    // Verify.
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(options.out));

    if (reader == nullptr || reader->numChannels != 2 || reader->sampleRate != options.sampleRate
        || reader->lengthInSamples != totalSamples || reader->bitsPerSample != 32 || ! reader->usesFloatingPointData)
    {
        std::cerr << "Verification failed for " << options.out.getFullPathName() << "\n";
        return 3;
    }

    const auto peak = rendered.getMagnitude(0, totalSamples);
    std::cout << "Rendered " << totalSamples << " samples (" << options.seconds << " s) at " << options.sampleRate
              << " Hz, block " << options.blockSize << ", " << events.getNumEvents() << " MIDI events\n"
              << "Peak: " << juce::Decibels::toString(juce::Decibels::gainToDecibels(peak)) << "\n"
              << "Wrote and verified: " << options.out.getFullPathName() << "\n";

    return 0;
}
