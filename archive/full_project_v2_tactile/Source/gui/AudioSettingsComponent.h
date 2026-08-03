#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class AudioSettingsComponent : public juce::Component
{
public:
    explicit AudioSettingsComponent (juce::AudioDeviceManager& deviceManager);
    ~AudioSettingsComponent() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioDeviceSelectorComponent selectorComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSettingsComponent)
};

} // namespace time_dilation
