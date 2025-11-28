// Plugins/TinTin/Source/PluginProcessor.h
#pragma once

#include "Parameters.h"
#include <shared_plugin_helpers/shared_plugin_helpers.h>

#include "Tintin/TintinMapper.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include "BinaryData.h"


class TinTinProcessor : public PluginHelpers::ProcessorBase
{
public:
    TinTinProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void processBlock  (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

private:
    void updateOptions();
    void loadPianoSound();

    Parameters params;

    Tintinnabuli::Mapper tintin;

    juce::Synthesiser      piano;
    juce::AudioFormatManager formatManager;
};
