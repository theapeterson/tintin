// Plugins/TinTin/Source/Tintin/TintinMapper.cpp
#include "TintinMapper.h"
#include <algorithm>

static int toPitchClass(int midi)
{
    auto pc = midi % 12;
    return pc < 0 ? pc + 12 : pc;
}

static void getSortedChordPcs(const TintinChord& chord, int (&pcs)[3])
{
    pcs[0] = chord.pc1;
    pcs[1] = chord.pc2;
    pcs[2] = chord.pc3;
    std::sort(pcs, pcs + 3);
}

static int nextChordToneUpOnce(int note, const TintinChord& chord)
{
    int pcs[3];
    getSortedChordPcs(chord, pcs);

    auto pc = toPitchClass(note);
    int best = 128;

    for (int i = 0; i < 3; ++i)
    {
        auto diff = pcs[i] - pc;
        if (diff <= 0)
            diff += 12;

        if (diff < best)
            best = diff;
    }

    return note + best;
}

static int nextChordToneDownOnce(int note, const TintinChord& chord)
{
    int pcs[3];
    getSortedChordPcs(chord, pcs);

    auto pc = toPitchClass(note);
    int best = 128;

    for (int i = 0; i < 3; ++i)
    {
        auto diff = pc - pcs[i];
        if (diff <= 0)
            diff += 12;

        if (diff < best)
            best = diff;
    }

    return note - best;
}

static int clampVelocity(int v)
{
    if (v < 1)
        return 1;

    if (v > 127)
        return 127;

    return v;
}


void TintinMapper::resetOrbit()
{
    settings.orbitCounter = 0;
    scheduler.clear();
}

void TintinMapper::process(juce::MidiBuffer& midi,
                           double sampleRate,
                           int numSamples)
{
    // build chord from current settings
    chord.setFromTriad(settings.rootNote, settings.triad);

    juce::MidiBuffer output;

    // keep original M-voice events
    for (const auto metadata : midi)
        output.addEvent(metadata.getMessage(), metadata.samplePosition);

    // generate T-voice
    for (const auto metadata : midi)
    {
        auto msg = metadata.getMessage();
        auto pos = metadata.samplePosition;

        if (! (msg.isNoteOn() || msg.isNoteOff()))
            continue;

        auto mNote = msg.getNoteNumber();
        auto qNote = applyQuantizer(mNote);
        auto tNote = computeTintinNote(qNote);

        auto delaySec = getDelaySeconds(settings);
        int delaySamples = (int) std::round(delaySec * sampleRate);

        if (delaySamples < 0)
            delaySamples = 0;

        if (msg.isNoteOn())
        {
            auto mVel = (int) msg.getVelocity();
            auto tVel = applyVelocity(mVel);

            juce::MidiMessage tOn { juce::MidiMessage::noteOn(msg.getChannel(),
                                                              tNote,
                                                              (juce::uint8) tVel) };

            scheduler.add(tOn, delaySamples, pos);

            // feedback repeats
            for (int r = 0; r < settings.feedbackRepeats; ++r)
            {
                auto extraDelay = delaySamples * (r + 1);
                scheduler.add(tOn, delaySamples + extraDelay, pos);
            }

            // multiple T voices (basic: extra voice mirrored)
            for (int v = 1; v < settings.numTVoices; ++v)
            {
                // simple: alternate direction for second voice
                auto altNote = tNote;

                if (settings.mode == TintinSettings::TMode::Plus1)
                    altNote = computeTintinNote(qNote); // same for now; can be extended

                juce::MidiMessage altOn { juce::MidiMessage::noteOn(msg.getChannel(),
                                                                    altNote,
                                                                    (juce::uint8) tVel) };
                scheduler.add(altOn, delaySamples, pos);
            }
        }
        else if (msg.isNoteOff())
        {
            juce::MidiMessage tOff { juce::MidiMessage::noteOff(msg.getChannel(), tNote) };

            scheduler.add(tOff, delaySamples, pos);

            for (int r = 0; r < settings.feedbackRepeats; ++r)
            {
                auto extraDelay = delaySamples * (r + 1);
                scheduler.add(tOff, delaySamples + extraDelay, pos);
            }

            for (int v = 1; v < settings.numTVoices; ++v)
            {
                juce::MidiMessage altOff { juce::MidiMessage::noteOff(msg.getChannel(), tNote) };
                scheduler.add(altOff, delaySamples, pos);
            }
        }
    }

    // emit scheduled T-voice events for this block
    scheduler.processBlock(output, numSamples);

    midi.swapWith(output);
}


int TintinMapper::computeTintinNote(int mNote)
{
    using TMode = TintinSettings::TMode;

    if (settings.mode == TMode::Orbit)
    {
        auto isEven = (settings.orbitCounter % 2) == 0;
        settings.orbitCounter = settings.orbitCounter + 1;

        if (isEven)
        {
            auto n = nextChordToneUpOnce(mNote, chord);
            return n + settings.octaveOffset * 12;
        }

        auto n = nextChordToneDownOnce(mNote, chord);
        return n + settings.octaveOffset * 12;
    }

    if (settings.mode == TMode::Plus1)
    {
        auto n = nextChordToneUpOnce(mNote, chord);
        return n + settings.octaveOffset * 12;
    }

    if (settings.mode == TMode::Plus2)
    {
        auto n = nextChordToneUpOnce(nextChordToneUpOnce(mNote, chord), chord);
        return n + settings.octaveOffset * 12;
    }

    if (settings.mode == TMode::Minus1)
    {
        auto n = nextChordToneDownOnce(mNote, chord);
        return n + settings.octaveOffset * 12;
    }

    // -2
    auto n = nextChordToneDownOnce(nextChordToneDownOnce(mNote, chord), chord);
    return n + settings.octaveOffset * 12;
}

int TintinMapper::applyVelocity(int mVelocity) const
{
    using VM = TintinSettings::VelocityMode;

    if (settings.velocityMode == VM::Follow)
        return clampVelocity(mVelocity);

    if (settings.velocityMode == VM::Scaled)
    {
        auto scaled = (int) ((float) mVelocity * settings.velocityScale);
        return clampVelocity(scaled);
    }

    return clampVelocity(settings.fixedVelocity);
}

int TintinMapper::applyQuantizer(int mNote) const
{
    return TintinQuantizer::quantize(mNote,
                                     settings.scaleIndex,
                                     settings.rootNote);
}

// 16 sync values (client spec)
double TintinMapper::getSyncBeats(int index)
{
    static const double beats[16] =
    {
        0.25 / 4.0,                 // 1/16
        0.25 / 4.0 * 1.5,           // 1/16 dotted
        0.25 / 4.0 * (2.0 / 3.0),   // 1/16 triplet

        0.5 / 4.0,                  // 1/8
        0.5 / 4.0 * 1.5,            // 1/8 dotted
        0.5 / 4.0 * (2.0 / 3.0),    // 1/8 triplet

        1.0,                        // 1/4
        1.0 * 1.5,                  // 1/4 dotted
        1.0 * (2.0 / 3.0),          // 1/4 triplet

        2.0,                        // 1/2
        2.0 * 1.5,                  // 1/2 dotted
        2.0 * (2.0 / 3.0),          // 1/2 triplet

        4.0,                        // 1 bar
        8.0,                        // 2 bars
        12.0,                       // 3 bars
        16.0                        // 4 bars
    };

    if (index < 0)
        return beats[0];

    if (index > 15)
        return beats[15];

    return beats[index];
}

double TintinMapper::getDelaySeconds(const TintinSettings& s)
{
    using DM = TintinSettings::DisplacementMode;

    if (s.displacementMode == DM::None)
        return 0.0;

    if (s.displacementMode == DM::Absolute)
        return s.displacementMs / 1000.0;

    // Sync
    if (s.bpm <= 0.0)
        return 0.0;

    auto beats = getSyncBeats(s.syncIndex);
    auto secPerBeat = 60.0 / s.bpm;

    return beats * secPerBeat;
}
