#pragma once

#include <array>
#include <vector>
#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
// #include "PluginProcessor.h"


class TinTinProcessor;

class TintinLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TintinLookAndFeel();

    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOver,
                               bool isButtonDown) override;

    void drawButtonText (juce::Graphics& g,
                         juce::TextButton& button,
                         bool isMouseOver,
                         bool isButtonDown) override;
};


class TintinPianoView : public juce::Component
{
public:
    using NoteCallback = std::function<void (int midiNote, bool isDown)>;

    TintinPianoView();

    // static T layer (gold)
    void setStaticTNotes (const std::vector<int>& midiNotes);

    // live layers (blue + red) coming from processor model
    void setMVoiceState (const std::array<bool, 128>& state);
    void setTVoiceState (const std::array<bool, 128>& state);

    // called by editor to wire UI -> processor
    void setNoteCallback (NoteCallback cb) { noteCallback = std::move (cb); }

    // local response for mouse clicks
    void noteOnM  (int midi);
    void noteOffM (int midi);

    void paint     (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

private:
    static constexpr int numWhiteKeys    = 14; // C4..B5
    static constexpr int whitesPerOctave = 7;
    static constexpr int baseMidi        = 60; // C4
    static constexpr int numSemitones    = 24;

    void paintBasePiano  (juce::Graphics& g,
                          const juce::Rectangle<float>& bounds) const;
    void paintHighlights (juce::Graphics& g,
                          const juce::Rectangle<float>& bounds) const;

    int  midiToWhiteIndex (int midi) const;
    int  midiFromPosition (float x) const;

    void drawStripe (juce::Graphics& g,
                     const juce::Rectangle<float>& bounds,
                     float keyW,
                     float keyH,
                     int whiteIndex,
                     float heightFraction,
                     float yOffsetFraction,
                     const juce::Colour& colour) const;

    NoteCallback noteCallback;

    std::array<bool, 128> staticTNotes{}; // gold (all allowed)
    std::array<bool, 128> mVoiceActive{}; // blue (input)
    std::array<bool, 128> tVoiceActive{}; // red (output)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TintinPianoView)
};

class TinTinProcessorEditor : public juce::AudioProcessorEditor
    , private juce::Timer
{
public:
    explicit TinTinProcessorEditor (TinTinProcessor&);
    ~TinTinProcessorEditor() override;

    void paint   (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    static int midiRootFromIndex (int index);
    static int modeIndexFromPositionButton (int index);
    static int positionButtonFromModeIndex (int modeIndex);

    void initRootButtons();
    void initTriadButtons();
    void initPositionButtons();
    void initDelayButtons();
    void initMVoiceButton();
    void initPianoToggles();

    void syncFromParams();
    void syncPianoFromProcessor();

    void selectRoot      (int index);
    void selectTriad     (bool isMajor);
    void selectPosition  (int index);
    void setMVoice       (bool on);
    void setSyncButton   (int buttonIndex);
    void setFreeDelayMode();

    void handlePianoNote (int midiNote, bool isDown);

    void updateRootButtons();
    void updateTriadButtons();
    void updatePositionButtons();
    void updateMVoiceButton();
    void updateDelayButtons();

    static constexpr int margin   = 10;
    static constexpr int pad      = 6;
    static constexpr int buttonH  = 26;
    static constexpr int smallH   = 22;
    static constexpr int leftWidth = 280;
    static constexpr int pianoHeight = 200;

    TinTinProcessor& processor;
    TintinLookAndFeel lf;

    int rootIndexUI = 0;

    juce::Label titleLabel;

    std::array<juce::TextButton, 12> rootButtons;

    juce::TextButton majorButton { "Major" };
    juce::TextButton minorButton { "Minor" };

    juce::TextButton posNoneButton { "None" };
    juce::TextButton pos2SupButton { "2nd Position Superior" };
    juce::TextButton pos1SupButton { "1st Position Superior" };
    juce::TextButton pos1InfButton { "1st Position Inferior" };
    juce::TextButton pos2InfButton { "2nd Position Inferior" };

    juce::TextButton mVoiceButton { "M-Voice Heard" };

    TintinPianoView pianoView;

    juce::TextButton tHighlightButton  { "" };
    juce::TextButton orbitToggleButton { "\u00B1" };
    juce::TextButton mHighlightButton  { "" };
    juce::TextButton randomButton      { "?" };

    static constexpr int numSyncButtons = 8;
    juce::TextButton nowButton { "Now" };
    std::array<juce::TextButton, numSyncButtons> syncButtons;
    juce::TextButton freeButton { "Free" };
    juce::Slider     msSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TinTinProcessorEditor)
};
