// Plugins/TinTin/Source/Tintin/TintinScheduler.h
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

struct TintinScheduler
{
    struct Pending
    {
        juce::MidiMessage msg;
        int samplesRemaining = 0;  // when this hits < numSamples, event is in current block
    };

    void clear();
    void add(const juce::MidiMessage& msg,
             int delaySamples,
             int baseSamplePos);
    void processBlock(juce::MidiBuffer& out, int numSamples);

private:
    std::vector<Pending> queue;
};
