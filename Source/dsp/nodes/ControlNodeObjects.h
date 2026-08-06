#pragma once

#include "../RelativisticNodeGraph.h"

namespace time_dilation
{

// 14. [table] Shared Audio Sample / Function Lookup Buffer Object
class TableNode : public RelativisticNode
{
public:
    TableNode (int id, const std::string& name = "array1", int size = 512);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    const float* getBufferData() const { return sampleBuffer.data(); }
    int getBufferSize() const { return static_cast<int>(sampleBuffer.size()); }
    void resizeBuffer (int newSize);
    void setSample (int index, float value);

private:
    std::string tableName;
    std::vector<float> sampleBuffer;
};

// 15. [tabread~] Table Audio Reader Object
class TabreadNode : public RelativisticNode
{
public:
    TabreadNode (int id);
    void setLabel (const std::string& l) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    std::string tableName;
};

// 16. [tabwrite~] Table Audio Writer Object
class TabwriteNode : public RelativisticNode
{
public:
    TabwriteNode (int id);
    void setLabel (const std::string& l) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    std::string tableName;
    int writePosition = 0;
    bool isWriting = false;
};

// 17. [pack] Multi-Type Control List Packer Object
class PackNode : public RelativisticNode
{
public:
    PackNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 18. [unpack] Multi-Type Control List Unpacker Object
class UnpackNode : public RelativisticNode
{
public:
    UnpackNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 20. [spigot] Signal & Control Flow Switch Gate Object
class SpigotNode : public RelativisticNode
{
public:
    SpigotNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 21. [select] / [sel] Value Router & Trigger Pulse Matching Object
class SelectNode : public RelativisticNode
{
public:
    SelectNode (int id, float targetValue = 0.0f);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float matchTarget = 0.0f;
    float lastValueIn = -999999.0f;
};

// 22. [route] Multi-Key Message & Value Splitter Object
class RouteNode : public RelativisticNode
{
public:
    RouteNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 23. [change] Value Duplication Suppressor Object
class ChangeNode : public RelativisticNode
{
public:
    ChangeNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float lastSeenValue = -999999.0f;
};

// 24. [moses] Numeric Range Splitter Object
class MosesNode : public RelativisticNode
{
public:
    MosesNode (int id, float threshold = 0.0f);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float splitThreshold = 0.0f;
};

// 25. [line] Control Linear Ramp & Glide Generator Object
class LineNode : public RelativisticNode
{
public:
    LineNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float currentValue = 0.0f;
    float targetValue = 0.0f;
    float stepDelta = 0.0f;
    int remainingSteps = 0;
};

// 26. [line~] Audio-Rate Linear Envelope & Ramp Generator Object
class LineAudioNode : public RelativisticNode
{
public:
    LineAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float currentValue = 0.0f;
    float targetValue = 0.0f;
    float stepDelta = 0.0f;
    int remainingSamples = 0;
};

// 27. [vd~] Variable Delay Line with Fractional Interpolation Object
class VdNode : public RelativisticNode
{
public:
    VdNode (int id);
    void prepare (double sampleRate, int samplesPerBlock) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;
};

// 28. [delwrite~] Tap Delay Line Buffer Writer Object
class DelwriteNode : public RelativisticNode
{
public:
    DelwriteNode (int id, const std::string& name = "del1", float maxTimeMs = 1000.0f);
    void prepare (double sampleRate, int samplesPerBlock) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    const float* getBufferReadPointer (int channel) const { return delayBuffer.getReadPointer (channel); }
    int getBufferSize() const { return delayBuffer.getNumSamples(); }
    int getWritePosition() const { return writePosition; }
    std::string getLineName() const { return lineName; }

private:
    std::string lineName;
    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;
};

// 29. [delread~] Tap Delay Line Buffer Reader Object
class DelreadNode : public RelativisticNode
{
public:
    DelreadNode (int id);
    void setLabel (const std::string& l) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    std::string targetLineName;
};

// 30. [env~] Envelope Follower & RMS Power Meter Object
class EnvNode : public RelativisticNode
{
public:
    EnvNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float currentRmsDb = -100.0f;
};

// 31. [hip~] Highpass Filter Object
class HipNode : public RelativisticNode
{
public:
    HipNode (int id, float cutoffHz = 100.0f);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float stateL = 0.0f;
    float stateR = 0.0f;
    float prevInL = 0.0f;
    float prevInR = 0.0f;
};

// 32. [lop~] Lowpass Filter Object
class LopNode : public RelativisticNode
{
public:
    LopNode (int id, float cutoffHz = 1000.0f);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float stateL = 0.0f;
    float stateR = 0.0f;
};

// 33. [bp~] Bandpass Filter Object
class BpNode : public RelativisticNode
{
public:
    BpNode (int id, float centerHz = 1000.0f, float q = 2.0f);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float state1L = 0.0f, state2L = 0.0f;
    float state1R = 0.0f, state2R = 0.0f;
};

// 34. [vcf~] Voltage Controlled Resonant Filter Object
class VcfNode : public RelativisticNode
{
public:
    VcfNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float state1L = 0.0f, state2L = 0.0f;
    float state1R = 0.0f, state2R = 0.0f;
};

// 35. [noise~] White Noise Generator Object
class NoiseNode : public RelativisticNode
{
public:
    NoiseNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 36. [pink~] Pink Noise Generator Object (Voss-McCartney Filtered)
class PinkNode : public RelativisticNode
{
public:
    PinkNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, b3 = 0.0f, b4 = 0.0f, b5 = 0.0f, b6 = 0.0f;
};

// 37. [thresh~] Signal Threshold Trigger Detector Object
class ThreshNode : public RelativisticNode
{
public:
    ThreshNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    bool isAbove = false;
};

// 38. [snapshot~] Audio to Control Sample Value Sampler Object
class SnapshotNode : public RelativisticNode
{
public:
    SnapshotNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 39. [samplerate~] Engine Sample Rate Query Object
class SamplerateNode : public RelativisticNode
{
public:
    SamplerateNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 40. [blocksize~] Audio Vector Size Query Object
class BlocksizeNode : public RelativisticNode
{
public:
    BlocksizeNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 41. [clip~] Audio Signal Hard Clipper / Limiter Object
class ClipAudioNode : public RelativisticNode
{
public:
    ClipAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 42. [wrap~] Fractional Phase Wrapper Object
class WrapNode : public RelativisticNode
{
public:
    WrapNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 43. [sqrt~] Audio Square Root Shaper Object
class SqrtAudioNode : public RelativisticNode
{
public:
    SqrtAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 44. [rsqrt~] Reciprocal Square Root Shaper Object
class RsqrtAudioNode : public RelativisticNode
{
public:
    RsqrtAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 45. [log~] Audio Natural Logarithm Object
class LogAudioNode : public RelativisticNode
{
public:
    LogAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 46. [ftom~] Audio Frequency to MIDI Pitch Converter
class FtomAudioNode : public RelativisticNode
{
public:
    FtomAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 47. [mtof~] Audio MIDI Pitch to Frequency Converter
class MtofAudioNode : public RelativisticNode
{
public:
    MtofAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 48. [pow~] Signal Power Function Shaper Object
class PowAudioNode : public RelativisticNode
{
public:
    PowAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 49. [abs~] Full-Wave Rectifier Object
class AbsAudioNode : public RelativisticNode
{
public:
    AbsAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 50. [adc~] Live Audio Input ADC Microphone Object
class AdcNode : public RelativisticNode
{
public:
    AdcNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 51. [sig~] Control Float to Constant Audio Signal Converter
class SigNode : public RelativisticNode
{
public:
    SigNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 52. [osc.quad~] Quad-Phase LFO & Quadrature Modulation Generator
class OscQuadNode : public RelativisticNode
{
public:
    OscQuadNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    double phase = 0.0;
};

// 53. [reverb~] Freeverb Stereo Algorithmic Reverb Object
class ReverbNode : public RelativisticNode
{
public:
    ReverbNode (int id);
    void prepare (double sampleRate, int samplesPerBlock) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    juce::Reverb reverbEngine;
    juce::Reverb::Parameters reverbParams;
};

// 54. [gain~] Master Volume Attenuator & Gain Object
class GainNode : public RelativisticNode
{
public:
    GainNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 11. [expr] Pure Data-style Control Expression Object
class ExprNode : public RelativisticNode
{
public:
    ExprNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 11b. [number] / [f] / [i] Control Number Box Object
class NumberNode : public RelativisticNode
{
public:
    NumberNode (int id, bool isIntegerMode = false);
    void setLabel (const std::string& l) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    bool getIsIntegerMode() const { return isIntegerMode; }

private:
    bool isIntegerMode = false;
    float lastHotValue = 0.0f;
    float lastColdValue = 0.0f;
};

// 19. [v] / [msg] / [message] Value & Message Symbol Box Object
class ValueNode : public RelativisticNode
{
public:
    ValueNode (int id);
    void setLabel (const std::string& l) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    static std::string formatMessageWithArgs (const std::string& templateStr, float val1, float val2 = 0.0f);

private:
    float storedValue = 0.0f;
    std::string messageTemplate = "";
    float lastHotValue = 0.0f;
    float lastColdValue = 0.0f;
    std::string lastMessageIn = "";
};

// 11c. [bang] Control Trigger Pulse Object
class BangNode : public RelativisticNode
{
public:
    BangNode (int id);
    void triggerBang();
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;
private:
    bool bangRequested = false;
};

// 11d. [bang~] Audio Rate Impulse Spike Object
class BangAudioNode : public RelativisticNode
{
public:
    BangAudioNode (int id);
    void triggerBang();
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;
private:
    bool bangRequested = false;
};

// 11e. [counter] Smart Value Counter Object
class CounterNode : public RelativisticNode
{
public:
    CounterNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    float getCurrentCount() const { return currentCount; }
private:
    float currentCount = 0.0f;
    bool lastInletState = false;
    bool lastResetState = false;
};

// 11f. [metro] Standard Control Metronome Object
class MetroNode : public RelativisticNode
{
public:
    MetroNode (int id, float initialPeriodMs = 500.0f);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;
    void receiveMessage (const std::string& msg, float val = 1.0f) override;
private:
    double sampleProgress = 0.0;
    bool isRunning = false;
    bool lastHotInletState = false;
};

// 11g. [send] / [s] Wireless Control Message Broadcaster
class SendNode : public RelativisticNode
{
public:
    SendNode (int id, const std::string& busName = "bus1");
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    void receiveMessage (const std::string& msg, float val = 1.0f) override;
};

// 11h. [receive] / [r] Wireless Control Message Receiver
class ReceiveNode : public RelativisticNode
{
public:
    ReceiveNode (int id, const std::string& busName = "bus1");
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    void receiveMessage (const std::string& msg, float val = 1.0f) override;
};

// 55. [slider] Control Slider UI Node Object
class SliderNode : public RelativisticNode
{
public:
    SliderNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 56. [toggle] Control Toggle Switch UI Node Object
class ToggleNode : public RelativisticNode
{
public:
    ToggleNode (int id);
    void toggleState();
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;
};

// 57. [audio2time~] Audio Waveform to Time Dilation Signal Converter
class AudioToTimeNode : public RelativisticNode
{
public:
    AudioToTimeNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 58. [time2audio~] Relativistic Time Signal to Audio Buffer Converter
class TimeToAudioNode : public RelativisticNode
{
public:
    TimeToAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 69. [print] / [monitor] Signal Logger & Inspector Node
class PrintMonitorNode : public RelativisticNode
{
public:
    PrintMonitorNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getLogHistory() const;
    void clearLog();
private:
    mutable std::mutex logMutex;
    std::vector<std::string> logHistory;
    float lastLoggedValue = -99999.0f;
    std::string lastLoggedMsg;
};

} // namespace time_dilation
