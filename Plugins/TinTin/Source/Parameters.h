//Plugins/Tintin/Source/Tintin/Parameters.h
#pragma once

#include <shared_plugin_helpers/shared_plugin_helpers.h>

#pragma once

struct Parameters
{
    struct IDs
    {
        static constexpr auto root     = "root";
        static constexpr auto triad    = "triad";
        static constexpr auto mode     = "mode";
        static constexpr auto octave   = "octave";
        static constexpr auto velocity = "velocity";
        static constexpr auto dispMode   = "dispMode";
        static constexpr auto dispSync   = "dispSync";
        static constexpr auto dispMs     = "dispMs";
        static constexpr auto velScale   = "velScale";
        static constexpr auto velFixed   = "velFixed";
        static constexpr auto scale      = "scale";
        static constexpr auto mVoice   = "mvoice";
    };

    void add(juce::AudioProcessor& p) const
    {
        p.addParameter(rootNote);
        p.addParameter(triadType);
        p.addParameter(modeSelect);
        p.addParameter(octaveOffset);
        p.addParameter(velocityMode);
        p.addParameter(displacementMode);
        p.addParameter(displacementSync);
        p.addParameter(displacementMs);
        p.addParameter(velocityScaleParam);
        p.addParameter(fixedVelocityParam);
        p.addParameter(scaleSelect);
        p.addParameter(mVoiceOn);
    }

    juce::AudioParameterInt* rootNote =
        new juce::AudioParameterInt({ IDs::root, 1 }, "Root",
                                    0, 127, 60);

    juce::AudioParameterChoice* triadType =
        new juce::AudioParameterChoice({ IDs::triad, 1 }, "Triad",
                                       juce::StringArray{ "Major", "Minor" }, 1);

    juce::AudioParameterChoice* modeSelect =
        new juce::AudioParameterChoice({ IDs::mode, 1 }, "T-Mode",
                                       juce::StringArray{ "None",
                                                          "T+1", "T+2",
                                                          "T-1", "T-2",
                                                          "Orbit" },
                                       0);

    juce::AudioParameterInt* octaveOffset =
        new juce::AudioParameterInt({ IDs::octave, 1 }, "Octave",
                                    -3, 3, 0);

    juce::AudioParameterChoice* velocityMode =
        new juce::AudioParameterChoice({ IDs::velocity, 1 }, "Velocity",
                                       juce::StringArray{ "Follow",
                                                          "Scaled",
                                                          "Fixed" },
                                       0);
    juce::AudioParameterChoice* displacementMode =
    new juce::AudioParameterChoice({ IDs::dispMode, 1 }, "Displacement",
                                   juce::StringArray{ "None", "Sync", "Absolute" }, 0);

    juce::AudioParameterInt* displacementSync =
        new juce::AudioParameterInt({ IDs::dispSync, 1 }, "Sync Index",
                                    0, 15, 0); // 16 sync values

    juce::AudioParameterFloat* displacementMs =
        new juce::AudioParameterFloat({ IDs::dispMs, 1 }, "Delay (ms)",
                                      10.0f, 2000.0f, 100.0f);

    juce::AudioParameterFloat* velocityScaleParam =
        new juce::AudioParameterFloat({ IDs::velScale, 1 }, "Velocity Scale",
                                      0.0f, 1.0f, 1.0f);

    juce::AudioParameterInt* fixedVelocityParam =
        new juce::AudioParameterInt({ IDs::velFixed, 1 }, "Fixed Velocity",
                                    1, 127, 90);

    juce::AudioParameterChoice* scaleSelect =
        new juce::AudioParameterChoice({ IDs::scale, 1 }, "Scale",
            juce::StringArray{
                "Chromatic",
                "Major",
                "Minor",
                "Dorian",
                "Phrygian",
                "Lydian",
                "Mixolydian",
                "Locrian",
                "Pentatonic Major",
                "Pentatonic Minor"
            },
            0);
    juce::AudioParameterBool* mVoiceOn =
    new juce::AudioParameterBool({ IDs::mVoice, 1 }, "M Voice Heard", true);

};
