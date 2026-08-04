#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/RelativisticTimeline.h"

namespace time_dilation
{

class TimelineEditorComponent : public juce::Component,
                                public juce::Timer
{
public:
    TimelineEditorComponent (TimelineNode& timelineNode);
    ~TimelineEditorComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

    std::function<void()> onOpenPoolRequested;

private:
    TimelineNode& node;

    juce::TextButton addAudioTrackBtn { "+ Audio Track" };
    juce::TextButton addMidiTrackBtn  { "+ MIDI Track" };
    juce::TextButton addDilTrackBtn   { "+ Time Dilation Track" };
    juce::TextButton openPoolBtn      { "MEDIA POOL" };
    juce::ToggleButton recordArmToggle { "REC ARM" };
    juce::Slider bpmSlider;
    juce::Label bpmLabel { {}, "BPM" };

    float zoomFactor = 20.0f; // pixels per beat
    float trackHeight = 64.0f;
    float headerWidth = 140.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineEditorComponent)
};

} // namespace time_dilation
