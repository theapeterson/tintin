#pragma once

#include "PluginProcessor.h"

class TinTinProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit TinTinProcessorEditor(TinTinProcessor&);
private:
    void paint(juce::Graphics&) override;
    void resized() override;

    // juce::GenericAudioProcessorEditor genericEditor {processor};
    juce::GenericAudioProcessorEditor genericEditor;
};
