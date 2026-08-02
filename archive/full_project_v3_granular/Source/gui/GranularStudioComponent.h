#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"
#include "../dsp/GranularEngine.h"

namespace time_dilation
{

class GranularStudioComponent : public juce::Component,
                                 public juce::FileDragAndDropTarget,
                                 public juce::Timer
{
public:
    explicit GranularStudioComponent (GranularEngine& engine);
    ~GranularStudioComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // File Drag & Drop Target
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

private:
    GranularEngine& granularEngine;

    // Granular Rotary Control Dials
    juce::Slider grainSizeSlider;
    juce::Label grainSizeLabel;

    juce::Slider densitySlider;
    juce::Label densityLabel;

    juce::Slider pitchSlider;
    juce::Label pitchLabel;

    juce::Slider pitchJitterSlider;
    juce::Label pitchJitterLabel;

    juce::Slider spraySlider;
    juce::Label sprayLabel;

    juce::Slider panSpreadSlider;
    juce::Label panSpreadLabel;

    juce::TextButton playButton { "PLAY GRANULAR CLOUD" };
    juce::TextButton exportButton { "BOUNCE GRANULAR WAV TO DESKTOP" };

    void drawWaveform (juce::Graphics& g, juce::Rectangle<float> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularStudioComponent)
};

} // namespace time_dilation
