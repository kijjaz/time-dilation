#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class TactileAudioStudioComponent : public juce::Component,
                                     public juce::FileDragAndDropTarget,
                                     public juce::Timer
{
public:
    explicit TactileAudioStudioComponent (TimeDilationEngine& engine);
    ~TactileAudioStudioComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // File Drag & Drop Target
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

private:
    TimeDilationEngine& engine;
    int selectedTrackIdx = 0;

    // Top Header Transport Controls
    juce::TextButton playButton { "PLAY" };
    juce::TextButton pauseButton { "PAUSE" };
    juce::TextButton stopButton { "STOP" };
    juce::TextButton bounceButton { "BOUNCE WAV TO DESKTOP" };

    juce::Slider masterGammaSlider;
    juce::Label masterGammaLabel;

    juce::TextButton addTrackButton { "+ ADD TRACK" };

    // Per-Track UI Control Holders
    struct TrackControlRow
    {
        std::unique_ptr<juce::Slider> gammaSlider;
        std::unique_ptr<juce::Slider> volSlider;
        std::unique_ptr<juce::TextButton> loopBtn;
        std::unique_ptr<juce::TextButton> muteBtn;
        std::unique_ptr<juce::TextButton> soloBtn;
        std::unique_ptr<juce::TextButton> warpBtn;
    };
    std::vector<TrackControlRow> trackRows;

    void rebuildTrackUI();
    void drawWaveformForBuffer (juce::Graphics& g, const juce::AudioBuffer<float>& buffer, juce::Rectangle<float> bounds, juce::Colour color);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TactileAudioStudioComponent)
};

} // namespace time_dilation
