#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "gui/CarbonGoldLookAndFeel.h"
#include "gui/AudioSettingsComponent.h"
#include "gui/MinimalistStudioComponent.h"
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

    MinimalistStudioComponent studioComponent;

    // Transport Bar Controls
    juce::TextButton playButton { "PLAY" };
    juce::TextButton pauseButton { "PAUSE" };
    juce::TextButton stopButton { "STOP" };

    juce::Slider bpmSlider;
    juce::Label bpmLabel;

    juce::Slider masterGammaSlider;
    juce::Label masterGammaLabel;

    juce::TextButton audioSettingsButton { "AUDIO SETTINGS" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeDilationAudioProcessorEditor)
};

} // namespace time_dilation
