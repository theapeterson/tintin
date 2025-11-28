// Plugins/TinTin/Source/Tintinnabuli/TintinnabuliTypes.h
#pragma once

#include <vector>
#include <algorithm>
#include <cstdint>

namespace Tintinnabuli
{
constexpr int kMidiMin = 0;
constexpr int kMidiMax = 127;

enum class TriadType
{
    Major = 0,
    Minor = 1
};

enum class TMode
{
    Plus1 = 0,
    Plus2,
    Minus1,
    Minus2,
    Orbit
};

enum class VelocityMode
{
    Follow = 0,
    Scaled,
    Fixed
};

struct Context
{
    int rootNote      = 60;                    // C4
    TriadType triad   = TriadType::Major;
    TMode mode        = TMode::Plus1;
    int octaveOffset  = 0;                     // in octaves
    VelocityMode velocityMode = VelocityMode::Follow;
    float velocityScale       = 0.7f;          // used if Scaled
    int fixedVelocity         = 90;            // used if Fixed

    // internal orbit state (alternate T+ / T− per note)
    bool orbitNextUp          = true;
};

// Build all chord tones (T-chord) across the MIDI range
inline std::vector<int> buildChordPitches (int root, TriadType triad)
{
    const int third = (triad == TriadType::Major ? 4 : 3);
    const int offsets[3] = { 0, third, 7 };

    std::vector<int> tones;
    tones.reserve (128);

    // cover full MIDI range generously
    for (int oct = -2; oct <= 8; ++oct)
    {
        const int base = root + 12 * oct;

        for (int off : offsets)
        {
            const int note = base + off;
            if (note >= kMidiMin && note <= kMidiMax)
                tones.push_back (note);
        }
    }

    std::sort (tones.begin (), tones.end ());
    return tones;
}
} // namespace Tintinnabuli
