// #include "PluginProcessor.h"
// #include "PluginEditor.h"
//
// NewPluginTemplateAudioProcessor::NewPluginTemplateAudioProcessor()
// {
//     parameters.add(*this);
// }
//
// void NewPluginTemplateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
//                                                    juce::MidiBuffer& midiMessages)
//
// {
//     buffer.clear();
//
//     mapper.process(midiMessages, randomTransposer);
// }
//
// juce::AudioProcessorEditor* NewPluginTemplateAudioProcessor::createEditor()
// {
//     return new NewPluginTemplateAudioProcessorEditor(*this);
// }
//
// void NewPluginTemplateAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
// {
//     //Serializes your parameters, and any other potential data into an XML:
//
//     auto params = PluginHelpers::saveParamsTree(*this);
//
//     auto pluginPreset = juce::ValueTree(getName());
//     pluginPreset.appendChild(params, nullptr);
//     //This a good place to add any non-parameters to your preset
//
//     copyXmlToBinary(*pluginPreset.createXml(), destData);
// }
//
// void NewPluginTemplateAudioProcessor::setStateInformation(const void* data,
//                                                           int sizeInBytes)
// {
//     //Loads your parameters, and any other potential data from an XML:
//
//     if (auto xml = getXmlFromBinary(data, sizeInBytes))
//     {
//         auto preset = juce::ValueTree::fromXml(*xml);
//         auto params = preset.getChildWithName("Params");
//
//         PluginHelpers::loadParamsTree(*this, params);
//
//         //Load your non-parameter data now
//     }
// }
//
// juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
// {
//     return new NewPluginTemplateAudioProcessor();
// }

#include "PluginProcessor.h"
#include "PluginEditor.h"

TinTinProcessor::TinTinProcessor()
{
    params.add(*this);
}

void TinTinProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    updateOptions();
}

void TinTinProcessor::updateOptions()
{
    using namespace EA::MIDI;

    tintin.context.rootNote = params.rootNote->get();
    tintin.context.triadType =
    (params.triadType->getIndex() == 0
         ? EA::MIDI::TriadType::Major
         : EA::MIDI::TriadType::Minor);
    switch (params.modeSelect->getIndex())
    {
        case 0:
            tintin.context.mode = EA::MIDI::TMode::Plus1;
            break;
        case 1:
            tintin.context.mode = EA::MIDI::TMode::Plus2;
            break;
        case 2:
            tintin.context.mode = EA::MIDI::TMode::Minus1;
            break;
        case 3:
            tintin.context.mode = EA::MIDI::TMode::Minus2;
            break;
        case 4:
            tintin.context.mode = EA::MIDI::TMode::Orbit;
            break;
    }

    tintin.context.octaveOffset = params.octaveOffset->get();
    tintin.context.velocityMode = params.velocityMode->getIndex();
}

void TinTinProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    updateOptions();
    mapper.process(midiMessages, tintin);
}

juce::AudioProcessorEditor* TinTinProcessor::createEditor()
{
    return new TinTinProcessorEditor(*this);
}

void TinTinProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto paramsTree = PluginHelpers::saveParamsTree(*this);

    auto pluginPreset = juce::ValueTree(getName());
    pluginPreset.appendChild(paramsTree, nullptr);

    copyXmlToBinary(*pluginPreset.createXml(), destData);
}

void TinTinProcessor::setStateInformation(const void* data,
                                          int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        auto preset = juce::ValueTree::fromXml(*xml);
        auto paramsTree = preset.getChildWithName("Params");

        PluginHelpers::loadParamsTree(*this, paramsTree);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TinTinProcessor();
}