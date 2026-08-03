#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"
#include "ModularDataflowCanvas.h"

namespace time_dilation
{

class RelativisticStudioComponent : public juce::Component, public juce::Timer
{
public:
    explicit RelativisticStudioComponent (TimeDilationEngine& engine);
    ~RelativisticStudioComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    TimeDilationEngine& engine;
    ModularDataflowCanvas dataflowCanvas;

    juce::Slider masterGammaSlider;
    juce::Label masterGammaLabel;

    juce::TextButton auditionButton { "AUDITION SOUND" };
    juce::TextButton addSynthButton { "+ ADD SYNTH NODE" };
    juce::TextButton addMixerBusButton { "+ ADD MIXER BUS" };
    juce::TextButton addTrackButton { "+ ADD TRACK" };
    juce::TextButton addSubTrackButton { "+ ADD SUB-TRACK" };

    juce::TextButton warpModeButton { "MODE: VARISPEED TAPE" };
    int selectedTrackIdx = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativisticStudioComponent)
};

} // namespace time_dilation
