#include "CarbonGoldLookAndFeel.h"

namespace time_dilation
{

CarbonGoldLookAndFeel::CarbonGoldLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff121824));
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
    setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff8fafc));
    setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xfff59e0b));

    setColour (juce::TabbedButtonBar::tabTextColourId, juce::Colour (0xffcbd5e1));
    setColour (juce::TabbedButtonBar::frontTextColourId, juce::Colour (0xfff59e0b));
}

juce::Typeface::Ptr CarbonGoldLookAndFeel::getTypefaceForFont (const juce::Font& font)
{
    auto smoochTf = FontManager::getInstance().getSmoochTypeface();
    auto notoTf = FontManager::getInstance().getNotoTypeface();

    if (smoochTf != nullptr) return smoochTf;
    if (notoTf != nullptr) return notoTf;

    return LookAndFeel_V4::getTypefaceForFont (font);
}

void CarbonGoldLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                float sliderPosProportional, float rotaryStartAngle,
                                                float rotaryEndAngle, juce::Slider& /*slider*/)
{
    auto radius = (float) std::min (width, height) / 2.0f - 4.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;

    // Background track arc
    g.setColour (juce::Colour (0xff1e293b));
    juce::Path bgArc;
    bgArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.strokePath (bgArc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Active track arc
    auto currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    g.setColour (juce::Colour (0xfff59e0b)); // Gold
    juce::Path activeArc;
    activeArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, currentAngle, true);
    g.strokePath (activeArc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Dial body
    g.setColour (juce::Colour (0xff0f172a));
    g.fillEllipse (rx + 3.0f, ry + 3.0f, rw - 6.0f, rw - 6.0f);

    // Pointer line
    juce::Path p;
    auto pointerLength = radius * 0.6f;
    p.addRectangle (-1.5f, -pointerLength, 3.0f, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (currentAngle).translated (centreX, centreY));
    g.setColour (juce::Colour (0xff38bdf8)); // Cyan glowing pointer
    g.fillPath (p);
}

void CarbonGoldLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                                float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                                const juce::Slider::SliderStyle style, juce::Slider& /*slider*/)
{
    if (style == juce::Slider::LinearVertical)
    {
        float trackW = 6.0f;
        float trackX = (float) x + ((float) width - trackW) * 0.5f;

        g.setColour (juce::Colour (0xff1e293b));
        g.fillRoundedRectangle (trackX, (float) y, trackW, (float) height, 3.0f);

        g.setColour (juce::Colour (0xff38bdf8));
        g.fillRoundedRectangle (trackX, sliderPos, trackW, (float) (y + height) - sliderPos, 3.0f);

        // Fader Thumb
        float thumbW = 20.0f;
        float thumbH = 12.0f;
        float thumbX = (float) x + ((float) width - thumbW) * 0.5f;

        g.setColour (juce::Colour (0xff0f172a));
        g.fillRoundedRectangle (thumbX, sliderPos - thumbH * 0.5f, thumbW, thumbH, 3.0f);

        g.setColour (juce::Colour (0xfff59e0b));
        g.drawRoundedRectangle (thumbX, sliderPos - thumbH * 0.5f, thumbW, thumbH, 3.0f, 1.5f);
    }
    else
    {
        float trackH = 6.0f;
        float trackY = (float) y + ((float) height - trackH) * 0.5f;

        g.setColour (juce::Colour (0xff1e293b));
        g.fillRoundedRectangle ((float) x, trackY, (float) width, trackH, 3.0f);

        g.setColour (juce::Colour (0xfff59e0b));
        g.fillRoundedRectangle ((float) x, trackY, sliderPos - (float) x, trackH, 3.0f);

        // Horizontal Thumb
        float thumbW = 12.0f;
        float thumbH = 18.0f;

        g.setColour (juce::Colour (0xff0f172a));
        g.fillRoundedRectangle (sliderPos - thumbW * 0.5f, (float) y + ((float) height - thumbH) * 0.5f, thumbW, thumbH, 3.0f);

        g.setColour (juce::Colour (0xff38bdf8));
        g.drawRoundedRectangle (sliderPos - thumbW * 0.5f, (float) y + ((float) height - thumbH) * 0.5f, thumbW, thumbH, 3.0f, 1.5f);
    }
}

void CarbonGoldLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                   const juce::Colour& backgroundColour,
                                                   bool shouldDrawButtonAsHighlighted,
                                                   bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = 4.0f;

    auto baseColor = backgroundColour;
    if (shouldDrawButtonAsDown) baseColor = baseColor.darker (0.2f);
    else if (shouldDrawButtonAsHighlighted) baseColor = baseColor.brighter (0.2f);

    g.setColour (baseColor);
    g.fillRoundedRectangle (bounds, cornerSize);

    if (button.getToggleState())
    {
        g.setColour (juce::Colour (0xfff59e0b));
        g.drawRoundedRectangle (bounds.reduced (0.5f), cornerSize, 1.5f);
    }
    else
    {
        g.setColour (juce::Colour (0xff334155));
        g.drawRoundedRectangle (bounds.reduced (0.5f), cornerSize, 1.0f);
    }
}

void CarbonGoldLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                           int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                           juce::ComboBox& /*box*/)
{
    auto cornerSize = 4.0f;
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRoundedRectangle (0.0f, 0.0f, (float) width, (float) height, cornerSize);

    g.setColour (juce::Colour (0xff334155));
    g.drawRoundedRectangle (0.5f, 0.5f, (float) width - 1.0f, (float) height - 1.0f, cornerSize, 1.0f);

    // Arrow triangle
    juce::Path arrow;
    float arrowX = (float) width - 16.0f;
    float arrowY = (float) height * 0.5f - 2.0f;
    arrow.addTriangle (arrowX, arrowY, arrowX + 8.0f, arrowY, arrowX + 4.0f, arrowY + 5.0f);
    g.setColour (juce::Colour (0xfff59e0b));
    g.fillPath (arrow);
}

void CarbonGoldLookAndFeel::drawTabButton (juce::TabBarButton& button, juce::Graphics& g,
                                           bool isMouseOver, bool /*isMouseDown*/)
{
    auto area = button.getActiveArea();
    bool isSelected = button.isFrontTab();

    g.setColour (isSelected ? juce::Colour (0xff1e293b) : (isMouseOver ? juce::Colour (0xff161e2e) : juce::Colour (0xff0f172a)));
    g.fillRoundedRectangle (area.toFloat(), 4.0f);

    if (isSelected)
    {
        g.setColour (juce::Colour (0xfff59e0b));
        g.fillRect (area.getX(), area.getBottom() - 3, area.getWidth(), 3);
    }

    g.setColour (isSelected ? juce::Colour (0xfff59e0b) : juce::Colour (0xff94a3b8));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText (button.getButtonText(), area, juce::Justification::centred);
}

} // namespace time_dilation
