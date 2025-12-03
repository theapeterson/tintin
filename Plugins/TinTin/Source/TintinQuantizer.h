// Plugins/TinTin/Source/Tintin/TintinQuantizer.h
#pragma once

#include <vector>

struct TintinQuantizer
{
    // ok...so quantize midiNote into the scale indicated by scaleIndex, use root as tonic for now
    // scaleIndex:
    //   0 = chromatic (no quantize)
    //   1..9 = scales defined in getScale()
    static int quantize(int midiNote, int scaleIndex, int rootMidiNote);

private:
    static std::vector<int> getScale(int scaleIndex);
};
