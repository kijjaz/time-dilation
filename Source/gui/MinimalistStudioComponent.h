#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class MinimalistStudioComponent : public juce::Component, public juce::Timer
{
public:
    explicit MinimalistStudioComponent (TimeDilationEngine& engine);
    ~MinimalistStudioComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void mouseDown (const juce::MouseEvent& e) override;

private:
    TimeDilationEngine& engine;
    int selectedTrackIdx = 0;

    // Device Rack Knobs for Selected Track
    juce::Slider attackSlider;
    juce::Label attackLabel;

    juce::Slider releaseSlider;
    juce::Label releaseLabel;

    juce::Slider cutoffSlider;
    juce::Label cutoffLabel;

    juce::Slider gammaSlider;
    juce::Label gammaLabel;

    juce::TextButton warpModeButton { "MODE: VARISPEED TAPE" };
    juce::TextButton auditionButton { "PLAY AUDITION NOTE" };

    juce::TextButton addTrackButton { "+ ADD TRACK" };
    juce::TextButton addSubTrackButton { "+ ADD SUB-TRACK" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MinimalistStudioComponent)
};

} // namespace time_dilation
