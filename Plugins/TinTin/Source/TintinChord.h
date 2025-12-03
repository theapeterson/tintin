////Plugins/Tintin/Source/Tintin/TintinChord.h
#pragma once

#include "TintinSettings.h"

struct TintinChord
{
    // pitch classes 0 thru 11
    int pc1 = 0;
    int pc2 = 4;
    int pc3 = 7;

    void setFromTriad(int rootMidiNote, TintinSettings::TriadType triad)
    {
        auto rootPc = rootMidiNote % 12;
        if (rootPc < 0)
            rootPc += 12;

        if (triad == TintinSettings::TriadType::Major)
        {
            pc1 = rootPc;
            pc2 = (rootPc + 4) % 12;
            pc3 = (rootPc + 7) % 12;
            return;
        }

        pc1 = rootPc;
        pc2 = (rootPc + 3) % 12;
        pc3 = (rootPc + 7) % 12;
    }
};
