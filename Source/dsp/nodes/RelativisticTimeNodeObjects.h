#pragma once

#include "../RelativisticNodeGraph.h"

namespace time_dilation
{

// 1. [time.warp~] Dilated Time Context Generator Object
class TimeWarpNode : public RelativisticNode
{
public:
    TimeWarpNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    double phase = 0.0;
};

// 2. [time.retro~] Retrograde Time Reverser Object
class TimeRetroNode : public RelativisticNode
{
public:
    TimeRetroNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 3. [time.quantize~] Metric Grid Time Quantizer Object
class TimeQuantizeNode : public RelativisticNode
{
public:
    TimeQuantizeNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 4. [time.metro~] Dilated Metronome Pulse Generator Object
class TimeMetroNode : public RelativisticNode
{
public:
    TimeMetroNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    double beatProgress = 0.0;
};

// 4b. [time.stasis~] Gravitational Time Stasis / Freeze Engine Object
class TimeStasisNode : public RelativisticNode
{
public:
    TimeStasisNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 4c. [time.singularity~] Event Horizon Relativistic Warp / Redshift Object
class TimeSingularityNode : public RelativisticNode
{
public:
    TimeSingularityNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 4e. [time.future~] Future Lookahead Causality Offset Object
class TimeFutureNode : public RelativisticNode
{
public:
    TimeFutureNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    double getRequestedLookaheadSec() const { return getParameter ("lookahead", 1.0f); }
};

// 4d. [time.transport] Multi-Instance Relativistic Transport Object
class TimeTransportNode : public RelativisticNode
{
public:
    TimeTransportNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    double getCurrentBeatPosition() const { return currentBeatPosition; }
    bool getIsPlaying() const { return isPlaying; }
    bool getIsBeatFlashing() const { return beatFlashCounter > 0; }

private:
    double currentBeatPosition = 0.0;
    bool isPlaying = false;
    int beatFlashCounter = 0;
};

// 4e. [time.scope] Relativistic Time Monitor & Telemetry Visualizer Object
class TimeScopeNode : public RelativisticNode
{
public:
    TimeScopeNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    float getMonitoredGamma() const { return monitoredGamma; }
    double getMonitoredTimeSec() const { return monitoredTimeSec; }
    const std::vector<float>& getSignalHistory() const { return signalHistory; }
    size_t getHistoryWritePos() const { return historyWritePos; }
    float getAutoScaleMax() const { return autoScaleMax; }

private:
    float monitoredGamma = 1.0f;
    double monitoredTimeSec = 0.0;
    std::vector<float> signalHistory;
    size_t historyWritePos = 0;
    float autoScaleMax = 1.0f;
};

struct Point2D { float x = 0.0f; float y = 0.0f; };

// 4f. [time.xy] 2D Time & Control Signal XY Oscilloscope Plot Object
class TimeXYNode : public RelativisticNode
{
public:
    TimeXYNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    const std::vector<Point2D>& getPointHistory() const { return pointHistory; }
    float getAutoScaleRadius() const { return autoScaleRadius; }

private:
    std::vector<Point2D> pointHistory;
    size_t writePos = 0;
    float autoScaleRadius = 1.0f;
};

// 59. [time.math~] Lorentz Velocity Addition Composition Node
class TimeMathNode : public RelativisticNode
{
public:
    TimeMathNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 60. [time.scale~] Time Dilation Signal Scaler & Shifter
class TimeScaleNode : public RelativisticNode
{
public:
    TimeScaleNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 61. [time.filter~] Time Signal Inertia & Gravitational Slew Filter
class TimeFilterNode : public RelativisticNode
{
public:
    TimeFilterNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    double filteredGamma = 1.0;
};

// 63. [time.boost~] Relativistic Velocity Boost & Lorentz Transformer
class TimeLorentzBoostNode : public RelativisticNode
{
public:
    TimeLorentzBoostNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 64. [time.noise~] Relativistic Stochastic Temporal Jitter & Drift Generator
class TimeNoiseNode : public RelativisticNode
{
public:
    TimeNoiseNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    double randomState = 1.0;
};

// 65. [time.samplehold~] Relativistic Time Dilation Sample & Hold
class TimeSampleHoldNode : public RelativisticNode
{
public:
    TimeSampleHoldNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    double heldGamma = 1.0;
    float lastTrigState = 0.0f;
};

// 66. [time.invert~] Reciprocal Time Dilation & Un-Warping Restitution Node
class TimeInvertNode : public RelativisticNode
{
public:
    TimeInvertNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 67. [time.logic~] Relativistic Time Comparator & Gate Node
class TimeLogicNode : public RelativisticNode
{
public:
    TimeLogicNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 68. [time.delay~] Relativistic Time Signal Delay Line (Time Memory)
class TimeDelayNode : public RelativisticNode
{
public:
    TimeDelayNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    std::vector<double> historyBuffer;
    int writeHead = 0;
};

} // namespace time_dilation
