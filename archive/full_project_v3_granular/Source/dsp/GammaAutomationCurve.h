#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <algorithm>

namespace time_dilation
{

struct AutomationPoint
{
    double timeInSeconds = 0.0;
    float gamma = 1.0f; // 0.1x to 4.0x
};

class GammaAutomationCurve
{
public:
    GammaAutomationCurve()
    {
        // Default curve with constant gamma = 1.0
        points.push_back ({ 0.0, 1.0f });
        points.push_back ({ 60.0, 1.0f });
    }

    void addPoint (double timeInSeconds, float gamma)
    {
        gamma = juce::jlimit (0.1f, 4.0f, gamma);
        points.push_back ({ timeInSeconds, gamma });
        sortPoints();
    }

    void clearPoints()
    {
        points.clear();
        points.push_back ({ 0.0, 1.0f });
    }

    const std::vector<AutomationPoint>& getPoints() const { return points; }

    float getGammaAtTime (double timeInSeconds) const
    {
        if (points.empty()) return 1.0f;
        if (timeInSeconds <= points.front().timeInSeconds) return points.front().gamma;
        if (timeInSeconds >= points.back().timeInSeconds) return points.back().gamma;

        // Linear interpolation between adjacent control points
        for (size_t i = 0; i < points.size() - 1; ++i)
        {
            if (timeInSeconds >= points[i].timeInSeconds && timeInSeconds <= points[i + 1].timeInSeconds)
            {
                double t0 = points[i].timeInSeconds;
                double t1 = points[i + 1].timeInSeconds;
                float g0 = points[i].gamma;
                float g1 = points[i + 1].gamma;

                double alpha = (timeInSeconds - t0) / (t1 - t0);
                return static_cast<float>(g0 + alpha * (g1 - g0));
            }
        }

        return 1.0f;
    }

private:
    std::vector<AutomationPoint> points;

    void sortPoints()
    {
        std::sort (points.begin(), points.end(), [] (const AutomationPoint& a, const AutomationPoint& b) {
            return a.timeInSeconds < b.timeInSeconds;
        });
    }
};

} // namespace time_dilation
