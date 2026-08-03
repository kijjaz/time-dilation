#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>

namespace time_dilation
{

class DopplerDelay
{
public:
    DopplerDelay() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels = 2)
    {
        this->sampleRate = sampleRate;
        this->numChannels = numChannels;
        bufferSize = std::max (maxBlockSize * 32, 131072);
        delayBuffer.setSize (numChannels, bufferSize);
        delayBuffer.clear();
        writePos = 0;
        currentDelaySamples = sampleRate * 0.05; // 50ms default delay
    }

    void reset()
    {
        delayBuffer.clear();
        writePos = 0;
    }

    void process (const juce::AudioBuffer<float>& inputBuffer,
                  juce::AudioBuffer<float>& outputBuffer,
                  float gamma)
    {
        const int numSamples = inputBuffer.getNumSamples();
        const int outSamples = outputBuffer.getNumSamples();
        gamma = juce::jlimit (0.1f, 4.0f, gamma);

        // Calculate target delay samples based on Doppler velocity (gamma)
        double targetDelaySamples = sampleRate * 0.05 * (1.0 / gamma);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* inData = inputBuffer.getReadPointer (ch);
            float* delayData = delayBuffer.getWritePointer (ch);

            for (int i = 0; i < numSamples; ++i)
            {
                int wPos = (writePos + i) % bufferSize;
                delayData[wPos] = inData[i];
            }
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* delayData = delayBuffer.getReadPointer (ch);
            float* outData = outputBuffer.getWritePointer (ch);

            for (int i = 0; i < outSamples; ++i)
            {
                // Smoothly modulate delay time (Doppler shift)
                currentDelaySamples += (targetDelaySamples - currentDelaySamples) * 0.005;

                double readPos = static_cast<double>((writePos + i) % bufferSize) - currentDelaySamples;
                if (readPos < 0) readPos += bufferSize;

                int i1 = static_cast<int>(readPos) % bufferSize;
                int i2 = (i1 + 1) % bufferSize;
                float frac = static_cast<float>(readPos - std::floor (readPos));

                outData[i] = delayData[i1] + frac * (delayData[i2] - delayData[i1]);
            }
        }

        writePos = (writePos + numSamples) % bufferSize;
    }

private:
    double sampleRate = 44100.0;
    int numChannels = 2;
    int bufferSize = 16384;
    juce::AudioBuffer<float> delayBuffer;
    int writePos = 0;
    double currentDelaySamples = 2205.0;
};

} // namespace time_dilation
