// Plugins/TinTin/Source/PluginEditor.cpp
#include "PluginEditor.h"
#include "PluginProcessor.h"

TintinLookAndFeel::TintinLookAndFeel()
{
    using namespace juce;

    setColour (ResizableWindow::backgroundColourId, Colour (0xff303030));
    setColour (TextButton::buttonColourId,          Colour (0xff404040));
    setColour (TextButton::buttonOnColourId,        Colour (0xfff3a623)); // gold
    setColour (Label::textColourId,                 Colours::goldenrod);
}

void TintinLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                              juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool isMouseOver,
                                              bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat();

    auto isOn  = button.getToggleState();
    auto base  = isOn ? findColour (juce::TextButton::buttonOnColourId)
                      : backgroundColour;
    auto border = juce::Colours::black;
    auto hover  = base.brighter (0.1f);
    auto colour = (isMouseOver || isButtonDown) ? hover : base;

    g.setColour (colour);
    g.fillRoundedRectangle (bounds, 3.0f);

    g.setColour (border);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);
}

void TintinLookAndFeel::drawButtonText (juce::Graphics& g,
                                        juce::TextButton& button,
                                        bool isMouseOver,
                                        bool isButtonDown)
{
    juce::ignoreUnused (isMouseOver, isButtonDown);

    auto bounds = button.getLocalBounds();
    auto text   = button.getButtonText();

    auto colour = button.isEnabled()
                      ? juce::Colours::black
                      : juce::Colours::darkgrey;

    g.setColour (colour);
    g.setFont (juce::Font (14.0f));
    g.drawFittedText (text, bounds, juce::Justification::centred, 1);
}


TintinPianoView::TintinPianoView()
{
    staticTNotes.fill (false);
    mVoiceActive.fill (false);
    tVoiceActive.fill (false);

    setInterceptsMouseClicks (true, false);
}

void TintinPianoView::setStaticTNotes (const std::vector<int>& midiNotes)
{
    staticTNotes.fill (false);

    for (auto n : midiNotes)
    {
        if (n >= 0 && n < 128)
            staticTNotes[(size_t) n] = true;
    }

    repaint();
}

void TintinPianoView::setMVoiceState (const std::array<bool, 128>& state)
{
    mVoiceActive = state;
    repaint();
}

void TintinPianoView::setTVoiceState (const std::array<bool, 128>& state)
{
    tVoiceActive = state;
    repaint();
}

void TintinPianoView::noteOnM (int midi)
{
    if (midi < 0 || midi >= 128)
        return;

    mVoiceActive[(size_t) midi] = true;
    repaint();
}

void TintinPianoView::noteOffM (int midi)
{
    if (midi < 0 || midi >= 128)
        return;

    mVoiceActive[(size_t) midi] = false;
    repaint();
}

void TintinPianoView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    paintBasePiano (g, bounds);
    paintHighlights (g, bounds);
}

void TintinPianoView::mouseDown (const juce::MouseEvent& e)
{
    auto midi = midiFromPosition ((float) e.position.x);
    if (midi < 0)
        return;

    noteOnM (midi); // blue highlight immediately

    if (noteCallback)
        noteCallback (midi, true);
}

void TintinPianoView::mouseUp (const juce::MouseEvent& e)
{
    auto midi = midiFromPosition ((float) e.position.x);
    if (midi < 0)
        return;

    noteOffM (midi);

    if (noteCallback)
        noteCallback (midi, false);
}

void TintinPianoView::paintBasePiano (juce::Graphics& g,
                                      const juce::Rectangle<float>& bounds) const
{
    using namespace juce;

    g.setColour (Colours::white);
    g.fillRect (bounds);

    g.setColour (Colours::black.withAlpha (0.4f));
    g.drawRect (bounds, 1.0f);

    auto keyW = bounds.getWidth() / (float) numWhiteKeys;
    auto keyH = bounds.getHeight();

    for (int i = 0; i < numWhiteKeys; ++i)
    {
        auto x = bounds.getX() + keyW * (float) i;
        auto r = Rectangle<float> (x, bounds.getY(), keyW, keyH);

        g.setColour (Colours::white);
        g.fillRect (r);

        g.setColour (Colours::black.withAlpha (0.3f));
        g.drawRect (r, 0.5f);
    }

    auto drawBlack = [&] (int whiteIndex)
    {
        auto x = bounds.getX() + keyW * ((float) whiteIndex + 0.7f);
        auto r = Rectangle<float> (x, bounds.getY(), keyW * 0.6f, keyH * 0.6f);

        g.setColour (Colours::black);
        g.fillRect (r);
    };

    // C D E F G A B C D E F G A B pattern
    int pattern[] = { 0, 1, 3, 4, 5, 7, 8, 10, 11 };

    for (int whiteIndex : pattern)
    {
        if (whiteIndex < numWhiteKeys - 1)
            drawBlack (whiteIndex);
    }
}

void TintinPianoView::paintHighlights (juce::Graphics& g,
                                       const juce::Rectangle<float>& bounds) const
{
    using namespace juce;

    auto keyW = bounds.getWidth() / (float) numWhiteKeys;
    auto keyH = bounds.getHeight();

    // Top: static T-grid (gold)
    for (int midi = 0; midi < 128; ++midi)
    {
        if (! staticTNotes[(size_t) midi])
            continue;

        auto whiteIndex = midiToWhiteIndex (midi);

        drawStripe (g, bounds, keyW, keyH,
                    whiteIndex,
                    0.18f,
                    0.72f,
                    Colours::goldenrod.withAlpha (0.8f));
    }

    // Bottom: M voice (blue)
    for (int midi = 0; midi < 128; ++midi)
    {
        if (! mVoiceActive[(size_t) midi])
            continue;

        auto whiteIndex = midiToWhiteIndex (midi);

        drawStripe (g, bounds, keyW, keyH,
                    whiteIndex,
                    0.18f,
                    0.08f,
                    Colours::steelblue.withAlpha (0.9f));
    }

    // Middle: T voice output (red, thicker)
    for (int midi = 0; midi < 128; ++midi)
    {
        if (! tVoiceActive[(size_t) midi])
            continue;

        auto whiteIndex = midiToWhiteIndex (midi);

        drawStripe (g, bounds, keyW, keyH,
                    whiteIndex,
                    0.30f,
                    0.35f,
                    Colours::red.withAlpha (0.85f));
    }
}

int TintinPianoView::midiToWhiteIndex (int midi) const
{
    if (midi < baseMidi)
        return -1;

    if (midi >= baseMidi + numSemitones)
        return -1;

    auto offset = midi - baseMidi; // 0..23
    auto octave = offset / 12;     // 0 or 1
    auto pc     = offset % 12;     // 0..11

    static const int whitePcs[whitesPerOctave] = { 0, 2, 4, 5, 7, 9, 11 };

    int idxInOct = -1;

    for (int i = 0; i < whitesPerOctave; ++i)
    {
        if (whitePcs[i] == pc)
        {
            idxInOct = i;
            break;
        }
    }

    if (idxInOct < 0)
        return -1;

    auto idx = octave * whitesPerOctave + idxInOct;

    if (idx < 0 || idx >= numWhiteKeys)
        return -1;

    return idx;
}

void TintinPianoView::drawStripe (juce::Graphics& g,
                                  const juce::Rectangle<float>& bounds,
                                  float keyW,
                                  float keyH,
                                  int whiteIndex,
                                  float heightFraction,
                                  float yOffsetFraction,
                                  const juce::Colour& colour) const
{
    if (whiteIndex < 0 || whiteIndex >= numWhiteKeys)
        return;

    auto x = bounds.getX() + keyW * (float) whiteIndex;

    auto keyRect = juce::Rectangle<float> (x,
                                           bounds.getY(),
                                           keyW,
                                           keyH);

    auto stripeH = keyH * heightFraction;
    auto stripeY = keyRect.getY() + keyH * yOffsetFraction;

    auto stripeRect = juce::Rectangle<float> (keyRect.getX(),
                                              stripeY,
                                              keyRect.getWidth(),
                                              stripeH);

    g.setColour (colour);
    g.fillRect (stripeRect.reduced (1.0f));
}

int TintinPianoView::midiFromPosition (float x) const
{
    auto bounds = getLocalBounds().toFloat();

    auto keyW = bounds.getWidth() / (float) numWhiteKeys;
    if (keyW <= 0.0f)
        return -1;

    auto relX     = x - bounds.getX();
    auto rawIndex = (int) (relX / keyW);

    auto whiteIndex = juce::jlimit (0, numWhiteKeys - 1, rawIndex);

    static const int whiteMidi[numWhiteKeys] =
    {
        60, 62, 64, 65, 67, 69, 71,
        72, 74, 76, 77, 79, 81, 83
    };

    return whiteMidi[whiteIndex];
}


int TinTinProcessorEditor::midiRootFromIndex (int index)
{
    static const int roots[12] =
    {
        57, 58, 59, 60, 61, 62,
        63, 64, 65, 66, 67, 68
    };

    if (index < 0)
        return roots[0];

    if (index > 11)
        return roots[11];

    return roots[index];
}

int TinTinProcessorEditor::modeIndexFromPositionButton (int index)
{
    switch (index)
    {
        case 0: return 0; // None
        case 1: return 2; // 2nd Superior -> T+2
        case 2: return 1; // 1st Superior -> T+1
        case 3: return 3; // 1st Inferior -> T-1
        case 4: return 4; // 2nd Inferior -> T-2
        default: return 0;
    }
}

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


TinTinProcessorEditor::TinTinProcessorEditor (TinTinProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      lf()
{
    setLookAndFeel (&lf);

    addAndMakeVisible (pianoView);

    pianoView.setNoteCallback ([this] (int midi, bool isDown)
    {
        handlePianoNote (midi, isDown);
    });

    addAndMakeVisible (titleLabel);
    titleLabel.setText ("Tintinnabulator v1.3", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (juce::Font (26.0f, juce::Font::bold));

    initRootButtons();
    initTriadButtons();
    initMVoiceButton();
    initPositionButtons();
    initPianoToggles();
    initDelayButtons();

    msSlider.setRange (10.0, 2000.0, 1.0);
    msSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 18);
    addAndMakeVisible (msSlider);

    msSlider.onValueChange = [this]()
    {
        auto value = (float) msSlider.getValue();
        processor.getParams().displacementMs->setValueNotifyingHost (value);
    };

    setSize (800, 320);

    syncFromParams();
    syncPianoFromProcessor();

    startTimerHz (30);
}

TinTinProcessorEditor::~TinTinProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void TinTinProcessorEditor::handlePianoNote (int midiNote, bool isDown)
{
    auto& state = processor.getPreviewKeyboardState();

    constexpr int   channel  = 1;
    constexpr float velocity = 0.8f;

    if (isDown)
        state.noteOn  (channel, midiNote, velocity);
    else
        state.noteOff (channel, midiNote, velocity);
}



void TinTinProcessorEditor::initRootButtons()
{
    static const char* names[12] =
    {
        "A", "Bb", "B",
        "C", "Db", "D",
        "Eb", "E", "F",
        "F#", "G", "G#"
    };

    for (int i = 0; i < (int) rootButtons.size(); ++i)
    {
        auto& button = rootButtons[(size_t) i];

        button.setButtonText (names[i]);
        button.setClickingTogglesState (true);
        addAndMakeVisible (button);

        button.onClick = [this, i]()
        {
            selectRoot (i);
        };
    }
}

void TinTinProcessorEditor::initTriadButtons()
{
    majorButton.setClickingTogglesState (true);
    minorButton.setClickingTogglesState (true);

    addAndMakeVisible (majorButton);
    addAndMakeVisible (minorButton);

    majorButton.onClick = [this]() { selectTriad (true); };
    minorButton.onClick = [this]() { selectTriad (false); };
}

void TinTinProcessorEditor::initMVoiceButton()
{
    mVoiceButton.setClickingTogglesState (true);
    addAndMakeVisible (mVoiceButton);

    mVoiceButton.onClick = [this]()
    {
        setMVoice (mVoiceButton.getToggleState());
    };
}

void TinTinProcessorEditor::initPositionButtons()
{
    auto init = [this] (juce::TextButton& button, int index)
    {
        button.setClickingTogglesState (true);
        addAndMakeVisible (button);

        button.onClick = [this, index]()
        {
            selectPosition (index);
        };
    };

    init (posNoneButton, 0);
    init (pos2SupButton, 1);
    init (pos1SupButton, 2);
    init (pos1InfButton, 3);
    init (pos2InfButton, 4);
}

void TinTinProcessorEditor::initPianoToggles()
{
    tHighlightButton.setClickingTogglesState (true);
    orbitToggleButton.setClickingTogglesState (true);
    mHighlightButton.setClickingTogglesState (true);
    randomButton.setClickingTogglesState (false);

    addAndMakeVisible (tHighlightButton);
    addAndMakeVisible (orbitToggleButton);
    addAndMakeVisible (mHighlightButton);
    addAndMakeVisible (randomButton);

    orbitToggleButton.onClick = [this]()
    {
        auto& params = processor.getParams();

        if (orbitToggleButton.getToggleState())
            params.modeSelect->setValueNotifyingHost (5); // Orbit

        updatePositionButtons();
    };

    randomButton.onClick = []()
    {
        // Randomizer left as a future feature.
    };
}

void TinTinProcessorEditor::initDelayButtons()
{
    nowButton.setClickingTogglesState (true);
    addAndMakeVisible (nowButton);

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
        auto& button = syncButtons[(size_t) i];

        button.setButtonText (syncNames[i]);
        button.setClickingTogglesState (true);
        addAndMakeVisible (button);

        button.onClick = [this, i]()
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
}

void TinTinProcessorEditor::timerCallback()
{
    syncPianoFromProcessor();
}

void TinTinProcessorEditor::syncPianoFromProcessor()
{
    const auto& h = processor.getPianoHighlightState();

    std::vector<int> notes;
    notes.reserve (32);

    for (int n = 0; n < 128; ++n)
    {
        if (h.staticT[(size_t) n])
            notes.push_back (n);
    }

    pianoView.setStaticTNotes (notes);
    pianoView.setMVoiceState   (h.inputM);
    pianoView.setTVoiceState   (h.outputT);
}


void TinTinProcessorEditor::syncFromParams()
{
    auto& params = processor.getParams();

    {
        const int midi = params.rootNote->get();
        rootIndexUI = 0;

        for (int i = 0; i < 12; ++i)
        {
            if (midiRootFromIndex (i) == midi)
            {
                rootIndexUI = i;
                break;
            }
        }

        updateRootButtons();
    }

    {
        auto triadIndex = params.triadType->getIndex(); // 0 = Major, 1 = Minor
        majorButton.setToggleState (triadIndex == 0, juce::dontSendNotification);
        minorButton.setToggleState (triadIndex == 1, juce::dontSendNotification);
    }

    updatePositionButtons();
    updateMVoiceButton();

    msSlider.setValue (params.displacementMs->get(), juce::dontSendNotification);
    updateDelayButtons();
}

void TinTinProcessorEditor::selectRoot (int index)
{
    rootIndexUI = juce::jlimit (0, 11, index);

    auto& params = processor.getParams();
    auto  midi   = midiRootFromIndex (rootIndexUI);

    params.rootNote->setValueNotifyingHost (midi);

    updateRootButtons();
}

void TinTinProcessorEditor::updateRootButtons()
{
    for (int i = 0; i < 12; ++i)
        rootButtons[(size_t) i].setToggleState (i == rootIndexUI,
                                                juce::dontSendNotification);
}

void TinTinProcessorEditor::selectTriad (bool isMajor)
{
    auto& params = processor.getParams();

    if (isMajor)
        params.triadType->setValueNotifyingHost (0);
    else
        params.triadType->setValueNotifyingHost (1);

    updateTriadButtons();
}

void TinTinProcessorEditor::selectPosition (int index)
{
    auto& params = processor.getParams();

    auto modeIndex = modeIndexFromPositionButton (index);
    params.modeSelect->setValueNotifyingHost (modeIndex);

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
    static const int syncMap[numSyncButtons] = { 12, 13, 7, 6, 4, 3, 1, 0 };

    if (buttonIndex < 0 || buttonIndex >= numSyncButtons)
        return;

    auto& params = processor.getParams();
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


void TinTinProcessorEditor::updateTriadButtons()
{
    auto triadIndex = processor.getParams().triadType->getIndex();

    majorButton.setToggleState (triadIndex == 0, juce::dontSendNotification);
    minorButton.setToggleState (triadIndex == 1, juce::dontSendNotification);
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

    orbitToggleButton.setToggleState (idx == 5, juce::dontSendNotification);
}

void TinTinProcessorEditor::updateMVoiceButton()
{
    auto on = processor.getParams().mVoiceOn->get();
    mVoiceButton.setToggleState (on, juce::dontSendNotification);
}

void TinTinProcessorEditor::updateDelayButtons()
{
    auto& params = processor.getParams();

    auto mode = params.displacementMode->getIndex(); // 0=None,1=Sync,2=Absolute
    auto sync = params.displacementSync->get();

    nowButton.setToggleState (mode == 0, juce::dontSendNotification);

    for (auto& button : syncButtons)
        button.setToggleState (false, juce::dontSendNotification);

    freeButton.setToggleState (mode == 2, juce::dontSendNotification);

    if (mode == 1)
    {
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

    auto left  = r.removeFromLeft (leftWidth);
    r.removeFromLeft (pad);
    auto right = r;

    // root grid 4x3
    {
        auto rootArea = left.removeFromTop (buttonH * 4 + pad * 3);
        auto areaCopy = rootArea;

        auto cellW = rootArea.getWidth() / 3;
        int  idx   = 0;

        for (int row = 0; row < 4; ++row)
        {
            auto rowArea = areaCopy.removeFromTop (buttonH);

            if (row < 3)
                areaCopy.removeFromTop (pad);

            for (int col = 0; col < 3; ++col)
            {
                auto cell = rowArea.removeFromLeft (cellW);
                rootButtons[(size_t) idx++].setBounds (cell.reduced (1));
            }
        }
    }

    left.removeFromTop (pad);

    {
        auto triadRow = left.removeFromTop (buttonH);
        auto leftHalf = triadRow.removeFromLeft (triadRow.getWidth() / 2);

        majorButton.setBounds (leftHalf.reduced (1));
        minorButton.setBounds (triadRow.reduced (1));
    }

    left.removeFromTop (pad);

    // M-voice button
    {
        auto mRow = left.removeFromTop (buttonH);
        mVoiceButton.setBounds (mRow.reduced (1));
    }

    left.removeFromTop (pad);

    // position buttons column
    {
        auto posArea = left.removeFromTop (buttonH * 5 + pad * 4);

        auto row = posArea.removeFromTop (buttonH);
        posNoneButton.setBounds (row.reduced (1));
        posArea.removeFromTop (pad);

        row = posArea.removeFromTop (buttonH);
        pos2SupButton.setBounds (row.reduced (1));
        posArea.removeFromTop (pad);

        row = posArea.removeFromTop (buttonH);
        pos1SupButton.setBounds (row.reduced (1));
        posArea.removeFromTop (pad);

        row = posArea.removeFromTop (buttonH);
        pos1InfButton.setBounds (row.reduced (1));
        posArea.removeFromTop (pad);

        row = posArea.removeFromTop (buttonH);
        pos2InfButton.setBounds (row.reduced (1));
    }

    // rigthside

    {
        auto pianoArea = right.removeFromTop (pianoHeight);
        pianoView.setBounds (pianoArea.reduced (pad));
    }

    right.removeFromTop (pad);

    // toggles under piano
    {
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
    }

    right.removeFromTop (pad);

    // delay row
    {
        auto delayRow = right.removeFromTop (buttonH);

        nowButton.setBounds (delayRow.removeFromLeft (60).reduced (1));
        delayRow.removeFromLeft (pad);

        auto syncTotalWidth = delayRow.getWidth() - 140;
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

    // putting controls above the piano?!
    for (auto& b : rootButtons) b.toFront (false);
    majorButton.toFront (false);
    minorButton.toFront (false);
    mVoiceButton.toFront (false);

    posNoneButton.toFront (false);
    pos2SupButton.toFront (false);
    pos1SupButton.toFront (false);
    pos1InfButton.toFront (false);
    pos2InfButton.toFront (false);

    tHighlightButton.toFront (false);
    orbitToggleButton.toFront (false);
    mHighlightButton.toFront (false);
    randomButton.toFront (false);

    nowButton.toFront (false);
    for (auto& b : syncButtons) b.toFront (false);
    freeButton.toFront (false);
    msSlider.toFront (false);
}
