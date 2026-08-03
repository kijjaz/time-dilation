#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>

namespace time_dilation
{

/**
 * High-Fidelity Cubic Hermite Fractional Delay Line & Resampler
 * Allows smooth varispeed pitch & rate interpolation for Track Time Dilation (gamma = 0.1x to 4.0x)
 */
class HermiteResampler
{
public:
    HermiteResampler() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels = 2)
    {
        this->sampleRate = sampleRate;
        this->numChannels = numChannels;
        bufferSize = std::max (maxBlockSize * 32, 131072);
        ringBuffer.setSize (numChannels, bufferSize);
        ringBuffer.clear();
        writePos = 0;
    }

    void reset()
    {
        ringBuffer.clear();
        writePos = 0;
    }

    /** Interpolates 4 consecutive points using 3rd-order cubic Hermite spline */
    static inline float interpolateHermite (float y0, float y1, float y2, float y3, float frac)
    {
        float c0 = y1;
        float c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

    /** Process input block into output block with speed multiplier gamma */
    void process (const juce::AudioBuffer<float>& inputBuffer,
                  juce::AudioBuffer<float>& outputBuffer,
                  float gamma)
    {
        const int numSamples = inputBuffer.getNumSamples();
        const int outSamples = outputBuffer.getNumSamples();
        gamma = juce::jlimit (0.1f, 4.0f, gamma);

        // 1. Write incoming samples into ring buffer
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* inData = inputBuffer.getReadPointer (ch);
            float* ringData = ringBuffer.getWritePointer (ch);

            for (int i = 0; i < numSamples; ++i)
            {
                int wPos = (writePos + i) % bufferSize;
                ringData[wPos] = inData[i];
            }
        }
        writePos = (writePos + numSamples) % bufferSize;

        // 2. Read with fractional step based on gamma
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* ringData = ringBuffer.getReadPointer (ch);
            float* outData = outputBuffer.getWritePointer (ch);

            float readPosFloat = static_cast<float>((writePos - numSamples + bufferSize) % bufferSize);

            for (int i = 0; i < outSamples; ++i)
            {
                int baseIndex = static_cast<int>(std::floor (readPosFloat));
                float frac = readPosFloat - baseIndex;

                int i0 = (baseIndex - 1 + bufferSize) % bufferSize;
                int i1 = (baseIndex + bufferSize) % bufferSize;
                int i2 = (baseIndex + 1 + bufferSize) % bufferSize;
                int i3 = (baseIndex + 2 + bufferSize) % bufferSize;

                outData[i] = interpolateHermite (ringData[i0], ringData[i1], ringData[i2], ringData[i3], frac);
                readPosFloat += gamma;

                if (readPosFloat >= bufferSize)
                    readPosFloat -= bufferSize;
            }
        }
    }

private:
    double sampleRate = 44100.0;
    int numChannels = 2;
    int bufferSize = 8192;
    juce::AudioBuffer<float> ringBuffer;
    int writePos = 0;
};

} // namespace time_dilation
