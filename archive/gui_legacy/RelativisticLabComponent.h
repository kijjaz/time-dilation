#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"
#include "SpacetimeVisualizerComponent.h"
#include "GammaScriptEditorComponent.h"

namespace time_dilation
{

class RelativisticLabComponent : public juce::Component
{
public:
    explicit RelativisticLabComponent (TimeDilationEngine& engine);
    ~RelativisticLabComponent() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setSelectedTrack (int trackIndex)
    {
        scriptEditorComponent.setSelectedTrack (trackIndex);
    }

private:
    TimeDilationEngine& engine;
    SpacetimeVisualizerComponent visualizerComponent;
    GammaScriptEditorComponent scriptEditorComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativisticLabComponent)
};

} // namespace time_dilation
