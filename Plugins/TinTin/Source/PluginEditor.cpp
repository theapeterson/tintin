//Plugins/Tintin/Source/Tintin/PluginEditor.cpp
#include "PluginEditor.h"
#include "PluginProcessor.h"

TinTinProcessorEditor::TinTinProcessorEditor(TinTinProcessor& p)
    : AudioProcessorEditor(&p),
      genericEditor(p)
{
    addAndMakeVisible(genericEditor);
    setSize(400, 300);
}

void TinTinProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel()
        .findColour(juce::ResizableWindow::backgroundColourId));
}

void TinTinProcessorEditor::resized()
{
    genericEditor.setBounds(getLocalBounds());
}