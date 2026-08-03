#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class SpacetimeVisualizerComponent : public juce::Component, public juce::Timer
{
public:
    explicit SpacetimeVisualizerComponent (TimeDilationEngine& engine);
    ~SpacetimeVisualizerComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    TimeDilationEngine& engine;
    std::vector<float> audioData;
    float phase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpacetimeVisualizerComponent)
};

} // namespace time_dilation
