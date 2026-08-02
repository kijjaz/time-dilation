#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace time_dilation
{

class PolySynthSound : public juce::SynthesiserSound
{
public:
    PolySynthSound() = default;
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class PolySynthVoice : public juce::SynthesiserVoice
{
public:
    PolySynthVoice() = default;

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<PolySynthSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        currentPitch = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        level = velocity;
        tailOff = 0.0;
        env = 0.0f;
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            if (tailOff == 0.0)
                tailOff = 1.0;
        }
        else
        {
            clearCurrentNote();
            level = 0.0f;
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (currentPitch > 0)
        {
            const double cyclesPerSample = currentPitch / getSampleRate();
            const float angleDelta = static_cast<float>(cyclesPerSample * 2.0 * juce::MathConstants<double>::pi);

            while (--numSamples >= 0)
            {
                float currentSample = std::sin (currentAngle) * level * 0.4f;

                if (tailOff > 0.0)
                {
                    currentSample *= static_cast<float>(tailOff);
                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();
                        break;
                    }
                }

                for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
                    outputBuffer.addSample (channel, startSample, currentSample);

                currentAngle += angleDelta;
                ++startSample;
            }
        }
    }

private:
    double currentPitch = 0.0;
    double currentAngle = 0.0;
    double tailOff = 0.0;
    float level = 0.0f;
    float env = 0.0f;
};

} // namespace time_dilation
