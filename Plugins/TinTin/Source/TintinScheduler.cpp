// Plugins/TinTin/Source/Tintin/TintinScheduler.cpp
#include "TintinScheduler.h"

void TintinScheduler::clear()
{
    queue.clear();
}

void TintinScheduler::add(const juce::MidiMessage& msg,
                          int delaySamples,
                          int baseSamplePos)
{
    Pending p;
    p.msg = msg;
    p.samplesRemaining = baseSamplePos + delaySamples;

    queue.emplace_back(p);
}

void TintinScheduler::processBlock(juce::MidiBuffer& out,
                                   int numSamples)
{
    for (auto it = queue.begin(); it != queue.end();)
    {
        if (it->samplesRemaining < numSamples)
        {
            auto pos = juce::jmax(0, it->samplesRemaining);
            out.addEvent(it->msg, pos);
            it = queue.erase(it);
            continue;
        }

        it->samplesRemaining -= numSamples;
        ++it;
    }
}
