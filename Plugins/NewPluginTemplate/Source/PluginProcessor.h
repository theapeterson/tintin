#pragma once

#include "Parameters.h"
#include <ea_midi_mapper/ea_midi_mapper.h>

class NewPluginTemplateAudioProcessor : public PluginHelpers::ProcessorBase
{
public:
    NewPluginTemplateAudioProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:

    Parameters parameters;
    EA::MIDI::RandomTransposer randomTransposer;
    EA::MIDI::Mapper mapper;
};
