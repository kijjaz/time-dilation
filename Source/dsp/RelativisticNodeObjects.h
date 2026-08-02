#pragma once

#include "RelativisticNodeGraph.h"

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

private:
    double currentBeatPosition = 0.0;
    bool isPlaying = false;
};

// 5. [osc~] PolyBLEP & Wavetable Polyphonic Synthesizer Oscillator Object
class OscNode : public RelativisticNode
{
public:
    OscNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    struct Voice
    {
        double phase = 0.0;
        float note = 69.0f;
        float freq = 440.0f;
        float velocity = 0.8f;
        bool isPingPongReversing = false;
    };

    float interpolateSample (const float* tableData, int tableSize, double pos, int interpMode) const;

private:
    std::vector<Voice> voices;
    std::string tableName;
};

// 5b. [mtof] MIDI Note to Frequency (Hz) Converter Node Object
class MtofNode : public RelativisticNode
{
public:
    MtofNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 5c. [ftom] Frequency (Hz) to MIDI Note Converter Node Object
class FtomNode : public RelativisticNode
{
public:
    FtomNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 5d. [note] Algorithmic MIDI Note Generator Node Object
class NoteGenNode : public RelativisticNode
{
public:
    NoteGenNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 6. [phasor~] Dilated Sawtooth Ramp Generator Object
class PhasorNode : public RelativisticNode
{
public:
    PhasorNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    double phase = 0.0;
};

// 7. [sampler~] Advanced Hermite Varispeed & Granular Audio Sampler Object
class SamplerNode : public RelativisticNode
{
public:
    SamplerNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    bool loadAudioFile (const juce::File& file);

private:
    double samplePosition = 0.0;
    bool isPingPongReversing = false;
    juce::AudioBuffer<float> internalBuffer;

    void generateSynthSample();
    float interpolateHermite (const float* buffer, int bufferSize, double pos) const;
};

// 8. [filter~] 24dB Moog Resonant Lowpass Filter Object
class FilterNode : public RelativisticNode
{
public:
    FilterNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float filterStateL = 0.0f;
    float filterStateR = 0.0f;
};

// 9. [delay~] Delay Line Object
class DelayNode : public RelativisticNode
{
public:
    DelayNode (int id);
    void prepare (double sampleRate, int samplesPerBlock) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;
};

// 10. [dac~] Audio Master Output DAC Object
class DacNode : public RelativisticNode
{
public:
    DacNode (int id);
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

// 12. [expr~] Pure Data-style Audio Signal Expression Object
class ExprAudioNode : public RelativisticNode
{
public:
    ExprAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 13. [fexpr~] Pure Data-style Filter / Recurrent Audio Expression Object
class FexprAudioNode : public RelativisticNode
{
public:
    FexprAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float prevSampleL = 0.0f;
    float prevSampleR = 0.0f;
};

// 14. [gain~] Audio Signal Scaler Node Object
class GainNode : public RelativisticNode
{
public:
    GainNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 15. [out~] Master Audio Output Node Object with Live RMS & Peak Metering
class OutNode : public RelativisticNode
{
public:
    OutNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    float getRmsL() const { return rmsL; }
    float getRmsR() const { return rmsR; }
    float getPeakL() const { return peakL; }
    float getPeakR() const { return peakR; }

private:
    float rmsL = 0.0f;
    float rmsR = 0.0f;
    float peakL = 0.0f;
    float peakR = 0.0f;
};

// 16. [env~] Envelope Follower Node Object
class EnvFollowerNode : public RelativisticNode
{
public:
    EnvFollowerNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float currentEnvelope = 0.0f;
};

// 17. [tap] Control Signal Wireless Tap Object
class TapControlNode : public RelativisticNode
{
public:
    TapControlNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 18. [tap~] Audio Signal Wireless Tap Object
class TapAudioNode : public RelativisticNode
{
public:
    TapAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 19. [v] Value Storage Control Node Object
class ValueNode : public RelativisticNode
{
public:
    ValueNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float storedValue = 0.0f;
};

// 20. [z~] 1-Sample Feedback Delay Node Object
class OneSampleDelayNode : public RelativisticNode
{
public:
    OneSampleDelayNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float lastSampleL = 0.0f;
    float lastSampleR = 0.0f;
};

// 21. [snapshot~] Audio-to-Control Snapshot Node Object
class SnapshotNode : public RelativisticNode
{
public:
    SnapshotNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 22. [+] Signal & Control Adder Node Object
class AddMathNode : public RelativisticNode
{
public:
    AddMathNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 23. [*] Signal & Control Multiplier Node Object
class MulMathNode : public RelativisticNode
{
public:
    MulMathNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 24. [table] Pure Data-Style Named Float Buffer Table Object
class TableNode : public RelativisticNode
{
public:
    TableNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    std::string getTableName() const { return tableName; }
    void setTableName (const std::string& name) { tableName = name; setLabel ("table " + name); }

    int getSize() const { return static_cast<int>(buffer.size()); }
    void resize (int newSize);

    float readSample (int idx) const;
    float readSampleHermite (double pos) const;
    void writeSample (int idx, float val);
    const std::vector<float>& getBufferData() const { return buffer; }

    void generatePreset (int presetIndex);

private:
    std::string tableName = "array1";
    std::vector<float> buffer;
};

// 25. [tabwrite~] Sound & Data Recorder Node Object
class TabWriteNode : public RelativisticNode
{
public:
    TabWriteNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    int writeIndex = 0;
    bool isRecording = false;
};

// 26. [tabread~] Table Reader Node Object
class TabReadNode : public RelativisticNode
{
public:
    TabReadNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 27. [tabosc4~] 4-Point Hermite Wavetable Oscillator Node Object
class TabOscNode : public RelativisticNode
{
public:
    TabOscNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    double phase = 0.0;
};

// 28. [svfilter~] Multi-Mode State Variable Filter Node Object
class SvFilterNode : public RelativisticNode
{
public:
    SvFilterNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float s1 = 0.0f;
    float s2 = 0.0f;
};

// 29. [drive~] Non-Linear Tube Saturation & Overdrive Node Object
class DriveNode : public RelativisticNode
{
public:
    DriveNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 30. [reverb~] High-Density Stereo Reverb Node Object
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

// 31. [crush~] Bit-Crusher & Downsampler Node Object
class CrushNode : public RelativisticNode
{
public:
    CrushNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    float holdSampleL = 0.0f;
    float holdSampleR = 0.0f;
    int sampleCounter = 0;
};

// 32. [adsr~] 4-Stage ADSR Envelope Generator Node Object
class AdsrNode : public RelativisticNode
{
public:
    AdsrNode (int id);
    void prepare (double sampleRate, int samplesPerBlock) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
private:
    juce::ADSR adsrEngine;
    juce::ADSR::Parameters adsrParams;
    bool lastGateState = false;
};

} // namespace time_dilation
