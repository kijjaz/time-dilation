#pragma once

#include "RelativisticNodeGraph.h"
#include <vector>
#include <string>

namespace time_dilation
{

// ----------------------------------------------------
// 1. [seq] / [step] Step Sequencer Node
// ----------------------------------------------------
class StepSequencerNode : public RelativisticNode
{
public:
    StepSequencerNode (int id);

    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    void setPatternString (const std::string& patStr);
    std::string getPatternString() const { return patternString; }
    void setLabel (const std::string& l) override;

    int getCurrentStep() const { return currentStep; }
    const std::vector<float>& getStepValues() const { return stepValues; }


private:
    std::string patternString = "60 62 64 65 67 69 71 72";
    std::vector<float> stepValues;
    int currentStep = 0;
    double stepProgress = 0.0;
    bool isPingPongReversing = false;
    int flashCounter = 0;
};

// ----------------------------------------------------
// 2. [euclid] Euclidean Rhythm Generator
// ----------------------------------------------------
class EuclideanSequencerNode : public RelativisticNode
{
public:
    EuclideanSequencerNode (int id);

    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    void generatePattern();
    void setPitchPatternString (const std::string& patStr);
    void setLabel (const std::string& l) override;

private:
    int pulses = 5;
    int steps = 8;
    int rotation = 0;
    std::vector<bool> pattern;
    std::string pitchPatternString = "60 63 65 67 70";
    std::vector<float> pitchValues;
    int currentStep = 0;
    int pitchIdx = 0;
    double stepProgress = 0.0;
};

// ----------------------------------------------------
// 3. [markov] Generative Markov Chain Sequencer
// ----------------------------------------------------
class MarkovSequencerNode : public RelativisticNode
{
public:
    MarkovSequencerNode (int id);

    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    void setStatesString (const std::string& stateStr);
    void setLabel (const std::string& l) override;

private:
    std::string statesString = "60 63 65 67 70";
    std::vector<float> states { 60.0f, 63.0f, 65.0f, 67.0f, 70.0f }; // Pentatonic scale
    int currentStateIdx = 0;
    double stepProgress = 0.0;
};

// ----------------------------------------------------
// 4. [tidal] TidalCycles-Style Mini-Notation Sequencer
// ----------------------------------------------------
struct TidalEvent
{
    double startPhase = 0.0;     // Cycle phase start [0.0, 1.0)
    double durationPhase = 0.25;  // Duration in cycle phase
    float pitch = 60.0f;          // MIDI pitch or frequency value
    float gain = 1.0f;           // Amplitude velocity [0.0, 1.0]
    float pan = 0.5f;            // Stereo panning [0.0 = L, 1.0 = R]
    float cutoff = 2000.0f;      // Filter cutoff frequency (Hz)
    float decay = 200.0f;        // Envelope decay length (ms)
    double gamma = 1.0;          // Relativistic time dilation factor per step
    bool isRest = false;
    bool isStasis = false;
};

class TidalPatternSequencerNode : public RelativisticNode
{
public:
    TidalPatternSequencerNode (int id);

    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    void setPatternString (const std::string& patStr);
    std::string getPatternString() const { return patternString; }
    void setLabel (const std::string& l) override;

    void parsePattern();

private:
    std::string patternString = "60 [62 64] 65 [67 69 71]";
    std::vector<TidalEvent> events;
    double cyclePhase = 0.0;
    int currentEventIdx = -1;
    int cycleCount = 0;
};

// ----------------------------------------------------
// 5. [drumseq] Multi-Track Future Bass Drum Sequencer
// ----------------------------------------------------
struct DrumStep
{
    bool kick = false;
    bool snare = false;
    bool clap = false;
    bool hatClosed = false;
    bool hatOpen = false;
    bool perc = false;
    int percPitch = 48;
    float velocity = 0.9f;
};

class DrumSequencerNode : public RelativisticNode
{
public:
    DrumSequencerNode (int id);

    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    void setPresetPattern (const std::string& presetName);
    void setLabel (const std::string& l) override;

    const std::vector<DrumStep>& getSteps() const { return gridSteps; }
    int getCurrentStep() const { return currentStep; }

private:
    std::vector<DrumStep> gridSteps;
    int currentStep = 0;
    double stepProgress = 0.0;
};

} // namespace time_dilation
