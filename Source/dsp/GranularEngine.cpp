#include "GranularEngine.h"

namespace time_dilation
{

GranularEngine::GranularEngine()
{
    formatManager.registerBasicFormats();
    grains.resize (maxGrains);
}

void GranularEngine::prepare (double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
}

bool GranularEngine::loadSample (const juce::File& file)
{
    if (!file.existsAsFile()) return false;

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader != nullptr)
    {
        sampleBuffer.setSize (static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
        reader->read (&sampleBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
        sampleLoaded = true;
        return true;
    }
    return false;
}

float GranularEngine::getHanningWindow (float progress)
{
    return 0.5f * (1.0f - std::cos (2.0f * juce::MathConstants<float>::pi * progress));
}

void GranularEngine::spawnGrain()
{
    if (!sampleLoaded || sampleBuffer.getNumSamples() <= 0) return;

    for (auto& g : grains)
    {
        if (!g.active)
        {
            g.active = true;
            g.ageSamples = 0.0f;

            // Calculate Grain Duration
            g.durationSamples = std::max (441.0f, static_cast<float>(sampleRate * (grainSizeMs / 1000.0f)));

            // Random Position Spray Offset
            std::uniform_real_distribution<float> distSpray (-positionSpray, positionSpray);
            float normPos = juce::jlimit (0.0f, 1.0f, scanPositionNorm + distSpray (randomGen));
            g.samplePosition = normPos * (sampleBuffer.getNumSamples() - 1);

            // Random Pitch Jitter Offset
            std::uniform_real_distribution<float> distPitch (-pitchJitter, pitchJitter);
            float totalPitch = pitchSemitones + distPitch (randomGen);
            g.playSpeed = std::pow (2.0, totalPitch / 12.0);

            // Random Stereo Panning Offset
            std::uniform_real_distribution<float> distPan (-panSpread, panSpread);
            float pan = juce::jlimit (-1.0f, 1.0f, distPan (randomGen));
            g.panLeft = std::cos ((pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi);
            g.panRight = std::sin ((pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi);

            break;
        }
    }
}

void GranularEngine::processBlock (juce::AudioBuffer<float>& buffer)
{
    if (!sampleLoaded || sampleBuffer.getNumSamples() <= 0) return;

    const int numSamples = buffer.getNumSamples();
    const double grainIntervalSamples = sampleRate / std::max (1.0f, grainDensityHz);

    auto* leftOut = buffer.getWritePointer (0);
    auto* rightOut = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    const float* srcLeft = sampleBuffer.getReadPointer (0);
    const float* srcRight = sampleBuffer.getNumChannels() > 1 ? sampleBuffer.getReadPointer (1) : srcLeft;
    const int totalSrcSamples = sampleBuffer.getNumSamples();

    for (int s = 0; s < numSamples; ++s)
    {
        samplesSinceLastGrain += 1.0;
        if (samplesSinceLastGrain >= grainIntervalSamples)
        {
            samplesSinceLastGrain -= grainIntervalSamples;
            spawnGrain();
        }

        float outL = 0.0f;
        float outR = 0.0f;

        for (auto& g : grains)
        {
            if (g.active)
            {
                float progress = g.ageSamples / g.durationSamples;
                if (progress >= 1.0f)
                {
                    g.active = false;
                    continue;
                }

                float win = getHanningWindow (progress);
                int idx = static_cast<int>(g.samplePosition);
                if (idx >= 0 && idx < totalSrcSamples)
                {
                    float sampleL = srcLeft[idx] * win;
                    float sampleR = srcRight[idx] * win;

                    outL += sampleL * g.panLeft;
                    outR += sampleR * g.panRight;
                }

                g.samplePosition += g.playSpeed;
                if (g.samplePosition >= totalSrcSamples) g.samplePosition -= totalSrcSamples;
                g.ageSamples += 1.0f;
            }
        }

        leftOut[s] += outL * 0.4f;
        if (rightOut != nullptr) rightOut[s] += outR * 0.4f;
    }
}

} // namespace time_dilation
