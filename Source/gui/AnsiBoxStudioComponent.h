#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

enum class AnsiObjectType
{
    MidiPlayer,
    SynthSampler,
    AudioEvent,
    TimeGenerator
};

struct AnsiBoxObject
{
    juce::String id;
    juce::String title;
    AnsiObjectType type;
    juce::Rectangle<float> bounds;
    juce::Colour color;

    // Parameters
    float gamma = 1.0f;
    float volume = 0.8f;
    bool isBlinking = false;
    std::vector<std::string> outputTapTargets; // Fan-out output tapping
};

class AnsiBoxStudioComponent : public juce::Component, public juce::Timer
{
public:
    explicit AnsiBoxStudioComponent (TimeDilationEngine& engine);
    ~AnsiBoxStudioComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    void addObject (AnsiObjectType type);

private:
    TimeDilationEngine& engine;
    std::vector<AnsiBoxObject> objects;

    int draggedObjectIdx = -1;
    juce::Point<float> dragOffset;

    bool isTappingSignal = false;
    int tapStartObjectIdx = -1;
    juce::Point<float> tapCurrentPos;

    bool blinkState = false;

    // ANSI Control Buttons
    juce::TextButton btnAddMidi { "[+ MIDI PLAYER]" };
    juce::TextButton btnAddSynth { "[+ SYNTH/SAMPLER]" };
    juce::TextButton btnAddAudio { "[+ AUDIO EVENT]" };
    juce::TextButton btnAddTimeGen { "[+ TIME GEN]" };

    juce::TextButton btnPlay { "[ PLAY ]" };
    juce::TextButton btnStop { "[ STOP ]" };
    juce::TextButton btnAudition { "[ AUDITION ]" };

    void drawAnsiBox (juce::Graphics& g, const AnsiBoxObject& obj);
    void drawAnsiText (juce::Graphics& g, const juce::String& text, float x, float y, juce::Colour color, bool blink = false);
    void drawTapCables (juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnsiBoxStudioComponent)
};

} // namespace time_dilation
