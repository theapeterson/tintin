// Plugins/TinTin/Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

TinTinProcessor::TinTinProcessor()
{
    params.add (*this);

    // Synth setup
    piano.clearVoices();
    for (int i = 0; i < 8; ++i)
        piano.addVoice (new juce::SamplerVoice());

    formatManager.registerBasicFormats();
    loadPianoSound();
}

void TinTinProcessor::loadSample(const void* data,
                                 int dataSize,
                                 int rootMidiNote)
{
    auto input = std::make_unique<juce::MemoryInputStream>(data, dataSize, false);

    if (auto* reader = formatManager.createReaderFor(std::move(input)))
    {
        juce::BigInteger notes;
        notes.setRange(0, 128, true);

        auto* sound = new juce::SamplerSound(
            "IntimateSample",
            *reader,
            notes,
            rootMidiNote,
            0.0,   // attack
            0.2,   // release
            10.0   // max length
        );

        piano.addSound(sound);
    }
    else
    {
        DBG("Reader = nullptr (could not read sample!)");
    }
}

void TinTinProcessor::loadPianoSound()
{
    DBG("C4 size = " << BinaryData::C4_wavSize);
    DBG("Fs3 size = " << BinaryData::Fs4_wavSize);
    DBG("A2 size = " << BinaryData::A2_wavSize);

    loadSample(BinaryData::C4_wav,  BinaryData::C4_wavSize, 60); // C4
    loadSample(BinaryData::Fs4_wav, BinaryData::Fs4_wavSize, 54); // F#3
    loadSample(BinaryData::A2_wav,  BinaryData::A2_wavSize, 45); // A2
}

void TinTinProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    DBG("Voices: " << piano.getNumVoices());
    DBG("Sounds: " << piano.getNumSounds());

    juce::ignoreUnused (samplesPerBlock);
    piano.setCurrentPlaybackSampleRate (sampleRate);
    tintin.resetOrbit();
    updateOptions();
}

void TinTinProcessor::updateOptions()
{
    auto& c = tintin.settings;

    c.rootNote = params.rootNote->get();
    c.triad    = (params.triadType->getIndex() == 0
                      ? TintinSettings::TriadType::Major
                      : TintinSettings::TriadType::Minor);

    switch (params.modeSelect->getIndex())
    {
        case 0: c.mode = TintinSettings::TMode::Plus1;  break;
        case 1: c.mode = TintinSettings::TMode::Plus2;  break;
        case 2: c.mode = TintinSettings::TMode::Minus1; break;
        case 3: c.mode = TintinSettings::TMode::Minus2; break;
        case 4: c.mode = TintinSettings::TMode::Orbit;  break;
        default: c.mode = TintinSettings::TMode::Plus1; break;
    }

    c.octaveOffset = params.octaveOffset->get();

    switch (params.velocityMode->getIndex())
    {
        case 0: c.velocityMode = TintinSettings::VelocityMode::Follow; break;
        case 1: c.velocityMode = TintinSettings::VelocityMode::Scaled; break;
        case 2: c.velocityMode = TintinSettings::VelocityMode::Fixed;  break;
        default: c.velocityMode = TintinSettings::VelocityMode::Follow; break;
    }

    c.velocityScale = params.velocityScaleParam->get();
    c.fixedVelocity = params.fixedVelocityParam->get();

    switch (params.displacementMode->getIndex())
    {
        case 0: c.displacementMode = TintinSettings::DisplacementMode::None;     break;
        case 1: c.displacementMode = TintinSettings::DisplacementMode::Sync;     break;
        case 2: c.displacementMode = TintinSettings::DisplacementMode::Absolute; break;
        default: c.displacementMode = TintinSettings::DisplacementMode::None;    break;
    }

    c.syncIndex      = params.displacementSync->get();
    c.displacementMs = params.displacementMs->get();

    c.scaleIndex      = params.scaleSelect->getIndex();
    c.feedbackRepeats = 0;   // for future UI
    c.numTVoices      = 1;   // for future UI
}

void TinTinProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    updateOptions();

    // tempo for sync displacement
    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo pos;
        if (playHead->getCurrentPosition(pos) && pos.bpm > 0.0)
            bpm = pos.bpm;
    }
    tintin.settings.bpm = bpm;

    // process MIDI through tintin engine
    tintin.process(midiMessages, getSampleRate(), buffer.getNumSamples());

    // render from transformed MIDI
    piano.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}



juce::AudioProcessorEditor* TinTinProcessor::createEditor()
{
    return new TinTinProcessorEditor (*this);
}

void TinTinProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto paramsTree = PluginHelpers::saveParamsTree (*this);
    auto pluginPreset = juce::ValueTree (getName());
    pluginPreset.appendChild (paramsTree, nullptr);
    copyXmlToBinary (*pluginPreset.createXml(), destData);
}

void TinTinProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        auto preset     = juce::ValueTree::fromXml (*xml);
        auto paramsTree = preset.getChildWithName ("Params");
        PluginHelpers::loadParamsTree (*this, paramsTree);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TinTinProcessor();
}
