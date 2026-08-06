#pragma once

#include "../RelativisticNodeGraph.h"
#include "RelativisticTimeNodeObjects.h"

namespace time_dilation
{

// Global High-Precision Sine/Cosine Lookup Table Engine (-130 dBFS accuracy)
class SineLookupTable
{
public:
    static constexpr int TABLE_SIZE = 4096;

    SineLookupTable()
    {
        table.resize (TABLE_SIZE + 1);
        for (int i = 0; i <= TABLE_SIZE; ++i)
        {
            double phase = (static_cast<double>(i) / TABLE_SIZE) * 2.0 * juce::MathConstants<double>::pi;
            table[i] = static_cast<float>(std::sin (phase));
        }
    }

    static const SineLookupTable& getInstance()
    {
        static SineLookupTable instance;
        return instance;
    }

    inline float sin (double phase) const
    {
        double p = std::fmod (phase, 2.0 * juce::MathConstants<double>::pi);
        if (p < 0.0) p += 2.0 * juce::MathConstants<double>::pi;

        double pos = (p / (2.0 * juce::MathConstants<double>::pi)) * TABLE_SIZE;
        int idx = static_cast<int>(pos);
        float frac = static_cast<float>(pos - idx);

        float y1 = table[idx];
        float y2 = table[idx + 1];
        return y1 + frac * (y2 - y1);
    }

    inline float cos (double phase) const
    {
        return sin (phase + 0.5 * juce::MathConstants<double>::pi);
    }

private:
    std::vector<float> table;
};

// 5. [osc~] PolyBLEP & Wavetable Polyphonic Synthesizer Oscillator Object
class OscNode : public RelativisticNode
{
public:
    OscNode (int id);
    void setLabel (const std::string& l) override;
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

} // namespace time_dilation
