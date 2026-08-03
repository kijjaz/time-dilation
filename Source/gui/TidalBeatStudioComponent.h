#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TidalBeatEngine.h"

namespace time_dilation
{

class TidalBeatStudioComponent : public juce::Component,
                                  public juce::Timer
{
public:
    explicit TidalBeatStudioComponent (TidalBeatEngine& engine);
    ~TidalBeatStudioComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    TidalBeatEngine& beatEngine;

    juce::TextButton modeButton { "BEAT MODE (Tidal 2.0)" };
    juce::TextButton playButton { "STOP / PLAY" };

    juce::Slider bpmSlider;
    juce::Label bpmLabel;

    juce::TextButton motif1Button { "123 (Ascending)" };
    juce::TextButton motif2Button { "321 (Descending)" };
    juce::TextButton motif3Button { "123 321 (Alternating)" };
    juce::TextButton motif4Button { "Carnatic Var 7 (333 222 111 321)" };

    juce::TextButton exportButton { "BOUNCE CARNATIC BEAT WAV TO DESKTOP" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TidalBeatStudioComponent)
};

} // namespace time_dilation
