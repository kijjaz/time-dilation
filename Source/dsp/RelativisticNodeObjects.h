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

// 4g. [spectrometer~] Live Audio Spectrum Visualizer (Logo Gradient Palette)
class SpectrometerAudioNode : public RelativisticNode
{
public:
    SpectrometerAudioNode (int id);
    void prepare (double sampleRate, int samplesPerBlock) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    const std::vector<float>& getBands() const { return bands; }
    const std::vector<float>& getPeaks() const { return peaks; }

private:
    static constexpr size_t numBands = 32;
    std::vector<float> bands;
    std::vector<float> peaks;
    std::vector<float> bandFilters;
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

// 11b. [number] Control Number Box Object
class NumberNode : public RelativisticNode
{
public:
    NumberNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
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
    void prepare (double sampleRate, int samplesPerBlock) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

private:
    juce::AudioBuffer<float> warpBuffer;
    int writePos = 0;
    double readPos = 0.0;
};

// 15. [out~] Master Audio Output Node Object with Live Oscilloscope & Dual RMS/Peak Metering
class OutNode : public RelativisticNode
{
public:
    OutNode (int id);
    ~OutNode() override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    float getRmsL() const { return rmsL; }
    float getRmsR() const { return rmsR; }
    float getPeakL() const { return peakL; }
    float getPeakR() const { return peakR; }

    const std::vector<float>& getScopeL() const { return scopeBufferL; }
    const std::vector<float>& getScopeR() const { return scopeBufferR; }
    int getScopeWriteIndex() const { return scopeWriteIdx; }

    bool isRecordingActive() const { return isRecording; }
    std::string getLastRecordFilePath() const { return lastRecordFilePath; }
    void startRecording();
    void stopRecording();

private:
    float rmsL = 0.0f;
    float rmsR = 0.0f;
    float peakL = 0.0f;
    float peakR = 0.0f;

    float limiterGain = 1.0f;

    std::vector<float> scopeBufferL;
    std::vector<float> scopeBufferR;
    int scopeWriteIdx = 0;

    juce::CriticalSection recordLock;
    bool isRecording = false;
    std::string lastRecordFilePath;
    std::unique_ptr<juce::AudioFormatWriter> recordWriter;
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
    void parseLabelArguments (const std::string& label) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 23. [*] Signal & Control Multiplier Node Object
class MulMathNode : public RelativisticNode
{
public:
    MulMathNode (int id);
    void process (int numSamples) override;
    void parseLabelArguments (const std::string& label) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// 24. [table] Relativistic Named Float Buffer Table Object
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
    int getTableSize() const { return getSize(); }
    void resize (int newSize);

    float readSample (int idx) const;
    float readSampleHermite (double pos) const;
    void writeSample (int idx, float val);
    void writeSampleNormalized (float normX, float normVal);
    void normalize();
    const std::vector<float>& getBufferData() const { return buffer; }
    const std::vector<float>& getTableData() const { return buffer; }

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

// 33. [fbdrum~] / [drum~] Future Bass Drum Synthesizer Node Object
class FutureBassDrumNode : public RelativisticNode
{
public:
    FutureBassDrumNode (int id);
    void prepare (double sampleRate, int samplesPerBlock) override;
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;

    void triggerNote (int midiNote, float velocity = 0.9f);

private:
    // Voice 1: Sub Kick
    double kickPhase = 0.0;
    double kickFreq = 180.0;
    float kickEnv = 0.0f;
    float kickClickEnv = 0.0f;

    // Voice 2: Snare Body + Noise
    double snareBodyPhase = 0.0;
    double snareBodyFreq = 240.0;
    float snareBodyEnv = 0.0f;
    float snareNoiseEnv = 0.0f;
    float snareFilterState = 0.0f;

    // Voice 3: Multi-Impulse Clap
    float clapEnv = 0.0f;
    int clapBurstCounter = 0;
    int clapTimer = 0;
    float clapFilterState = 0.0f;

    // Voice 4 & 5: Closed & Open Hi-Hat
    float hatEnv = 0.0f;
    bool isHatOpen = false;
    float hatFilterStateL = 0.0f;
    float hatFilterStateR = 0.0f;

    // Voice 6: FM Pitched Perc
    double percCarrierPhase = 0.0;
    double percModPhase = 0.0;
    double percCarrierFreq = 440.0;
    double percModFreq = 616.0;
    float percEnv = 0.0f;

    float lastTrigState = 0.0f;
    float lastNoteValue = 0.0f;
};

// ----------------------------------------------------
// 52. [meter~] Audio Peak & RMS Level VU Meter Object
// ----------------------------------------------------
class VuMeterAudioNode : public RelativisticNode
{
public:
    VuMeterAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    float getPeakLevelDb() const { return peakLevelDb.load(); }
    float getRmsLevelDb() const { return rmsLevelDb.load(); }

private:
    std::atomic<float> peakLevelDb { -100.0f };
    std::atomic<float> rmsLevelDb { -100.0f };
};

// ----------------------------------------------------
// 53. [number~] Audio-Rate Sample Monitor Object
// ----------------------------------------------------
class NumberAudioNode : public RelativisticNode
{
public:
    NumberAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;

    float getCurrentSampleValue() const { return currentSampleValue.load(); }

private:
    std::atomic<float> currentSampleValue { 0.0f };
};

// ----------------------------------------------------
// 55. [slider] Control Slider UI Node Object
// ----------------------------------------------------
class SliderNode : public RelativisticNode
{
public:
    SliderNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// ----------------------------------------------------
// 56. [toggle] Control Toggle Switch UI Node Object
// ----------------------------------------------------
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

// ----------------------------------------------------
// 57. [audio2time~] Audio Waveform to Time Dilation Signal Converter
// ----------------------------------------------------
class AudioToTimeNode : public RelativisticNode
{
public:
    AudioToTimeNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// ----------------------------------------------------
// 58. [time2audio~] Relativistic Time Signal to Audio Buffer Converter
// ----------------------------------------------------
class TimeToAudioNode : public RelativisticNode
{
public:
    TimeToAudioNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// ----------------------------------------------------
// 59. [time.math~] Time Signal Combiner & Relativistic Lorentz Math Processor
// ----------------------------------------------------
class TimeMathNode : public RelativisticNode
{
public:
    TimeMathNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// ----------------------------------------------------
// 60. [time.scale~] Time Dilation Signal Scaler & Shifter
// ----------------------------------------------------
class TimeScaleNode : public RelativisticNode
{
public:
    TimeScaleNode (int id);
    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
};

// ----------------------------------------------------
// 61. [time.filter~] Time Signal Inertia & Gravitational Slew Filter
// ----------------------------------------------------
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
// ----------------------------------------------------
// 62. [print] Control & Signal Data Logger Node Object
// ----------------------------------------------------
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
    float lastLoggedValue = -999999.0f;
    std::vector<std::string> logHistory;
    mutable std::mutex logMutex;
};

} // namespace time_dilation
