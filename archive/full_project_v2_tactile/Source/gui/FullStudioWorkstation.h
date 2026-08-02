#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"
#include "PianoRollComponent.h"

namespace time_dilation
{

class FullStudioWorkstation : public juce::Component, public juce::Timer
{
public:
    explicit FullStudioWorkstation (TimeDilationEngine& engine);
    ~FullStudioWorkstation() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    TimeDilationEngine& engine;
    PianoRollComponent pianoRollComponent;

    // Transport Bar Controls
    juce::TextButton playButton { "PLAY" };
    juce::TextButton pauseButton { "PAUSE" };
    juce::TextButton stopButton { "STOP" };
    juce::TextButton auditionButton { "AUDITION CHORD" };

    juce::Slider bpmSlider;
    juce::Label bpmLabel;

    juce::Slider masterGammaSlider;
    juce::Label masterGammaLabel;

    juce::TextButton addTrackButton { "+ ADD TRACK" };
    juce::TextButton addSubTrackButton { "+ ADD SUB-TRACK" };
    juce::TextButton warpModeButton { "MODE: VARISPEED TAPE" };

    int selectedTrackIdx = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FullStudioWorkstation)
};

} // namespace time_dilation
