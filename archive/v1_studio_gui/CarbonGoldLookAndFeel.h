#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "FontManager.h"

namespace time_dilation
{

class CarbonGoldLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CarbonGoldLookAndFeel();
    ~CarbonGoldLookAndFeel() override = default;

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font& font) override;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    void drawTabButton (juce::TabBarButton& button, juce::Graphics& g,
                        bool isMouseOver, bool isMouseDown) override;
};

} // namespace time_dilation
