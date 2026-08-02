#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class TimelineSequencerComponent : public juce::Component,
                                    public juce::Timer,
                                    public juce::FileDragAndDropTarget
{
public:
    explicit TimelineSequencerComponent (TimeDilationEngine& engine);
    ~TimelineSequencerComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // File Drag and Drop Target Interface
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

private:
    TimeDilationEngine& engine;

    juce::TextButton playButton { "PLAY" };
    juce::TextButton pauseButton { "PAUSE" };
    juce::TextButton stopButton { "STOP" };

    juce::Slider bpmSlider;
    juce::Label bpmLabel;

    juce::Slider masterGammaSlider;
    juce::Label masterGammaLabel;

    // Virtual MIDI Keyboard
    juce::MidiKeyboardComponent keyboardComponent;

    struct TrackRowControls
    {
        juce::Slider gammaSlider;
        juce::Slider volumeSlider;
        juce::ComboBox warpModeBox;
        juce::TextButton muteButton { "M" };
        juce::TextButton soloButton { "S" };
        std::vector<std::unique_ptr<juce::TextButton>> stepButtons;
    };

    std::vector<std::unique_ptr<TrackRowControls>> rowControls;
    void rebuildTrackControls();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineSequencerComponent)
};

} // namespace time_dilation
