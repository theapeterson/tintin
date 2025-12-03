// Plugins/TinTin/Source/PluginEditor.h
#pragma once

#include <array>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//=============================================
// Simple custom look-and-feel for gold/grey UI
//=============================================
class TintinLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TintinLookAndFeel()
    {
        using namespace juce;

        setColour (ResizableWindow::backgroundColourId, Colour (0xff303030));
        setColour (TextButton::buttonColourId,          Colour (0xff404040));
        setColour (TextButton::buttonOnColourId,        Colour (0xfff3a623)); // gold
        setColour (Label::textColourId,                 Colours::goldenrod);
    }

    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOver,
                               bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        auto isOn = button.getToggleState();
        auto base = isOn ? findColour (juce::TextButton::buttonOnColourId)
                         : backgroundColour;

        auto border  = juce::Colours::black;
        auto hover   = base.brighter (0.1f);
        auto current = (isMouseOver || isButtonDown) ? hover : base;

        g.setColour (current);
        g.fillRoundedRectangle (bounds, 3.0f);

        g.setColour (border);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);
    }

    void drawButtonText (juce::Graphics& g,
                         juce::TextButton& button,
                         bool isMouseOver,
                         bool isButtonDown) override
    {
        juce::ignoreUnused (isMouseOver, isButtonDown);

        auto bounds = button.getLocalBounds().toFloat();

        auto text   = button.getButtonText();
        auto font   = juce::Font (14.0f);
        auto colour = juce::Colours::black;

        if (! button.isEnabled())
            colour = juce::Colours::darkgrey;

        g.setColour (colour);
        g.setFont (font);
        g.drawFittedText (text, bounds.toNearestInt(), juce::Justification::centred, 1);
    }
};

//=============================================
// Simple static piano view
//=============================================
class TintinPianoView : public juce::Component
{
public:
    TintinPianoView() = default;

    void paint (juce::Graphics& g) override
    {
        using namespace juce;

        auto bounds = getLocalBounds().toFloat();

        // draw white key bed
        g.setColour (Colours::white);
        g.fillRect (bounds);

        g.setColour (Colours::black.withAlpha (0.4f));
        g.drawRect (bounds, 1.0f);

        // very simple 2-octave piano (C to B) just for visuals
        const int numWhiteKeys = 14;
        auto keyW = bounds.getWidth() / (float) numWhiteKeys;
        auto keyH = bounds.getHeight();

        // white keys
        for (int i = 0; i < numWhiteKeys; ++i)
        {
            auto x = bounds.getX() + keyW * (float) i;
            auto r = juce::Rectangle<float> (x, bounds.getY(), keyW, keyH);
            g.setColour (Colours::white);
            g.fillRect (r);
            g.setColour (Colours::black.withAlpha (0.3f));
            g.drawRect (r, 0.5f);
        }

        // black keys pattern: 2-3-2-3 etc, just approximate
        auto drawBlack = [&] (int whiteIndex)
        {
            auto x = bounds.getX() + keyW * ((float) whiteIndex + 0.7f);
            auto r = juce::Rectangle<float> (x, bounds.getY(), keyW * 0.6f, keyH * 0.6f);
            g.setColour (Colours::black);
            g.fillRect (r);
        };

        g.setColour (juce::Colours::black);

        // pattern over 14 white keys (C D E F G A B C D E F G A B)
        int pattern[] = { 0, 1, 3, 4, 5, 7, 8, 10, 11, 12 };
        for (auto idx : pattern)
        {
            if (idx < numWhiteKeys - 1)
                drawBlack (idx);
        }
    }
};

//=============================================
// Main editor
//=============================================
class TinTinProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit TinTinProcessorEditor (TinTinProcessor&);
    ~TinTinProcessorEditor() override;

private:
    void paint   (juce::Graphics&) override;
    void resized() override;

    // helpers
    void syncFromParams();
    void selectRoot (int index);
    void selectTriad (bool major);
    void selectPosition (int index);
    void setMVoice (bool on);
    void setSyncButton (int buttonIndex);
    void setFreeDelayMode();

    void updateRootButtons();
    void updateTriadButtons();
    void updatePositionButtons();
    void updateMVoiceButton();
    void updateDelayButtons();

    // constants
    static constexpr int margin   = 10;
    static constexpr int pad      = 6;
    static constexpr int buttonH  = 26;
    static constexpr int smallH   = 22;

    // mapping
    static int midiRootFromIndex (int idx);
    static int modeIndexFromPositionButton (int idx);
    static int positionButtonFromModeIndex (int modeIndex);

    TinTinProcessor& processor;
    TintinLookAndFeel lf;

    // TITLE
    juce::Label titleLabel;

    // ROOT GRID: 12 buttons (A, Bb, B, C, Db, D, Eb, E, F, F#, G, G#)
    std::array<juce::TextButton, 12> rootButtons;

    // TRIAD
    juce::TextButton majorButton { "Major" };
    juce::TextButton minorButton { "Minor" };

    // POSITION MODES
    juce::TextButton posNoneButton      { "None" };
    juce::TextButton pos2SupButton      { "2nd Position Superior" };
    juce::TextButton pos1SupButton      { "1st Position Superior" };
    juce::TextButton pos1InfButton      { "1st Position Inferior" };
    juce::TextButton pos2InfButton      { "2nd Position Inferior" };

    // M-Voice
    juce::TextButton mVoiceButton { "M-Voice Heard" };

    // PIANO
    TintinPianoView pianoView;

    // SMALL TOGGLES UNDER PIANO
    juce::TextButton tHighlightButton   { "" };   // yellow square
    juce::TextButton orbitToggleButton  { "\u00B1" }; // ±
    juce::TextButton mHighlightButton   { "" };   // grey square
    juce::TextButton randomButton       { "?" };

    // DELAY ROW
    juce::TextButton nowButton   { "Now" };

    // 1n, 2n, 4d, 4n, 8d, 8n, 16d, 16n
    static constexpr int numSyncButtons = 8;
    std::array<juce::TextButton, numSyncButtons> syncButtons;

    juce::TextButton freeButton { "Free" };
    juce::Slider     msSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TinTinProcessorEditor)
};
