#include "../RelativisticCanvasComponent.h"
#include <cmath>

namespace time_dilation
{

void RelativisticCanvasComponent::drawCable (juce::Graphics& g, juce::Point<float> p1, juce::Point<float> p2, NodePortType type, bool isFeedbackLoop)
{
    juce::Colour cableColour = juce::Colour (0xff06b6d4); // Audio = Cyan
    if (type == NodePortType::Time)    cableColour = juce::Colour (0xffa855f7); // Time = Purple
    if (type == NodePortType::Control) cableColour = juce::Colour (0xfff59e0b); // Control = Amber
    if (type == NodePortType::Message) cableColour = juce::Colour (0xff10b981); // Message = Emerald Green
    if (isFeedbackLoop)                cableColour = juce::Colour (0xffef4444); // Feedback Warning = Neon Red

    if (cableStyle == CableStyle::Straight)
    {
        // Drop Shadow
        g.setColour (juce::Colour (0x66000000));
        g.drawLine (p1.x + 2.0f, p1.y + 3.0f, p2.x + 2.0f, p2.y + 3.0f, 4.0f);

        // Core Cable
        g.setColour (cableColour);
        g.drawLine (p1.x, p1.y, p2.x, p2.y, isFeedbackLoop ? 3.5f : 2.5f);
        return;
    }

    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;

    juce::Path path;
    path.startNewSubPath (p1);

    if (cableStyle == CableStyle::Organic)
    {
        // Natural Gravity Sag Control Points
        float sagFactor = std::max (50.0f, std::abs (dy) * 0.65f);
        float bowFactor = (dy < 0.0f) ? (dx >= 0.0f ? 90.0f : -90.0f) : (dx * 0.15f);

        juce::Point<float> c1 { p1.x + bowFactor, p1.y + sagFactor };
        juce::Point<float> c2 { p2.x - bowFactor, p2.y - sagFactor };

        path.cubicTo (c1, c2, p2);
    }
    else // SmoothS
    {
        float deltaY = std::abs (dy) * 0.5f + 30.0f;
        path.cubicTo (p1.x, p1.y + deltaY, p2.x, p2.y - deltaY, p2.x, p2.y);
    }

    // 1. Render Drop Shadow
    juce::Path shadowPath = path;
    shadowPath.applyTransform (juce::AffineTransform::translation (2.0f, 3.0f));
    g.setColour (juce::Colour (0x55000000));
    g.strokePath (shadowPath, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Render Anti-Aliased Core Cable
    g.setColour (cableColour);
    g.strokePath (path, juce::PathStrokeType (isFeedbackLoop ? 3.5f : 2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 3. Render Feedback Warning Badge on Cable Midpoint
    if (isFeedbackLoop && (p1.x > p2.x || p1.y > p2.y))
    {
        juce::Point<float> mid = path.getPointAlongPath (path.getLength() * 0.5f);
        g.setColour (juce::Colour (0xff7f1d1d));
        g.fillRoundedRectangle (mid.x - 70.0f, mid.y - 10.0f, 140.0f, 20.0f, 4.0f);
        g.setColour (juce::Colour (0xffef4444));
        g.drawRoundedRectangle (mid.x - 70.0f, mid.y - 10.0f, 140.0f, 20.0f, 4.0f, 1.0f);
        g.setColour (juce::Colour (0xfff8fafc));
        g.setFont (FontManager::getInstance().getOxaniumFont (11.0f, true));
        g.drawText ("[FEEDBACK: 1-SMP DELAY]", mid.x - 70.0f, mid.y - 10.0f, 140.0f, 20.0f, juce::Justification::centred);
    }
}

} // namespace time_dilation
