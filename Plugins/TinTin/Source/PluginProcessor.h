// Plugins/TinTin/Source/PluginProcessor.h
#pragma once

#include "Parameters.h"
#include <shared_plugin_helpers/shared_plugin_helpers.h>

// #include "Tintin/TintinMapper.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include "BinaryData.h"
#include "TintinMapper.h"

struct PianoHighlightState
{
    std::array<bool, 128> staticT {};   // allowed T notes (yellow)
    std::array<bool, 128> inputM {};    // incoming M notes (blue)
    std::array<bool, 128> outputT {};   // Tintin T output (red)
};

class TinTinProcessor : public PluginHelpers::ProcessorBase

{
public:
    TinTinProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void processBlock  (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

    const PianoHighlightState& getPianoHighlightState() const noexcept { return pianoHighlightState; }
    juce::MidiKeyboardState&   getPreviewKeyboardState() noexcept      { return previewKeyboardState; }

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    Parameters& getParams() { return params; }
    const Parameters& getParams() const { return params; }

private:
    void updateOptions();
    void updateStaticTGrid();
    void updateHighlightState (const juce::MidiBuffer& mInput,
                               const juce::MidiBuffer& tOutput);

    void loadSample(const void* data, int dataSize, int rootMidiNote);
    void loadPianoSound();

    Parameters params;
    TintinMapper tintin;

    juce::Synthesiser      piano;
    juce::AudioFormatManager formatManager;

    PianoHighlightState pianoHighlightState;
    juce::MidiKeyboardState previewKeyboardState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TinTinProcessor)
};
