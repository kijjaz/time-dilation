#include "RelativisticLabComponent.h"

namespace time_dilation
{

RelativisticLabComponent::RelativisticLabComponent (TimeDilationEngine& e)
    : engine (e), visualizerComponent (e), scriptEditorComponent (e)
{
    addAndMakeVisible (visualizerComponent);
    addAndMakeVisible (scriptEditorComponent);
}

void RelativisticLabComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0f172a));
}

void RelativisticLabComponent::resized()
{
    const int vizWidth = 320;
    visualizerComponent.setBounds (5, 5, vizWidth, getHeight() - 10);
    scriptEditorComponent.setBounds (vizWidth + 10, 5, getWidth() - vizWidth - 15, getHeight() - 10);
}

} // namespace time_dilation
