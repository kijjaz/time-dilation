#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "gui/CarbonGoldLookAndFeel.h"
#include "gui/TactileAudioStudioComponent.h"
#include "gui/FontManager.h"

namespace time_dilation
{

class TimeDilationAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit TimeDilationAudioProcessorEditor (TimeDilationAudioProcessor&);
    ~TimeDilationAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TimeDilationAudioProcessor& audioProcessor;
    CarbonGoldLookAndFeel lookAndFeel;

    TactileAudioStudioComponent studioComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeDilationAudioProcessorEditor)
};

} // namespace time_dilation
