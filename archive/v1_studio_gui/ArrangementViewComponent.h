#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class ArrangementViewComponent : public juce::Component,
                                 public juce::Timer,
                                 public juce::FileDragAndDropTarget
{
public:
    explicit ArrangementViewComponent (TimeDilationEngine& engine);
    ~ArrangementViewComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Mouse Interaction Handlers
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    // File Drag and Drop Interface
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    void setZoomLevel (float pixelsPerBeat);
    float getZoomLevel() const { return pixelsPerBeat; }

    int getSelectedTrackIndex() const { return selectedTrackIndex; }
    void setSelectedTrackIndex (int index);

    std::function<void(int)> onTrackSelected;

private:
    TimeDilationEngine& engine;
    float pixelsPerBeat = 40.0f; // Zoom factor
    int selectedTrackIndex = 0;
    bool isScrubbingPlayhead = false;

    juce::TextButton addTrackButton { "+ ADD TRACK" };
    juce::TextButton addSubTrackButton { "+ ADD SUB-TRACK" };
    juce::TextButton deleteTrackButton { "DELETE TRACK" };

    void drawTimeRuler (juce::Graphics& g, float width, float height);
    void drawTrackLanes (juce::Graphics& g, float width, float height);
    void drawPlayhead (juce::Graphics& g, float width, float height);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrangementViewComponent)
};

} // namespace time_dilation
