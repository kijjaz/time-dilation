#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace time_dilation
{

TimeDilationAudioProcessor::TimeDilationAudioProcessor()
    : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

TimeDilationAudioProcessor::~TimeDilationAudioProcessor()
{
}

void TimeDilationAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepareToPlay (sampleRate, samplesPerBlock);
}

void TimeDilationAudioProcessor::releaseResources()
{
    engine.releaseResources();
}

bool TimeDilationAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void TimeDilationAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    engine.processBlock (buffer, midiMessages);
}

juce::AudioProcessorEditor* TimeDilationAudioProcessor::createEditor()
{
    return new TimeDilationAudioProcessorEditor (*this);
}

void TimeDilationAudioProcessor::getStateInformation (juce::MemoryBlock& /*destData*/)
{
}

void TimeDilationAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
}

} // namespace time_dilation

// Create JUCE Plugin Instance
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new time_dilation::TimeDilationAudioProcessor();
}
