#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class GammaScriptEditorComponent : public juce::Component
{
public:
    explicit GammaScriptEditorComponent (TimeDilationEngine& engine);
    ~GammaScriptEditorComponent() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setSelectedTrack (int trackIndex);

private:
    TimeDilationEngine& engine;
    int currentTrackIndex = 0;

    juce::TextEditor scriptEditor;
    juce::TextButton applyButton { "APPLY SCRIPT" };
    juce::ComboBox presetBox;

    void loadPresets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GammaScriptEditorComponent)
};

} // namespace time_dilation
