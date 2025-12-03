// Plugins/TinTin/Source/PluginEditor.cpp
#include "PluginEditor.h"
#include "PluginProcessor.h"

//====================== static helpers ======================

int TinTinProcessorEditor::midiRootFromIndex (int idx)
{
    // 0..11 → A, Bb, B, C, Db, D, Eb, E, F, F#, G, G#
    // use octave 4-ish so chord PC is correct
    static const int roots[12] =
    {
        57, // A
        58, // Bb
        59, // B
        60, // C
        61, // Db
        62, // D
        63, // Eb
        64, // E
        65, // F
        66, // F#
        67, // G
        68  // G#
    };

    if (idx < 0)   return roots[0];
    if (idx > 11)  return roots[11];
    return roots[idx];
}

// UI position buttons → modeSelect index
// modeSelect choices: "None","T+1","T+2","T-1","T-2","Orbit"
// we only map the 5 position buttons here
int TinTinProcessorEditor::modeIndexFromPositionButton (int idx)
{
    switch (idx)
    {
        case 0: return 0; // None
        case 1: return 2; // 2nd Superior -> T+2
        case 2: return 1; // 1st Superior -> T+1
        case 3: return 3; // 1st Inferior -> T-1
        case 4: return 4; // 2nd Inferior -> T-2
        default: return 0;
    }
}

// modeSelect index -> which position button is active
int TinTinProcessorEditor::positionButtonFromModeIndex (int modeIndex)
{
    switch (modeIndex)
    {
        case 0: return 0; // None
        case 2: return 1; // T+2
        case 1: return 2; // T+1
        case 3: return 3; // T-1
        case 4: return 4; // T-2
        default: return -1; // Orbit or unknown
    }
}

//====================== ctor/dtor ======================

TinTinProcessorEditor::TinTinProcessorEditor (TinTinProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p)
{
    setLookAndFeel (&lf);

    // TITLE
    addAndMakeVisible (titleLabel);
    titleLabel.setText ("Tintinnabulator v1.3", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (juce::Font (26.0f, juce::Font::bold));

    // ROOT GRID
    {
        static const char* names[12] =
        {
            "A", "Bb", "B",
            "C", "Db", "D",
            "Eb","E","F",
            "F#","G","G#"
        };

        for (int i = 0; i < (int) rootButtons.size(); ++i)
        {
            auto& b = rootButtons[(size_t) i];
            b.setButtonText (names[i]);
            b.setClickingTogglesState (true);
            addAndMakeVisible (b);

            b.onClick = [this, i]()
            {
                selectRoot (i);
            };
        }
    }

    // TRIAD
    majorButton.setClickingTogglesState (true);
    minorButton.setClickingTogglesState (true);
    addAndMakeVisible (majorButton);
    addAndMakeVisible (minorButton);

    majorButton.onClick = [this]() { selectTriad (true);  };
    minorButton.onClick = [this]() { selectTriad (false); };

    // POSITION
    auto initPosButton = [this] (juce::TextButton& b, int idx)
    {
        b.setClickingTogglesState (true);
        addAndMakeVisible (b);
        b.onClick = [this, idx]() { selectPosition (idx); };
    };

    initPosButton (posNoneButton, 0);
    initPosButton (pos2SupButton, 1);
    initPosButton (pos1SupButton, 2);
    initPosButton (pos1InfButton, 3);
    initPosButton (pos2InfButton, 4);

    // M-Voice
    mVoiceButton.setClickingTogglesState (true);
    addAndMakeVisible (mVoiceButton);
    mVoiceButton.onClick = [this]()
    {
        setMVoice (mVoiceButton.getToggleState());
    };

    // PIANO
    addAndMakeVisible (pianoView);

    // small toggles under piano
    tHighlightButton.setClickingTogglesState (true);
    orbitToggleButton.setClickingTogglesState (true);
    mHighlightButton.setClickingTogglesState (true);
    randomButton.setClickingTogglesState (false);

    addAndMakeVisible (tHighlightButton);
    addAndMakeVisible (orbitToggleButton);
    addAndMakeVisible (mHighlightButton);
    addAndMakeVisible (randomButton);

    // orbit toggle: if on -> Orbit mode, if off -> revert to last position
    orbitToggleButton.onClick = [this]()
    {
        auto& params = processor.getParams();

        if (orbitToggleButton.getToggleState())
        {
            params.modeSelect->setValueNotifyingHost (5); // Orbit
        }
        else
        {
            // restore from position buttons
            for (int i = 0; i < 5; ++i)
            {
                if (positionButtonFromModeIndex (params.modeSelect->getIndex()) == i)
                    return; // already set

                if (rootButtons[0].isVisible()) {} // dummy to keep lambda trivial
            }
        }

        updatePositionButtons();
    };

    // randomizer: simple v1
    randomButton.onClick = [this]()
    {
        auto& params = processor.getParams();
        auto rng = juce::Random::getSystemRandom();

        auto rootIdx  = rng.nextInt (12);
        auto triadMaj = (rng.nextBool());
        auto posIdx   = rng.nextInt (5);
        auto syncIdx  = rng.nextInt (8);

        selectRoot (rootIdx);
        selectTriad (triadMaj);
        selectPosition (posIdx);
        setSyncButton (syncIdx);
    };

    // DELAY ROW
    addAndMakeVisible (nowButton);
    nowButton.setClickingTogglesState (true);
    nowButton.onClick = [this]()
    {
        auto& params = processor.getParams();
        params.displacementMode->setValueNotifyingHost (0); // None
        updateDelayButtons();
    };

    static const char* syncNames[numSyncButtons] =
    {
        "1n", "2n", "4d", "4n", "8d", "8n", "16d", "16n"
    };

    for (int i = 0; i < numSyncButtons; ++i)
    {
        auto& b = syncButtons[(size_t) i];
        b.setButtonText (syncNames[i]);
        b.setClickingTogglesState (true);
        addAndMakeVisible (b);

        b.onClick = [this, i]()
        {
            setSyncButton (i);
        };
    }

    freeButton.setClickingTogglesState (true);
    addAndMakeVisible (freeButton);
    freeButton.onClick = [this]()
    {
        setFreeDelayMode();
    };

    msSlider.setRange (10.0, 2000.0, 1.0);
    msSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 18);
    addAndMakeVisible (msSlider);

    msSlider.onValueChange = [this]()
    {
        auto& params = processor.getParams();
        params.displacementMs->setValueNotifyingHost ((float) msSlider.getValue());
    };

    setSize (800, 320);
    syncFromParams();
}

TinTinProcessorEditor::~TinTinProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//====================== sync helpers ======================

void TinTinProcessorEditor::syncFromParams()
{
    auto& params = processor.getParams();

    // root
    {
        // map midi → index
        int midi = params.rootNote->get();
        int idx  = 0;
        for (int i = 0; i < 12; ++i)
        {
            if (midiRootFromIndex (i) == midi)
            {
                idx = i;
                break;
            }
        }
        for (int i = 0; i < 12; ++i)
            rootButtons[(size_t) i].setToggleState (i == idx, juce::dontSendNotification);
    }

    // triad
    {
        auto idx = params.triadType->getIndex(); // 0=Major,1=Minor
        majorButton.setToggleState (idx == 0, juce::dontSendNotification);
        minorButton.setToggleState (idx == 1, juce::dontSendNotification);
    }

    // position / mode
    updatePositionButtons();

    // M-Voice
    updateMVoiceButton();

    // delay
    msSlider.setValue (params.displacementMs->get(), juce::dontSendNotification);
    updateDelayButtons();
}

void TinTinProcessorEditor::selectRoot (int index)
{
    auto midi = midiRootFromIndex (index);

    auto& params = processor.getParams();
    params.rootNote->setValueNotifyingHost (midi);

    updateRootButtons();
}

void TinTinProcessorEditor::selectTriad (bool major)
{
    auto& params = processor.getParams();

    if (major)
        params.triadType->setValueNotifyingHost (0);
    else
        params.triadType->setValueNotifyingHost (1);

    updateTriadButtons();
}

void TinTinProcessorEditor::selectPosition (int index)
{
    auto& params = processor.getParams();

    auto modeIdx = modeIndexFromPositionButton (index);
    params.modeSelect->setValueNotifyingHost (modeIdx);

    // leaving Orbit toggle off
    orbitToggleButton.setToggleState (false, juce::dontSendNotification);

    updatePositionButtons();
}

void TinTinProcessorEditor::setMVoice (bool on)
{
    auto& params = processor.getParams();
    params.mVoiceOn->setValueNotifyingHost (on);
    updateMVoiceButton();
}

void TinTinProcessorEditor::setSyncButton (int buttonIndex)
{
    auto& params = processor.getParams();

    // map the 8 visible buttons to 16 sync indices
    // 0: 1n  -> index 12 (1 bar)
    // 1: 2n  -> index 13 (2 bars)
    // 2: 4d  -> index 7  (1/4 dotted)
    // 3: 4n  -> index 6  (1/4)
    // 4: 8d  -> index 4  (1/8 dotted)
    // 5: 8n  -> index 3  (1/8)
    // 6: 16d -> index 1  (1/16 dotted)
    // 7: 16n -> index 0  (1/16)
    static const int syncMap[numSyncButtons] = { 12, 13, 7, 6, 4, 3, 1, 0 };

    if (buttonIndex < 0 || buttonIndex >= numSyncButtons)
        return;

    params.displacementMode->setValueNotifyingHost (1); // Sync
    params.displacementSync->setValueNotifyingHost (syncMap[buttonIndex]);

    updateDelayButtons();
}

void TinTinProcessorEditor::setFreeDelayMode()
{
    auto& params = processor.getParams();
    params.displacementMode->setValueNotifyingHost (2); // Absolute
    updateDelayButtons();
}

//====================== update button states ======================

void TinTinProcessorEditor::updateRootButtons()
{
    auto midi = processor.getParams().rootNote->get();

    int active = 0;
    for (int i = 0; i < 12; ++i)
    {
        if (midiRootFromIndex (i) == midi)
        {
            active = i;
            break;
        }
    }

    for (int i = 0; i < 12; ++i)
        rootButtons[(size_t) i].setToggleState (i == active, juce::dontSendNotification);
}

void TinTinProcessorEditor::updateTriadButtons()
{
    auto idx = processor.getParams().triadType->getIndex();
    majorButton.setToggleState (idx == 0, juce::dontSendNotification);
    minorButton.setToggleState (idx == 1, juce::dontSendNotification);
}

void TinTinProcessorEditor::updatePositionButtons()
{
    auto idx = processor.getParams().modeSelect->getIndex();
    auto pos = positionButtonFromModeIndex (idx);

    posNoneButton.setToggleState (pos == 0, juce::dontSendNotification);
    pos2SupButton.setToggleState (pos == 1, juce::dontSendNotification);
    pos1SupButton.setToggleState (pos == 2, juce::dontSendNotification);
    pos1InfButton.setToggleState (pos == 3, juce::dontSendNotification);
    pos2InfButton.setToggleState (pos == 4, juce::dontSendNotification);

    // orbit toggle
    orbitToggleButton.setToggleState (idx == 5, juce::dontSendNotification);
}

void TinTinProcessorEditor::updateMVoiceButton()
{
    bool on = processor.getParams().mVoiceOn->get();
    mVoiceButton.setToggleState (on, juce::dontSendNotification);
}

void TinTinProcessorEditor::updateDelayButtons()
{
    auto& params = processor.getParams();

    auto mode = params.displacementMode->getIndex(); // 0=None,1=Sync,2=Absolute
    auto sync = params.displacementSync->get();

    nowButton.setToggleState (mode == 0, juce::dontSendNotification);

    // clear sync + free
    for (auto& b : syncButtons)
        b.setToggleState (false, juce::dontSendNotification);

    freeButton.setToggleState (mode == 2, juce::dontSendNotification);

    if (mode == 1)
    {
        // reverse-map sync index to one of visible buttons
        static const int syncMap[numSyncButtons] = { 12, 13, 7, 6, 4, 3, 1, 0 };

        for (int i = 0; i < numSyncButtons; ++i)
        {
            if (syncMap[i] == sync)
            {
                syncButtons[(size_t) i].setToggleState (true, juce::dontSendNotification);
                break;
            }
        }
    }
}

//====================== paint / resized ======================

void TinTinProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;

    g.fillAll (findColour (ResizableWindow::backgroundColourId));

    auto bounds = getLocalBounds().toFloat().reduced (4.0f);
    auto inner  = bounds.reduced (4.0f);

    g.setColour (Colours::darkgrey);
    g.fillRoundedRectangle (inner, 6.0f);

    g.setColour (Colours::black);
    g.drawRoundedRectangle (inner, 6.0f, 1.0f);
}

void TinTinProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (margin);

    auto titleArea = r.removeFromTop (40);
    titleLabel.setBounds (titleArea.removeFromLeft (260));

    // LEFT = control column, RIGHT = piano
    auto left  = r.removeFromLeft (280);
    r.removeFromLeft (pad);
    auto right = r;

    // LEFT: root grid at top
    auto rootArea = left.removeFromTop (buttonH * 4 + pad * 3);

    auto row = rootArea;
    row.removeFromBottom (rootArea.getHeight() - buttonH); // first row

    auto cellW = row.getWidth() / 3;

    int idx = 0;
    auto areaCopy = rootArea;

    for (int rowIdx = 0; rowIdx < 4; ++rowIdx)
    {
        auto rowR = areaCopy.removeFromTop (buttonH);
        if (rowIdx < 3)
            areaCopy.removeFromTop (pad);

        for (int colIdx = 0; colIdx < 3; ++colIdx)
        {
            auto cell = rowR.removeFromLeft (cellW);
            rootButtons[(size_t) idx++].setBounds (cell.reduced (1));
        }
    }

    left.removeFromTop (pad);

    // TRIAD row
    auto triadRow = left.removeFromTop (buttonH);
    majorButton.setBounds (triadRow.removeFromLeft (triadRow.getWidth() / 2).reduced (1));
    minorButton.setBounds (triadRow.reduced (1));

    left.removeFromTop (pad);

    // M-Voice
    auto mRow = left.removeFromTop (buttonH);
    mVoiceButton.setBounds (mRow.reduced (1));

    left.removeFromTop (pad);

    // POSITION buttons in a column
    auto posArea = left.removeFromTop (buttonH * 5 + pad * 4);

    auto posRow = posArea;
    posRow.removeFromBottom (posArea.getHeight() - buttonH);

    posNoneButton.setBounds (posRow.reduced (1));
    posArea.removeFromTop (buttonH + pad);

    posRow = posArea.removeFromTop (buttonH);
    pos2SupButton.setBounds (posRow.reduced (1));

    posArea.removeFromTop (pad);
    posRow = posArea.removeFromTop (buttonH);
    pos1SupButton.setBounds (posRow.reduced (1));

    posArea.removeFromTop (pad);
    posRow = posArea.removeFromTop (buttonH);
    pos1InfButton.setBounds (posRow.reduced (1));

    posArea.removeFromTop (pad);
    posRow = posArea.removeFromTop (buttonH);
    pos2InfButton.setBounds (posRow.reduced (1));

    // RIGHT: piano + toggles + delay row
    auto pianoArea = right.removeFromTop (right.getHeight() - 60);
    pianoView.setBounds (pianoArea.reduced (pad));

    // toggles under piano
    auto toggleRow = right.removeFromTop (smallH);
    auto boxSize   = smallH;

    auto tBox = toggleRow.removeFromLeft (boxSize);
    tHighlightButton.setBounds (tBox.reduced (2));

    toggleRow.removeFromLeft (pad);

    auto orbitBox = toggleRow.removeFromLeft (boxSize * 2);
    orbitToggleButton.setBounds (orbitBox.reduced (2));

    toggleRow.removeFromLeft (pad);

    auto mBox = toggleRow.removeFromLeft (boxSize);
    mHighlightButton.setBounds (mBox.reduced (2));

    toggleRow.removeFromLeft (pad);

    auto randBox = toggleRow.removeFromLeft (boxSize * 2);
    randomButton.setBounds (randBox.reduced (2));

    right.removeFromTop (pad);

    // delay row
    auto delayRow = right.removeFromTop (buttonH);

    nowButton.setBounds (delayRow.removeFromLeft (60).reduced (1));
    delayRow.removeFromLeft (pad);

    auto syncTotalWidth = delayRow.getWidth() - 140; // leave room for Free + ms
    auto syncCellWidth  = syncTotalWidth / (int) syncButtons.size();

    for (int i = 0; i < numSyncButtons; ++i)
    {
        auto cell = delayRow.removeFromLeft (syncCellWidth);
        syncButtons[(size_t) i].setBounds (cell.reduced (1));
    }

    auto freeWidth = 60;
    freeButton.setBounds (delayRow.removeFromLeft (freeWidth).reduced (1));
    delayRow.removeFromLeft (pad);

    msSlider.setBounds (delayRow.reduced (1));
}
