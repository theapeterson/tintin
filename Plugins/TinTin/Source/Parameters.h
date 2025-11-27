#pragma once

#include <shared_plugin_helpers/shared_plugin_helpers.h>

// struct Parameters
// {
//     struct IDs
//     {
//         static constexpr auto rootNote     = "rootNote";
//         static constexpr auto triadType    = "triadType";
//         static constexpr auto tMode        = "tMode";
//         static constexpr auto octaveOffset = "octaveOffset";
//         static constexpr auto velocityMode = "velocityMode";
//     };
//
//     void add(juce::AudioProcessor& processor) const
//     {
//         processor.addParameter(gain);
//         processor.addParameter(enable);
//     }
//
//     //Raw pointers. They will be owned by either the processor or the APVTS (if you use it)
//     juce::AudioParameterFloat* gain =
//         new juce::AudioParameterFloat({"Gain", 1}, "Gain", 0.f, 1.f, 0.5f);
//
//     juce::AudioParameterBool* enable =
//         new juce::AudioParameterBool({"Enable", 1}, "Enable", true);
// };
//
// #pragma once
//
// #include <shared_plugin_helpers/shared_plugin_helpers.h>
//
// namespace Params
// {
// struct IDs
// {
//     static constexpr auto root = "root";
//     static constexpr auto triad = "triad";
//     static constexpr auto mode = "mode";
//     static constexpr auto octave = "octave";
//     static constexpr auto velocity = "velocity";
// };
//
// inline void addAll(PluginHelpers::ParameterList& params)
// {
//     using namespace juce;
//
//     params.add(std::make_unique<AudioParameterInt>(IDs::root,
//         "Root", 0, 127, 60));
//
//     params.add(std::make_unique<AudioParameterChoice>(IDs::triad,
//         "Triad", StringArray{ "Major", "Minor" }, 1));
//
//     params.add(std::make_unique<AudioParameterChoice>(IDs::mode,
//         "T-Mode", StringArray{ "T+1", "T+2", "T-1", "T-2", "Orbit" }, 0));
//
//     params.add(std::make_unique<AudioParameterInt>(IDs::octave,
//         "Octave", -3, 3, 0));
//
//     params.add(std::make_unique<AudioParameterChoice>(IDs::velocity,
//         "Velocity", StringArray{ "Follow", "Scaled", "Fixed" }, 0));
// }
// } // namespace Params
#pragma once

#include <shared_plugin_helpers/shared_plugin_helpers.h>

struct Parameters
{
    struct IDs
    {
        static constexpr auto root     = "root";
        static constexpr auto triad    = "triad";
        static constexpr auto mode     = "mode";
        static constexpr auto octave   = "octave";
        static constexpr auto velocity = "velocity";
    };

    void add(juce::AudioProcessor& p) const
    {
        p.addParameter(rootNote);
        p.addParameter(triadType);
        p.addParameter(modeSelect);
        p.addParameter(octaveOffset);
        p.addParameter(velocityMode);
    }

    juce::AudioParameterInt* rootNote =
        new juce::AudioParameterInt({ IDs::root, 1 }, "Root",
                                    0, 127, 60);

    juce::AudioParameterChoice* triadType =
        new juce::AudioParameterChoice({ IDs::triad, 1 }, "Triad",
                                       juce::StringArray{ "Major", "Minor" }, 1);

    juce::AudioParameterChoice* modeSelect =
        new juce::AudioParameterChoice({ IDs::mode, 1 }, "T-Mode",
                                       juce::StringArray{ "T+1", "T+2",
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
};
