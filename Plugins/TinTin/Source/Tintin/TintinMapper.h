// Plugins/TinTin/Source/Tintinnabuli/TintinnabuliMapper.h
#pragma once

#include "TintinTypes.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace Tintinnabuli
{
class Mapper
{
public:
    Context context;

    void resetOrbit() noexcept { context.orbitNextUp = true; }

    // In-place MIDI transform: M-voice in, T-voice out
    void process (juce::MidiBuffer& midi);

private:
    int mapNoteForMode (int inputNote, TMode mode) const;
    int mapPlus (int inputNote, int stepsUp) const;   // T+1, T+2...
    int mapMinus (int inputNote, int stepsDown) const;// T-1, T-2...
    int applyOctaveOffset (int note) const;

    int computeVelocity (const juce::MidiMessage& m) const;
};
} // namespace Tintinnabuli
