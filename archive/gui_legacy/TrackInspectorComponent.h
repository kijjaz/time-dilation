#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class TrackInspectorComponent : public juce::Component
{
public:
    explicit TrackInspectorComponent (TimeDilationEngine& engine);
    ~TrackInspectorComponent() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setSelectedTrack (int trackIndex);

private:
    TimeDilationEngine& engine;
    int currentTrackIndex = 0;

    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    juce::Slider panSlider;
    juce::Label panLabel;

    juce::Slider gammaSlider;
    juce::Label gammaLabel;

    juce::ComboBox warpModeBox;
    juce::Label warpModeLabel;

    juce::ToggleButton loopToggle { "LOOP TIMELINE" };
    juce::Slider loopStartSlider;
    juce::Slider loopEndSlider;
    juce::Label loopLabel;

    juce::TextButton muteButton { "MUTE" };
    juce::TextButton soloButton { "SOLO" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackInspectorComponent)
};

} // namespace time_dilation
