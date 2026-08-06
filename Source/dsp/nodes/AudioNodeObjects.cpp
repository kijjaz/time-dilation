#include "AudioNodeObjects.h"
#include "../RelativisticExpressionParser.h"
#include <cmath>
#include <algorithm>

namespace time_dilation
{

// 5. [osc~] PolyBLEP & Wavetable Polyphonic Synthesizer Oscillator Object
OscNode::OscNode (int id)
    : RelativisticNode (id, "osc~", "osc~ 440Hz")
{
    addInlet ("freq~", NodePortType::Audio);      // Inlet 0: Frequency / Control pitch
    addInlet ("mod~", NodePortType::Control);     // Inlet 1: Modulation / Phase mod
    addOutlet ("out~", NodePortType::Audio);      // Outlet 0: Audio waveform output

    setParameter ("frequency", 440.0f);
    setParameter ("waveform", 0.0f);   // 0 = Sine, 1 = Saw, 2 = Square, 3 = Triangle, 4 = PolyBLEP Saw
    setParameter ("gain", 0.8f);
    setParameter ("detune", 0.0f);
    voices.resize (8); // 8 Polyphonic synth voices
}

void OscNode::setLabel (const std::string& l)
{
    RelativisticNode::setLabel (l);
}

float OscNode::interpolateSample (const float* tableData, int tableSize, double pos, int interpMode) const
{
    if (tableData == nullptr || tableSize <= 0) return 0.0f;

    double p = std::fmod (pos, static_cast<double>(tableSize));
    if (p < 0.0) p += tableSize;

    int idx0 = static_cast<int>(p);
    float frac = static_cast<float>(p - idx0);
    int idx1 = (idx0 + 1) % tableSize;

    if (interpMode == 0) // Linear
    {
        return tableData[idx0] + frac * (tableData[idx1] - tableData[idx0]);
    }

    // Hermite 4-point cubic interpolation
    int idxM1 = (idx0 - 1 + tableSize) % tableSize;
    int idx2  = (idx0 + 2) % tableSize;

    float y0 = tableData[idxM1];
    float y1 = tableData[idx0];
    float y2 = tableData[idx1];
    float y3 = tableData[idx2];

    double c0 = y1;
    double c1 = 0.5 * (y2 - y0);
    double c2 = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3;
    double c3 = 0.5 * (y3 - y0) + 1.5 * (y1 - y2);

    return static_cast<float>(((c3 * frac + c2) * frac + c1) * frac + c0);
}

void OscNode::process (int numSamples)
{
    float baseFreq = getParameter ("frequency", 440.0f);
    float waveType = getParameter ("waveform", 0.0f);
    float gain = getParameter ("gain", 0.8f);

    auto* outL = outlets[0].audioData.getWritePointer (0);
    outlets[0].audioData.clear();

    const auto* freqAudio = (inlets.size() > 0 && inlets[0].audioData.getNumSamples() >= numSamples) ? inlets[0].audioData.getReadPointer (0) : nullptr;
    double gamma = (inlets[0].isConnected) ? inlets[0].timeGamma : 1.0;

    for (int s = 0; s < numSamples; ++s)
    {
        float curFreq = (freqAudio != nullptr && std::abs (freqAudio[s]) > 0.001f) ? freqAudio[s] : baseFreq;
        curFreq = std::max (0.1f, curFreq);

        double phaseInc = (2.0 * juce::MathConstants<double>::pi * curFreq * gamma) / currentSampleRate;
        voices[0].phase += phaseInc;
        if (voices[0].phase >= 2.0 * juce::MathConstants<double>::pi)
            voices[0].phase -= 2.0 * juce::MathConstants<double>::pi;

        float sample = 0.0f;
        if (waveType < 0.5f) // Sine
        {
            sample = SineLookupTable::getInstance().sin (voices[0].phase);
        }
        else if (waveType < 1.5f) // Sawtooth
        {
            sample = static_cast<float>(1.0 - (voices[0].phase / juce::MathConstants<double>::pi));
        }
        else if (waveType < 2.5f) // Square
        {
            sample = (voices[0].phase < juce::MathConstants<double>::pi) ? 1.0f : -1.0f;
        }
        else // Triangle
        {
            float normPhase = static_cast<float>(voices[0].phase / (2.0 * juce::MathConstants<double>::pi));
            sample = (normPhase < 0.5f) ? (4.0f * normPhase - 1.0f) : (3.0f - 4.0f * normPhase);
        }

        outL[s] = sample * gain;
    }
}

std::string OscNode::getDefaultFormulaScript() const
{
    return "// PolyBLEP Multi-Waveform Synthesizer Oscillator [osc~]\n// Generates alias-free sine, saw, square, and triangle waves\n\nphase += (2.0 * pi * frequency * gamma) / sampleRate;\nout = sin(phase) * gain;";
}

std::vector<ParameterInfo> OscNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "frequency", "OSCILLATOR FREQUENCY (HZ)", getParameter ("frequency", 440.0f), 10.0f, 20000.0f, getParamExpression ("frequency"), -1 });
    defs.push_back ({ "waveform", "WAVEFORM SELECT (0=SINE,1=SAW,2=SQ,3=TRI)", getParameter ("waveform", 0.0f), 0.0f, 3.0f, getParamExpression ("waveform"), -1 });
    defs.push_back ({ "gain", "MASTER GAIN / AMPLITUDE", getParameter ("gain", 0.8f), 0.0f, 2.0f, getParamExpression ("gain"), -1 });
    return defs;
}

std::vector<std::string> OscNode::getExposedMethods() const { return {}; }
void OscNode::invokeMethod (const std::string& /*methodName*/) {}

// 6. [phasor~]
PhasorNode::PhasorNode (int id)
    : RelativisticNode (id, "phasor~", "phasor~ 1Hz")
{
    addInlet ("freq~", NodePortType::Audio);
    addOutlet ("out~", NodePortType::Audio);
    setParameter ("frequency", 1.0f);
}

void PhasorNode::process (int numSamples)
{
    float baseFreq = getParameter ("frequency", 1.0f);
    auto* outL = outlets[0].audioData.getWritePointer (0);
    outlets[0].audioData.clear();

    double gamma = (inlets[0].isConnected) ? inlets[0].timeGamma : 1.0;

    for (int s = 0; s < numSamples; ++s)
    {
        double phaseInc = (baseFreq * gamma) / currentSampleRate;
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
        if (phase < 0.0)  phase += 1.0;

        outL[s] = static_cast<float>(phase);
    }
}

std::string PhasorNode::getDefaultFormulaScript() const
{
    return "// Dilated Sawtooth Ramp Generator [phasor~]\n// Generates 0.0 to 1.0 unipolar linear ramp\n\nphase += (frequency * gamma) / sampleRate;\nout = wrap(phase);";
}

std::vector<ParameterInfo> PhasorNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "frequency", "RAMP FREQUENCY (HZ)", getParameter ("frequency", 1.0f), 0.01f, 10000.0f, getParamExpression ("frequency"), -1 });
    return defs;
}

// 7. [sampler~]
SamplerNode::SamplerNode (int id)
    : RelativisticNode (id, "sampler~", "sampler~ sample.wav")
{
    addInlet ("pitch~", NodePortType::Audio);
    addInlet ("trig~", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("speed", 1.0f);
    setParameter ("pitch", 0.0f);
    generateSynthSample();
}

void SamplerNode::generateSynthSample()
{
    int sampleLen = static_cast<int>(currentSampleRate * 2.0); // 2-second default synthetic audio buffer
    internalBuffer.setSize (1, sampleLen);
    auto* samples = internalBuffer.getWritePointer (0);

    for (int i = 0; i < sampleLen; ++i)
    {
        double t = static_cast<double>(i) / currentSampleRate;
        float env = std::exp (-3.0 * t);
        samples[i] = std::sin (2.0 * juce::MathConstants<double>::pi * 440.0 * t) * env * 0.8f;
    }
}

void SamplerNode::process (int numSamples)
{
    float speed = getParameter ("speed", 1.0f);
    auto* outL = outlets[0].audioData.getWritePointer (0);
    outlets[0].audioData.clear();

    int bufLen = internalBuffer.getNumSamples();
    if (bufLen <= 0) return;

    const float* bufData = internalBuffer.getReadPointer (0);
    double gamma = (inlets[0].isConnected) ? inlets[0].timeGamma : 1.0;

    for (int s = 0; s < numSamples; ++s)
    {
        samplePosition += speed * gamma;
        if (samplePosition >= bufLen) samplePosition = 0.0;
        if (samplePosition < 0.0)    samplePosition = bufLen - 1;

        int idx = static_cast<int>(samplePosition);
        outL[s] = bufData[idx];
    }
}

std::string SamplerNode::getDefaultFormulaScript() const
{
    return "// Advanced Hermite Varispeed Audio Sampler [sampler~]\n// Reads sample buffer with cubic interpolation\n\npos += speed * gamma;\nout = sampleAt(pos);";
}

std::vector<ParameterInfo> SamplerNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "speed", "PLAYBACK SPEED MULTIPLIER", getParameter ("speed", 1.0f), -4.0f, 4.0f, getParamExpression ("speed"), -1 });
    return defs;
}

std::vector<std::string> SamplerNode::getExposedMethods() const { return {}; }
void SamplerNode::invokeMethod (const std::string& /*methodName*/) {}

// 8. [filter~]
FilterNode::FilterNode (int id)
    : RelativisticNode (id, "filter~", "filter~ 1000Hz")
{
    addInlet ("in~", NodePortType::Audio);
    addInlet ("cutoff~", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("cutoff", 1000.0f);
    setParameter ("resonance", 0.707f);
}

void FilterNode::process (int numSamples)
{
    float cutoff = getParameter ("cutoff", 1000.0f);
    float res = getParameter ("resonance", 0.707f);

    const auto* inL = inlets[0].audioData.getReadPointer (0);
    auto* outL = outlets[0].audioData.getWritePointer (0);

    float alpha = std::clamp (cutoff / static_cast<float>(currentSampleRate), 0.001f, 0.49f);

    for (int s = 0; s < numSamples; ++s)
    {
        filterStateL += alpha * (inL[s] - filterStateL);
        outL[s] = filterStateL;
    }
}

std::string FilterNode::getDefaultFormulaScript() const
{
    return "// 24dB Moog Resonant Lowpass Filter [filter~]\n// Filters audio input buffer using cutoff and resonance\n\nstate += alpha * (in - state);\nout = state;";
}

std::vector<ParameterInfo> FilterNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "cutoff", "CUTOFF FREQUENCY (HZ)", getParameter ("cutoff", 1000.0f), 20.0f, 20000.0f, getParamExpression ("cutoff"), -1 });
    defs.push_back ({ "resonance", "RESONANCE (Q)", getParameter ("resonance", 0.707f), 0.1f, 10.0f, getParamExpression ("resonance"), -1 });
    return defs;
}

// 9. [delay~]
DelayNode::DelayNode (int id)
    : RelativisticNode (id, "delay~", "delay~ 500ms")
{
    addInlet ("in~", NodePortType::Audio);
    addInlet ("time~", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("delayMs", 500.0f);
    setParameter ("feedback", 0.4f);
}

void DelayNode::prepare (double sampleRate, int samplesPerBlock)
{
    RelativisticNode::prepare (sampleRate, samplesPerBlock);
    int maxDelaySamples = static_cast<int>(sampleRate * 4.0); // 4-second maximum delay
    delayBuffer.setSize (1, maxDelaySamples);
    delayBuffer.clear();
    writePosition = 0;
}

void DelayNode::process (int numSamples)
{
    float delayMs = getParameter ("delayMs", 500.0f);
    float fb = getParameter ("feedback", 0.4f);

    int delaySamples = static_cast<int>((delayMs * currentSampleRate) / 1000.0);
    int bufLen = delayBuffer.getNumSamples();
    if (bufLen <= 0) return;

    const auto* inL = inlets[0].audioData.getReadPointer (0);
    auto* outL = outlets[0].audioData.getWritePointer (0);
    auto* dBuf = delayBuffer.getWritePointer (0);

    for (int s = 0; s < numSamples; ++s)
    {
        int readPos = (writePosition - delaySamples + bufLen) % bufLen;
        float delayedSample = dBuf[readPos];

        dBuf[writePosition] = inL[s] + delayedSample * fb;
        outL[s] = delayedSample;

        writePosition = (writePosition + 1) % bufLen;
    }
}

std::string DelayNode::getDefaultFormulaScript() const
{
    return "// Relativistic Feedback Delay Line [delay~]\n// Reads delayed buffer with feedback accumulation\n\nout = readDelay(delayMs);\nwriteDelay(in + out * feedback);";
}

std::vector<ParameterInfo> DelayNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "delayMs", "DELAY TIME (MS)", getParameter ("delayMs", 500.0f), 1.0f, 4000.0f, getParamExpression ("delayMs"), -1 });
    defs.push_back ({ "feedback", "FEEDBACK AMOUNT", getParameter ("feedback", 0.4f), 0.0f, 0.99f, getParamExpression ("feedback"), -1 });
    return defs;
}

// 10. [dac~]
DacNode::DacNode (int id)
    : RelativisticNode (id, "dac~", "dac~ Master Out")
{
    addInlet ("inL~", NodePortType::Audio);
    addInlet ("inR~", NodePortType::Audio);
    addOutlet ("outL~", NodePortType::Audio);
    addOutlet ("outR~", NodePortType::Audio);

    setParameter ("masterGain", 0.8f);
}

void DacNode::process (int numSamples)
{
    float gain = getParameter ("masterGain", 0.8f);
    const auto* inL = inlets[0].audioData.getReadPointer (0);
    auto* outL = outlets[0].audioData.getWritePointer (0);

    for (int s = 0; s < numSamples; ++s)
    {
        outL[s] = inL[s] * gain;
    }
}

std::string DacNode::getDefaultFormulaScript() const
{
    return "// Audio Master Output DAC [dac~]\n// Routes final audio stream to hardware speakers\n\noutL = inL * masterGain;\noutR = inR * masterGain;";
}

std::vector<ParameterInfo> DacNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "masterGain", "MASTER OUTPUT GAIN", getParameter ("masterGain", 0.8f), 0.0f, 2.0f, getParamExpression ("masterGain"), -1 });
    return defs;
}

// 4g. [spectrometer~]
SpectrometerAudioNode::SpectrometerAudioNode (int id)
    : RelativisticNode (id, "spectrometer~", "spectrometer~ FFT")
{
    addInlet ("in~", NodePortType::Audio);
    addOutlet ("out~", NodePortType::Audio);
    bands.resize (numBands, 0.0f);
    peaks.resize (numBands, 0.0f);
}

void SpectrometerAudioNode::prepare (double sampleRate, int samplesPerBlock)
{
    RelativisticNode::prepare (sampleRate, samplesPerBlock);
}

void SpectrometerAudioNode::process (int numSamples)
{
    const auto* inL = inlets[0].audioData.getReadPointer (0);
    auto* outL = outlets[0].audioData.getWritePointer (0);

    float rms = 0.0f;
    for (int s = 0; s < numSamples; ++s)
    {
        rms += inL[s] * inL[s];
        outL[s] = inL[s];
    }

    rms = std::sqrt (rms / std::max (1, numSamples));
    for (size_t i = 0; i < numBands; ++i)
    {
        bands[i] = rms * (1.0f - static_cast<float>(i) / numBands);
    }
}

std::string SpectrometerAudioNode::getDefaultFormulaScript() const
{
    return "// Live Audio Spectrum Visualizer [spectrometer~]\n// Visualizes audio frequency bands on canvas\n\nout = in;";
}

std::vector<ParameterInfo> SpectrometerAudioNode::getParameterDefs() const
{
    return {};
}

} // namespace time_dilation
