#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

namespace time_dilation
{

class FontManager
{
public:
    static FontManager& getInstance()
    {
        static FontManager instance;
        return instance;
    }

    void loadFonts (const juce::File& fontDir)
    {
        if (BinaryData::OxaniumBold_ttfSize > 0)
        {
            oxaniumBoldTypeface = juce::Typeface::createSystemTypefaceFor (BinaryData::OxaniumBold_ttf, BinaryData::OxaniumBold_ttfSize);
        }
        if (BinaryData::OxaniumRegular_ttfSize > 0)
        {
            oxaniumRegularTypeface = juce::Typeface::createSystemTypefaceFor (BinaryData::OxaniumRegular_ttf, BinaryData::OxaniumRegular_ttfSize);
        }

        if (oxaniumBoldTypeface == nullptr)
        {
            auto boldFile = fontDir.getChildFile ("Oxanium-Bold.ttf");
            if (boldFile.existsAsFile())
            {
                juce::MemoryBlock mb;
                boldFile.loadFileAsData (mb);
                oxaniumBoldTypeface = juce::Typeface::createSystemTypefaceFor (mb.getData(), mb.getSize());
            }
        }

        if (oxaniumRegularTypeface == nullptr)
        {
            auto regFile = fontDir.getChildFile ("Oxanium-Regular.ttf");
            if (regFile.existsAsFile())
            {
                juce::MemoryBlock mb;
                regFile.loadFileAsData (mb);
                oxaniumRegularTypeface = juce::Typeface::createSystemTypefaceFor (mb.getData(), mb.getSize());
            }
        }
    }

    juce::Typeface::Ptr getOxaniumBoldTypeface()
    {
        if (oxaniumBoldTypeface == nullptr && BinaryData::OxaniumBold_ttfSize > 0)
        {
            oxaniumBoldTypeface = juce::Typeface::createSystemTypefaceFor (BinaryData::OxaniumBold_ttf, BinaryData::OxaniumBold_ttfSize);
        }
        return oxaniumBoldTypeface;
    }

    juce::Typeface::Ptr getOxaniumRegularTypeface()
    {
        if (oxaniumRegularTypeface == nullptr && BinaryData::OxaniumRegular_ttfSize > 0)
        {
            oxaniumRegularTypeface = juce::Typeface::createSystemTypefaceFor (BinaryData::OxaniumRegular_ttf, BinaryData::OxaniumRegular_ttfSize);
        }
        return oxaniumRegularTypeface;
    }

    juce::Font getOxaniumFont (float height = 14.0f, bool bold = false)
    {
        auto tf = bold ? getOxaniumBoldTypeface() : getOxaniumRegularTypeface();
        if (tf != nullptr)
        {
            return juce::Font (juce::FontOptions (tf).withHeight (height));
        }
        return juce::FontOptions ("Oxanium", height, bold ? juce::Font::bold : juce::Font::plain);
    }

    juce::Font getSciFiFont (float height = 14.0f, int style = juce::Font::plain)
    {
        return getOxaniumFont (height, style == juce::Font::bold);
    }

    juce::Font getFallbackFont (float height = 12.0f, int style = juce::Font::plain)
    {
        return getOxaniumFont (height, style == juce::Font::bold);
    }

private:
    FontManager() = default;
    juce::Typeface::Ptr oxaniumBoldTypeface;
    juce::Typeface::Ptr oxaniumRegularTypeface;

    JUCE_DECLARE_NON_COPYABLE (FontManager)
};

} // namespace time_dilation
