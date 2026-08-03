#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <map>

namespace time_dilation
{

struct GammaTap
{
    int tapId = 0;
    juce::String name;
    int sourceTrackIndex = 0;
    float currentGammaValue = 1.0f;
    float currentAmplitudeValue = 0.0f;
};

class GammaTapMatrix
{
public:
    GammaTapMatrix() = default;

    void registerTap (int tapId, const juce::String& name, int sourceTrackIndex)
    {
        GammaTap tap;
        tap.tapId = tapId;
        tap.name = name;
        tap.sourceTrackIndex = sourceTrackIndex;
        taps[tapId] = tap;
    }

    void updateTapValues (int tapId, float gamma, float amp)
    {
        if (taps.find (tapId) != taps.end())
        {
            taps[tapId].currentGammaValue = gamma;
            taps[tapId].currentAmplitudeValue = amp;
        }
    }

    float getTapGamma (int tapId) const
    {
        auto it = taps.find (tapId);
        if (it != taps.end())
            return it->second.currentGammaValue;
        return 1.0f;
    }

    float getTapAmplitude (int tapId) const
    {
        auto it = taps.find (tapId);
        if (it != taps.end())
            return it->second.currentAmplitudeValue;
        return 0.0f;
    }

    const std::map<int, GammaTap>& getTaps() const { return taps; }

private:
    std::map<int, GammaTap> taps;
};

} // namespace time_dilation
