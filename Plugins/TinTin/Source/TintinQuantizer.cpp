// Plugins/TinTin/Source/Tintin/TintinQuantizer.cpp
#include "TintinQuantizer.h"

static int wrapPc(int n)
{
    auto pc = n % 12;
    return pc < 0 ? pc + 12 : pc;
}

std::vector<int> TintinQuantizer::getScale(int s)
{
    // 0 = chromatic
    if (s == 0)
        return { 0,1,2,3,4,5,6,7,8,9,10,11 };

    switch (s)
    {
        case 1: return { 0,2,4,5,7,9,11 };       // Major
        case 2: return { 0,2,3,5,7,8,10 };       // Natural minor
        case 3: return { 0,2,3,5,7,9,10 };       // Dorian
        case 4: return { 0,1,3,5,7,8,10 };       // Phrygian
        case 5: return { 0,2,4,6,7,9,11 };       // Lydian
        case 6: return { 0,2,4,5,7,9,10 };       // Mixolydian
        case 7: return { 0,1,3,5,6,8,10 };       // Locrian
        case 8: return { 0,2,4,7,9 };            // Pentatonic major
        case 9: return { 0,3,5,7,10 };           // Pentatonic minor
    }

    // fallback to chromatic
    return { 0,1,2,3,4,5,6,7,8,9,10,11 };
}

int TintinQuantizer::quantize(int midiNote,
                              int scaleIndex,
                              int rootMidiNote)
{
    if (scaleIndex == 0)
        return midiNote; // chromatic, no quantize

    auto scale = getScale(scaleIndex);
    if (scale.empty())
        return midiNote;

    auto pc = wrapPc(midiNote - rootMidiNote);

    int bestPc = scale[0];
    int bestDist = 128;

    for (auto s : scale)
    {
        auto diff = s - pc;
        if (diff < 0)
            diff = -diff;

        if (diff < bestDist)
        {
            bestDist = diff;
            bestPc = s;
        }
    }

    auto baseOct = midiNote / 12;
    return baseOct * 12 + rootMidiNote + bestPc - (rootMidiNote % 12);
}
