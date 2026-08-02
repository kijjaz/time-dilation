#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace time_dilation
{

enum class LfoWaveform
{
    Off,
    Sine,
    Triangle,
    BlackHoleExp, // Exponential deceleration
    TachyonPulse, // High velocity spikes
    LorenzChaos   // Organic chaotic drift
};

class GammaLFO
{
public:
    GammaLFO() = default;

    void prepare (double sampleRate)
    {
        this->sampleRate = sampleRate;
        phase = 0.0;
        x = 0.1; y = 0.0; z = 0.0;
    }

    void setWaveform (LfoWaveform type) { waveform = type; }
    void setFrequency (float freqHz) { frequency = juce::jlimit (0.01f, 20.0f, freqHz); }
    void setDepth (float depthAmount) { depth = juce::jlimit (0.0f, 2.0f, depthAmount); }

    float getNextGamma (double currentCoordinateTime)
    {
        if (waveform == LfoWaveform::Off || depth <= 0.001f) return 1.0f;

        float lfoVal = 0.0f;
        const double deltaPhase = (frequency / sampleRate);
        phase = std::fmod (phase + deltaPhase, 1.0);

        switch (waveform)
        {
            case LfoWaveform::Sine:
                lfoVal = std::sin (phase * 2.0 * juce::MathConstants<double>::pi);
                break;
            case LfoWaveform::Triangle:
                lfoVal = static_cast<float>(4.0 * std::abs (phase - 0.5) - 1.0);
                break;
            case LfoWaveform::BlackHoleExp:
                // Exponential deceleration into slow time
                lfoVal = -std::exp (-static_cast<float>(phase) * 3.0f) + 0.5f;
                break;
            case LfoWaveform::TachyonPulse:
                lfoVal = (phase > 0.85) ? 1.0f : -0.2f;
                break;
            case LfoWaveform::LorenzChaos:
            {
                // Lorenz Attractor Differential Equations
                double dt = 0.01;
                double dx = 10.0 * (y - x) * dt;
                double dy = (x * (28.0 - z) - y) * dt;
                double dz = (x * y - (8.0 / 3.0) * z) * dt;
                x += dx; y += dy; z += dz;
                lfoVal = static_cast<float>(x / 20.0);
                break;
            }
            default:
                lfoVal = 0.0f;
        }

        // Multiplicative Gamma Modulation: gamma = 1.0 + (lfoVal * depth)
        float gamma = 1.0f + (lfoVal * depth);
        return juce::jlimit (0.1f, 4.0f, gamma);
    }

private:
    double sampleRate = 44100.0;
    double phase = 0.0;
    float frequency = 0.5f;
    float depth = 0.5f;
    LfoWaveform waveform = LfoWaveform::Off;

    // Lorenz Attractor State Variables
    double x = 0.1, y = 0.0, z = 0.0;
};

} // namespace time_dilation
