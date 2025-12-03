#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "TintinSettings.h"
#include "TintinChord.h"
#include "TintinScheduler.h"

struct TintinMapper
{
    TintinSettings  settings;
    TintinChord     chord;
    TintinScheduler scheduler;

    void resetOrbit();
    void process(juce::MidiBuffer& midi,
                 double sampleRate,
                 int numSamples);

private:
    int computeTintinNote(int mNote);
    int applyVelocity(int mVelocity) const;
    int applyQuantizer(int mNote) const;

    static double getSyncBeats(int index);
    static double getDelaySeconds(const TintinSettings& s);
};
