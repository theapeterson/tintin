// Plugins/TinTin/Source/Tintinnabuli/TintinnabuliMapper.cpp
#include "TintinnabuliMapper.h"

using namespace Tintinnabuli;

int Mapper::applyOctaveOffset (int note) const
{
    const int shifted = note + context.octaveOffset * 12;
    return std::clamp (shifted, kMidiMin, kMidiMax);
}

int Mapper::mapPlus (int inputNote, int stepsUp) const
{
    auto chord = buildChordPitches (context.rootNote, context.triad);

    // first chord tone >= input
    auto it = std::lower_bound (chord.begin(), chord.end(), inputNote);
    if (it == chord.end())
        it = chord.end() - 1;

    auto index = static_cast<int> (std::distance (chord.begin(), it)) + stepsUp;
    if (index >= static_cast<int> (chord.size()))
        index = static_cast<int> (chord.size()) - 1;

    return applyOctaveOffset (chord[(size_t) index]);
}

int Mapper::mapMinus (int inputNote, int stepsDown) const
{
    auto chord = buildChordPitches (context.rootNote, context.triad);

    auto it = std::lower_bound (chord.begin(), chord.end(), inputNote);

    if (it == chord.begin())
    {
        // everything is >= input; clamp to first
        // and step down from there (still clamps)
    }
    else if (it == chord.end() || *it > inputNote)
    {
        // move to last <= input
        --it;
    }

    int index = static_cast<int> (std::distance (chord.begin(), it)) - stepsDown;
    if (index < 0)
        index = 0;

    return applyOctaveOffset (chord[(size_t) index]);
}

int Mapper::mapNoteForMode (int inputNote, TMode mode) const
{
    switch (mode)
    {
        case TMode::Plus1:   return mapPlus  (inputNote, 0); // nearest higher
        case TMode::Plus2:   return mapPlus  (inputNote, 1); // second higher
        case TMode::Minus1:  return mapMinus (inputNote, 0); // nearest lower
        case TMode::Minus2:  return mapMinus (inputNote, 1); // second lower
        case TMode::Orbit:   break; // handled in process()
    }

    // fallback
    return applyOctaveOffset (inputNote);
}

int Mapper::computeVelocity (const juce::MidiMessage& m) const
{
    const int inVel = juce::jlimit (0, 127, (int) std::round (m.getVelocity() * 127.0f));

    switch (context.velocityMode)
    {
        case VelocityMode::Follow:
            return inVel;

        case VelocityMode::Scaled:
        {
            int v = (int) std::round (inVel * context.velocityScale);
            return juce::jlimit (1, 127, v);
        }

        case VelocityMode::Fixed:
            return juce::jlimit (1, 127, context.fixedVelocity);
    }

    return inVel;
}

void Mapper::process (juce::MidiBuffer& midi)
{
    juce::MidiBuffer output;
    output.ensureSize (midi.getNumEvents());

    for (auto metadata : midi)
    {
        const auto& msg = metadata.getMessage();
        const int pos   = metadata.samplePosition;

        if (msg.isNoteOnOrOff())
        {
            Tintinnabuli::TMode effectiveMode = context.mode;

            if (context.mode == TMode::Orbit)
            {
                effectiveMode = context.orbitNextUp ? TMode::Plus1
                                                    : TMode::Minus1;
                context.orbitNextUp = !context.orbitNextUp;
            }

            const int inputNote = msg.getNoteNumber();
            const int tNote     = mapNoteForMode (inputNote, effectiveMode);

            if (msg.isNoteOn())
            {
                const int vel = computeVelocity (msg);

                auto tOn = juce::MidiMessage::noteOn (msg.getChannel(),
                                                      tNote,
                                                      (juce::uint8) vel);
                tOn.setTimeStamp (msg.getTimeStamp());
                output.addEvent (tOn, pos);
            }
            else // note off
            {
                auto tOff = juce::MidiMessage::noteOff (msg.getChannel(), tNote);
                tOff.setTimeStamp (msg.getTimeStamp());
                output.addEvent (tOff, pos);
            }
        }
        else
        {
            // pass through other messages (CC, pitch bend, etc.)
            output.addEvent (msg, pos);
        }
    }

    midi.swapWith (output);
}
