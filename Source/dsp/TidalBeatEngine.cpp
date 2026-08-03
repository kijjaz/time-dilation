#include "TidalBeatEngine.h"
#include <cmath>

namespace time_dilation
{

TidalBeatEngine::TidalBeatEngine()
{
    generateEventsForMotif();
}

void TidalBeatEngine::prepare (double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
}

void TidalBeatEngine::setMotifPattern (int motifId)
{
    currentMotifId = motifId;
    generateEventsForMotif();
}

void TidalBeatEngine::generateEventsForMotif()
{
    currentEvents.clear();

    // Carnatic Solkattu numerical motif structures:
    // 0: 1-2-3 (Tha, Dhi, Thom)
    // 1: 3-2-1 (Reverse)
    // 2: 123 321 123 321 (Alternating)
    // 3: 333 222 111 321 (Carnatic Variation 7)

    std::vector<int> counts;
    if (currentMotifId == 0) counts = { 1, 2, 3 };
    else if (currentMotifId == 1) counts = { 3, 2, 1 };
    else if (currentMotifId == 2) counts = { 1, 2, 3, 3, 2, 1 };
    else counts = { 3, 3, 3, 2, 2, 2, 1, 1, 1, 3, 2, 1 };

    float beatCursor = 0.0f;
    int sylIdx = 0;

    for (int count : counts)
    {
        // Add "Tha" / "Dhi" / "Thom" / "Nam" repeats
        for (int r = 0; r < count; ++r)
        {
            SolkattuEvent ev;
            ev.beatPosition = beatCursor;
            ev.durationBeats = 1.0f;
            ev.syllableIndex = sylIdx % 4;
            ev.countValue = count;
            currentEvents.push_back (ev);

            beatCursor += 1.0f;
            beatCursor += 0.5f; // Gap (-)
        }

        // Add "Thakadimi" / "Tarikita" phrase cadence
        SolkattuEvent cadenceEv;
        cadenceEv.beatPosition = beatCursor;
        cadenceEv.durationBeats = 2.0f;
        cadenceEv.syllableIndex = 5; // Thakadimi
        cadenceEv.countValue = count;
        currentEvents.push_back (cadenceEv);

        beatCursor += 2.0f;
        sylIdx++;
    }
}

void TidalBeatEngine::renderPercussion (int syllableIdx, juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    // Frequencies & Timbres for Solkattu Syllables:
    // 0: Tha (Mridangam Bass - 85 Hz)
    // 1: Dhi (Mridangam High Ring - 220 Hz)
    // 2: Thom (Deep Resonance - 110 Hz)
    // 3: Nam (Crisp Edge - 330 Hz)
    // 4: Tarikita (Fast Roll - 440 Hz)
    // 5: Thakadimi (Slap - 180 Hz)

    float baseFreq = 85.0f;
    if (syllableIdx == 1) baseFreq = 220.0f;
    else if (syllableIdx == 2) baseFreq = 110.0f;
    else if (syllableIdx == 3) baseFreq = 330.0f;
    else if (syllableIdx == 4) baseFreq = 440.0f;
    else if (syllableIdx == 5) baseFreq = 180.0f;

    float phase = 0.0f;
    float phaseInc = static_cast<float>(2.0 * juce::MathConstants<double>::pi * baseFreq / sampleRate);

    for (int s = 0; s < numSamples; ++s)
    {
        int sampleIdx = startSample + s;
        if (sampleIdx >= buffer.getNumSamples()) break;

        float env = std::exp (-static_cast<float>(s) * 0.008f);
        float sampleVal = std::sin (phase) * env * 0.5f;

        left[sampleIdx] += sampleVal;
        if (right != nullptr) right[sampleIdx] += sampleVal;

        phase += phaseInc;
    }
}

void TidalBeatEngine::renderClick (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    for (int s = 0; s < std::min (numSamples, 120); ++s)
    {
        int sampleIdx = startSample + s;
        if (sampleIdx >= buffer.getNumSamples()) break;

        float clickVal = (s < 60 ? 0.3f : -0.3f) * (1.0f - static_cast<float>(s) / 120.0f);
        left[sampleIdx] += clickVal;
        if (right != nullptr) right[sampleIdx] += clickVal;
    }
}

void TidalBeatEngine::processBlock (juce::AudioBuffer<float>& buffer)
{
    if (!playing) return;

    const int numSamples = buffer.getNumSamples();
    const double beatsPerSecond = currentBpm / 60.0;
    const double beatsPerSample = beatsPerSecond / sampleRate;

    float nextBeatPos = currentBeatPos + static_cast<float>(numSamples * beatsPerSample);
    float totalPatternBeats = 16.0f;

    // Trigger Solkattu events falling within this block
    for (const auto& ev : currentEvents)
    {
        float evBeat = ev.beatPosition;
        if (currentMode == TimeMode::CycleMode)
        {
            // Normalize to [0, 1) cycle
            evBeat = std::fmod (evBeat, 8.0f);
        }

        if (evBeat >= currentBeatPos && evBeat < nextBeatPos)
        {
            int offsetSamples = static_cast<int>((evBeat - currentBeatPos) / beatsPerSample);
            offsetSamples = juce::jlimit (0, numSamples - 1, offsetSamples);
            renderPercussion (ev.syllableIndex, buffer, offsetSamples, numSamples - offsetSamples);
        }
    }

    // Trigger Beat Click Pulse
    int currentBeatInt = static_cast<int>(currentBeatPos);
    int nextBeatInt = static_cast<int>(nextBeatPos);

    if (nextBeatInt > currentBeatInt)
    {
        int offsetSamples = static_cast<int>((nextBeatInt - currentBeatPos) / beatsPerSample);
        offsetSamples = juce::jlimit (0, numSamples - 1, offsetSamples);
        renderClick (buffer, offsetSamples, numSamples - offsetSamples);
    }

    currentBeatPos = std::fmod (nextBeatPos, totalPatternBeats);
}

} // namespace time_dilation
