#include "SpacetimeVisualizerComponent.h"

namespace time_dilation
{

SpacetimeVisualizerComponent::SpacetimeVisualizerComponent (TimeDilationEngine& e)
    : engine (e)
{
    audioData.resize (512, 0.0f);
    startTimerHz (30); // 30 FPS visual update
}

SpacetimeVisualizerComponent::~SpacetimeVisualizerComponent()
{
    stopTimer();
}

void SpacetimeVisualizerComponent::timerCallback()
{
    engine.getVisualizerData (audioData.data(), static_cast<int>(audioData.size()));
    phase += 0.05f * engine.getMasterDilation();
    repaint();
}

void SpacetimeVisualizerComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();

    // 1. Dark Cosmic Background
    g.fillAll (juce::Colour (0xff0a0d14));

    // Panel Header Title
    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("SPACETIME GRAVITATIONAL VISUALIZER (NATIVE C++ / JUCE)", 10, 6, width - 20, 20, juce::Justification::left);

    // 2. Render Deforming 3D Perspective Grid
    g.setColour (juce::Colour (0x33263147));
    const float horizonY = height * 0.45f;
    const int cols = 16;
    const int rows = 8;

    for (int i = 0; i <= cols; ++i)
    {
        float xPercent = (float) i / (float) cols;
        float startX = width * 0.5f + (xPercent - 0.5f) * width * 0.2f;
        float endX = width * 0.5f + (xPercent - 0.5f) * width * 1.4f;
        g.drawLine (startX, horizonY, endX, height, 1.0f);
    }

    for (int j = 0; j <= rows; ++j)
    {
        float yRatio = std::pow ((float) j / (float) rows, 1.8f);
        float y = horizonY + yRatio * (height - horizonY);

        float warp = std::sin (phase + (float) j * 0.5f) * 10.0f * engine.getMasterDilation();
        g.setColour (juce::Colour (0x4406b6d4));
        g.drawLine (0.0f, y + warp, width, y - warp, 1.0f);
    }

    // 3. Render Oscilloscope Real-time Audio Waveform Path
    juce::Path wavePath;
    const float sliceWidth = width / (float) audioData.size();
    float x = 0.0f;

    for (size_t i = 0; i < audioData.size(); ++i)
    {
        float sample = audioData[i];
        float y = horizonY + sample * (height * 0.35f);

        if (i == 0)
            wavePath.startNewSubPath (x, y);
        else
            wavePath.lineTo (x, y);

        x += sliceWidth;
    }

    g.setColour (engine.isPlaying() ? juce::Colour (0xfff59e0b) : juce::Colour (0xff3b82f6));
    g.strokePath (wavePath, juce::PathStrokeType (2.0f));

    // Center Singularity Core Graphic
    g.setColour (juce::Colour (0xff06b6d4));
    float pulse = (std::sin (phase * 2.0f) * 4.0f + 16.0f);
    g.drawEllipse (width * 0.5f - pulse * 0.5f, horizonY - pulse * 0.5f, pulse, pulse, 2.0f);
}

void SpacetimeVisualizerComponent::resized()
{
}

} // namespace time_dilation
