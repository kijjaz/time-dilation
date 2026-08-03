#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class MixerConsoleComponent : public juce::Component, public juce::Timer
{
public:
    explicit MixerConsoleComponent (TimeDilationEngine& engine);
    ~MixerConsoleComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    TimeDilationEngine& engine;

    struct ChannelStripControls
    {
        juce::Slider volumeSlider;
        juce::Slider panSlider;
        juce::TextButton muteButton { "M" };
        juce::TextButton soloButton { "S" };
        juce::TextButton armButton { "R" };
        float peakLevel = 0.0f;
    };

    std::vector<std::unique_ptr<ChannelStripControls>> channelStrips;
    void rebuildStrips();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerConsoleComponent)
};

} // namespace time_dilation
