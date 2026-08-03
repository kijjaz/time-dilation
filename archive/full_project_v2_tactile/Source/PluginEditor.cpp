#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace time_dilation
{

TimeDilationAudioProcessorEditor::TimeDilationAudioProcessorEditor (TimeDilationAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      studioComponent (p.getEngine())
{
    auto appDir = juce::File::getCurrentWorkingDirectory();
    FontManager::getInstance().loadFonts (appDir.getChildFile ("Source").getChildFile ("assets").getChildFile ("fonts"));

    juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

    addAndMakeVisible (studioComponent);

    setResizable (true, true);
    setResizeLimits (900, 600, 3840, 2160);
    setSize (1180, 740);
}

TimeDilationAudioProcessorEditor::~TimeDilationAudioProcessorEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}

void TimeDilationAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0e17));
}

void TimeDilationAudioProcessorEditor::resized()
{
    studioComponent.setBounds (0, 0, getWidth(), getHeight());
}

} // namespace time_dilation
