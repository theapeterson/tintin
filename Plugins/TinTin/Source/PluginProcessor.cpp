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
    using namespace Tintinnabuli;

    auto& c = tintin.context;

    c.rootNote = params.rootNote->get();
    c.triad    = (params.triadType->getIndex() == 0
                     ? TriadType::Major
                     : TriadType::Minor);

    switch (params.modeSelect->getIndex())
    {
        case 0: c.mode = TMode::Plus1;  break;
        case 1: c.mode = TMode::Plus2;  break;
        case 2: c.mode = TMode::Minus1; break;
        case 3: c.mode = TMode::Minus2; break;
        case 4: c.mode = TMode::Orbit;  break;
        default: c.mode = TMode::Plus1; break;
    }

    c.octaveOffset = params.octaveOffset->get();

    // 0 = Follow, 1 = Scaled, 2 = Fixed – matches your UI
    switch (params.velocityMode->getIndex())
    {
        case 0: c.velocityMode = VelocityMode::Follow; break;
        case 1: c.velocityMode = VelocityMode::Scaled; break;
        case 2: c.velocityMode = VelocityMode::Fixed;  break;
        default: c.velocityMode = VelocityMode::Follow; break;
    }

    // TODO: if you add UI controls for scale/fixed values, set them here
    c.velocityScale = 0.7f;
    c.fixedVelocity = 90;
}

void TinTinProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    updateOptions();

    // 1) Transform M-voice MIDI → T-voice MIDI
    // tintin.process (midiMessages);

    // 2) Render piano from transformed MIDI
    piano.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
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
