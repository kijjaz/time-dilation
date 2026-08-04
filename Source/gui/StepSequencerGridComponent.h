#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/RelativisticSequencers.h"

namespace time_dilation
{

enum class SequencerGridType
{
    Drum,
    Step
};

class StepSequencerGridComponent : public juce::Component,
                                   public juce::Timer
{
public:
    explicit StepSequencerGridComponent (std::shared_ptr<DrumSequencerNode> drumNode);
    explicit StepSequencerGridComponent (std::shared_ptr<StepSequencerNode> stepNode);
    ~StepSequencerGridComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    void timerCallback() override;

private:
    SequencerGridType gridType = SequencerGridType::Drum;
    std::shared_ptr<DrumSequencerNode> drumNode;
    std::shared_ptr<StepSequencerNode> stepNode;

    // Header controls
    juce::TextButton presetTrapBtn { "TRAP 808" };
    juce::TextButton presetJerseyBtn { "JERSEY BOUNCE" };
    juce::TextButton presetGlitchBtn { "GLITCH ROLL" };
    juce::TextButton presetPolyBtn { "POLY RHYTHM" };
    juce::TextButton clearBtn { "CLEAR" };

    juce::ComboBox stepCountBox;
    juce::ComboBox timeSigBox;
    juce::Slider bpmSlider;
    juce::Label bpmLabel { {}, "BPM:" };

    juce::Slider zoomSlider;
    juce::Label zoomLabel { {}, "ZOOM:" };

    float zoomScale = 1.0f;
    int scrollOffsetX = 0;

    int draggingTrackIdx = -1;
    int draggingStepIdx = -1;
    int draggingSubStepIdx = -1;
    bool isDraggingValue = false;
    juce::Point<int> dragStartPos;

    void setupUI();
    int getStepCount() const;
    int getCurrentStep() const;
    void applyPreset (const std::string& presetName);
    void showTupletContextMenu (int stepIdx);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepSequencerGridComponent)
};

} // namespace time_dilation
