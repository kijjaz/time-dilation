#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"
#include "TimelineClipComponent.h"

namespace time_dilation
{

class RelativisticDAWWorkstation : public juce::Component, public juce::Timer
{
public:
    explicit RelativisticDAWWorkstation (TimeDilationEngine& engine);
    ~RelativisticDAWWorkstation() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

private:
    TimeDilationEngine& engine;
    int selectedTrackIdx = 0;

    TimelineClipComponent timelineClips;

    // Transport Bar Controls
    juce::TextButton playButton { "PLAY" };
    juce::TextButton pauseButton { "PAUSE" };
    juce::TextButton stopButton { "STOP" };
    juce::TextButton auditionButton { "PLAY AUDITION CHORD" };

    juce::Slider bpmSlider;
    juce::Label bpmLabel;

    juce::Slider masterGammaSlider;
    juce::Label masterGammaLabel;

    juce::TextButton audioSettingsButton { "AUDIO SETTINGS" };

    // Sidebar Track Action Buttons
    juce::TextButton addTrackButton { "+ ADD TRACK" };
    juce::TextButton addSubTrackButton { "+ ADD SUB-TRACK" };

    // Device Rack Rotary Controls for Selected Track
    juce::Slider attackSlider;
    juce::Label attackLabel;

    juce::Slider releaseSlider;
    juce::Label releaseLabel;

    juce::Slider cutoffSlider;
    juce::Label cutoffLabel;

    juce::Slider trackGammaSlider;
    juce::Label trackGammaLabel;

    juce::Slider lfoSpeedSlider;
    juce::Label lfoSpeedLabel;

    juce::TextButton warpModeButton { "MODE: VARISPEED TAPE" };

    // Per-Track Controls UI Holders
    struct TrackUIComponents
    {
        std::unique_ptr<juce::Slider> volSlider;
        std::unique_ptr<juce::Slider> panSlider;
        std::unique_ptr<juce::Slider> gammaSlider;
        std::unique_ptr<juce::TextButton> muteBtn;
        std::unique_ptr<juce::TextButton> soloBtn;
        std::unique_ptr<juce::TextButton> warpBtn;
    };
    std::vector<TrackUIComponents> trackUIs;

    void rebuildTrackUI();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativisticDAWWorkstation)
};

} // namespace time_dilation
