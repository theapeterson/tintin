////Plugins/Tintin/Source/Tintin/TintinSettings.h
#pragma once

struct TintinSettings
{
    enum class TriadType
    {
        Major,
        Minor
    };

    enum class TMode
    {
        Plus1,
        Plus2,
        Minus1,
        Minus2,
        Orbit
    };

    enum class VelocityMode
    {
        Follow,
        Scaled,
        Fixed
    };

    enum class DisplacementMode
    {
        None,
        Sync,
        Absolute
    };

    int rootNote = 60;
    TriadType triad = TriadType::Major;
    TMode mode = TMode::Plus1;

    int octaveOffset = 0;    // -3..3
    int orbitCounter = 0;

    VelocityMode velocityMode = VelocityMode::Follow;
    float velocityScale = 1.0f;   // 0..1
    int fixedVelocity = 90;       // 1..127

    DisplacementMode displacementMode = DisplacementMode::None;
    int syncIndex = 0;            // index into sync table
    float displacementMs = 0.0f;

    int scaleIndex = 0;         // which scale quantizer to use
    int feedbackRepeats = 0;    // optional repeats (refine for v2....?)
    int numTVoices = 1;         // optional extra voices (refine for v2....?)

    double bpm = 120.0;         // host tempo, needed for sync displacement..
};
