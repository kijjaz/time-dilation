#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

struct ClipItem
{
    juce::String id;
    juce::String name;
    int trackIndex = 0;
    float startBeat = 0.0f;
    float lengthBeats = 4.0f;
    float gamma = 1.0f;
    juce::Colour color;
    bool isAudio = false;
};

class TimelineClipComponent : public juce::Component
{
public:
    explicit TimelineClipComponent (TimeDilationEngine& engine);
    ~TimelineClipComponent() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    std::vector<ClipItem>& getClips() { return clips; }

private:
    TimeDilationEngine& engine;
    std::vector<ClipItem> clips;

    int draggedClipIdx = -1;
    bool isDraggingWarpHandle = false;
    bool isResizingEdge = false;
    juce::Point<float> dragStartPos;
    ClipItem clipStartSnapshot;

    float pixelsPerBeat = 25.0f;
    float trackRowHeight = 55.0f;

    void drawWaveform (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour color);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineClipComponent)
};

} // namespace time_dilation
