#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <algorithm>
#include "HermiteResampler.h"

namespace time_dilation
{

class RetrogradeAudioBuffer
{
public:
    RetrogradeAudioBuffer() = default;

    void setAudioBuffer (const juce::AudioBuffer<float>& buffer)
    {
        sourceBuffer.makeCopyOf (buffer);
        numSamples = sourceBuffer.getNumSamples();
    }

    // Read audio at proper time position tau, under time dilation gamma
    void readSamples (juce::AudioBuffer<float>& outputBuffer, int outputStartSample, int numToProcess, double& currentTau, float gamma, bool isLooping, double loopStartTau, double loopEndTau)
    {
        if (numSamples == 0) return;

        bool isReverse = (gamma < 0.0f);
        float absGamma = std::abs (gamma);
        if (absGamma < 0.001f) return;

        auto* outL = outputBuffer.getWritePointer (0, outputStartSample);
        auto* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, outputStartSample) : nullptr;

        const float* srcL = sourceBuffer.getReadPointer (0);
        const float* srcR = sourceBuffer.getNumChannels() > 1 ? sourceBuffer.getReadPointer (1) : srcL;

        double loopLen = std::max (0.1, loopEndTau - loopStartTau);

        for (int i = 0; i < numToProcess; ++i)
        {
            // Advance or recede proper time tau based on gamma sign
            if (isReverse)
            {
                currentTau -= (1.0 / 44100.0) * absGamma;
                if (isLooping && currentTau < loopStartTau)
                {
                    currentTau = loopEndTau - std::fmod (loopStartTau - currentTau, loopLen);
                }
                else if (!isLooping && currentTau < 0.0)
                {
                    currentTau = 0.0;
                }
            }
            else
            {
                currentTau += (1.0 / 44100.0) * absGamma;
                if (isLooping && currentTau >= loopEndTau)
                {
                    currentTau = loopStartTau + std::fmod (currentTau - loopStartTau, loopLen);
                }
                else if (!isLooping && currentTau >= (numSamples / 44100.0))
                {
                    currentTau = (numSamples - 1) / 44100.0;
                }
            }

            // Convert proper time tau to sample index
            double samplePos = currentTau * 44100.0;
            samplePos = juce::jlimit (0.0, static_cast<double>(numSamples - 1), samplePos);

            int idx0 = static_cast<int>(samplePos);
            int idx1 = std::min (idx0 + 1, numSamples - 1);
            float frac = static_cast<float>(samplePos - idx0);

            // Linear / Hermite sample interpolation
            float sampL = srcL[idx0] * (1.0f - frac) + srcL[idx1] * frac;
            float sampR = srcR[idx0] * (1.0f - frac) + srcR[idx1] * frac;

            outL[i] += sampL;
            if (outR != nullptr) outR[i] += sampR;
        }
    }

private:
    juce::AudioBuffer<float> sourceBuffer;
    int numSamples = 0;
};

} // namespace time_dilation
