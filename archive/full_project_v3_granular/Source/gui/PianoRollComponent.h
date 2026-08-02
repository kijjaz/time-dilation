#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

struct PianoRollNote
{
    int noteNumber = 60; // 0-127 MIDI note
    float startBeat = 0.0f;
    float durationBeats = 1.0f;
    float velocity = 0.8f;
};

class PianoRollComponent : public juce::Component
{
public:
    explicit PianoRollComponent (TimeDilationEngine& engine);
    ~PianoRollComponent() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

    void setSelectedTrack (int trackIndex);

private:
    TimeDilationEngine& engine;
    int selectedTrackIndex = 0;
    float pixelsPerBeat = 40.0f;
    float keyHeight = 12.0f;

    std::vector<PianoRollNote> notes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PianoRollComponent)
};

} // namespace time_dilation
