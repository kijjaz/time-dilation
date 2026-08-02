#include "AudioSettingsComponent.h"

namespace time_dilation
{

AudioSettingsComponent::AudioSettingsComponent (juce::AudioDeviceManager& dm)
    : selectorComponent (dm, 0, 2, 0, 2, true, true, true, false)
{
    addAndMakeVisible (selectorComponent);
    setSize (500, 400);
}

void AudioSettingsComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0f141d));
    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("AUDIO INTERFACE & SAMPLE RATE SETTINGS", 12, 10, 400, 24, juce::Justification::left);
}

void AudioSettingsComponent::resized()
{
    selectorComponent.setBounds (10, 40, getWidth() - 20, getHeight() - 50);
}

} // namespace time_dilation
