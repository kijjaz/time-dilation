#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <vector>
#include <random>

namespace time_dilation
{

struct Grain
{
    bool active = false;
    double samplePosition = 0.0;
    double playSpeed = 1.0;
    float currentEnvelopeSample = 0.0f;
    float durationSamples = 4410.0f;
    float ageSamples = 0.0f;
    float panLeft = 0.707f;
    float panRight = 0.707f;
};

class GranularEngine
{
public:
    GranularEngine();
    ~GranularEngine() = default;

    void prepare (double sampleRate, int samplesPerBlock);
    void processBlock (juce::AudioBuffer<float>& buffer);

    bool loadSample (const juce::File& file);
    const juce::AudioBuffer<float>& getSampleBuffer() const { return sampleBuffer; }
    bool hasSample() const { return sampleLoaded; }

    // Granular Parameters
    void setGrainSizeMs (float sizeMs) { grainSizeMs = sizeMs; }
    void setGrainDensity (float densityHz) { grainDensityHz = densityHz; }
    void setScanPosition (float normPos) { scanPositionNorm = juce::jlimit (0.0f, 1.0f, normPos); }
    void setPositionSpray (float spray) { positionSpray = spray; }
    void setPitchSemitones (float semi) { pitchSemitones = semi; }
    void setPitchJitter (float jitter) { pitchJitter = jitter; }
    void setStereoPanSpread (float spread) { panSpread = spread; }
    void setReverbLevel (float mix) { reverbMix = mix; }

    float getScanPosition() const { return scanPositionNorm; }

private:
    double sampleRate = 44100.0;
    juce::AudioFormatManager formatManager;

    juce::AudioBuffer<float> sampleBuffer;
    bool sampleLoaded = false;

    float grainSizeMs = 80.0f;
    float grainDensityHz = 25.0f;
    float scanPositionNorm = 0.3f;
    float positionSpray = 0.1f;
    float pitchSemitones = 0.0f;
    float pitchJitter = 0.2f;
    float panSpread = 0.5f;
    float reverbMix = 0.3f;

    std::vector<Grain> grains;
    static constexpr int maxGrains = 64;

    double samplesSinceLastGrain = 0.0;
    std::mt19937 randomGen { 1337 };

    void spawnGrain();
    float getHanningWindow (float progress);
};

} // namespace time_dilation
