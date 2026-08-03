#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <vector>
#include <string>

namespace time_dilation
{

enum class TimeMode
{
    BeatMode,  // Absolute metric beats (Tidal 2.0 beatMode)
    CycleMode  // Normalized cyclic time (TidalCycles)
};

struct SolkattuEvent
{
    float beatPosition = 0.0f; // Beat index
    float durationBeats = 1.0f;
    int syllableIndex = 0;    // 0: Tha, 1: Dhi, 2: Thom, 3: Nam, 4: Tarikita, 5: Thakadimi
    int countValue = 1;       // 1, 2, 3...
};

class TidalBeatEngine
{
public:
    TidalBeatEngine();
    ~TidalBeatEngine() = default;

    void prepare (double sampleRate, int samplesPerBlock);
    void processBlock (juce::AudioBuffer<float>& buffer);

    // Mode & Transport
    void setTimeMode (TimeMode mode) { currentMode = mode; }
    TimeMode getTimeMode() const { return currentMode; }

    void setBpm (double bpm) { currentBpm = bpm; }
    double getBpm() const { return currentBpm; }

    void setMotifPattern (int motifId);
    int getMotifPattern() const { return currentMotifId; }

    void togglePlay (bool shouldPlay) { playing = shouldPlay; }
    bool isPlaying() const { return playing; }

    float getCurrentBeatPosition() const { return currentBeatPos; }
    const std::vector<SolkattuEvent>& getActiveEvents() const { return currentEvents; }

    // Synthesize percussion voice
    void renderPercussion (int syllableIdx, juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

private:
    double sampleRate = 44100.0;
    TimeMode currentMode = TimeMode::BeatMode;
    double currentBpm = 135.0;
    bool playing = true;

    float currentBeatPos = 0.0f;
    int currentMotifId = 0; // 0: 123, 1: 321, 2: 123 321, 3: 333 222 111 321

    std::vector<SolkattuEvent> currentEvents;

    void generateEventsForMotif();
    void renderClick (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
};

} // namespace time_dilation
