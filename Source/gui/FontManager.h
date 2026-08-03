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

    juce::Typeface::Ptr getOxaniumBoldTypeface()
    {
        if (oxaniumBoldTypeface == nullptr)
        {
            ensureFontsLoaded();
        }
        return oxaniumBoldTypeface;
    }

    juce::Typeface::Ptr getOxaniumRegularTypeface()
    {
        if (oxaniumRegularTypeface == nullptr)
        {
            ensureFontsLoaded();
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

    void loadFonts (const juce::File& fontDir)
    {
        ensureFontsLoaded (fontDir);
    }

private:
    FontManager()
    {
        ensureFontsLoaded();
    }

    void ensureFontsLoaded (const juce::File& fontDir = juce::File())
    {
        if (oxaniumBoldTypeface != nullptr && oxaniumRegularTypeface != nullptr)
            return;

        // 1. Try BinaryData first (Embedded into binary executable)
        int boldSize = 0;
        const char* boldData = BinaryData::getNamedResource ("OxaniumBold_ttf", boldSize);
        if (boldData != nullptr && boldSize > 0)
        {
            oxaniumBoldTypeface = juce::Typeface::createSystemTypefaceFor (boldData, boldSize);
        }

        int regSize = 0;
        const char* regData = BinaryData::getNamedResource ("OxaniumRegular_ttf", regSize);
        if (regData != nullptr && regSize > 0)
        {
            oxaniumRegularTypeface = juce::Typeface::createSystemTypefaceFor (regData, regSize);
        }

        // 2. Try disk fallback if embedded binary not present
        if (oxaniumBoldTypeface == nullptr)
        {
            juce::File bFile = fontDir.getChildFile ("Oxanium-Bold.ttf");
            if (!bFile.existsAsFile())
            {
                bFile = juce::File::getCurrentWorkingDirectory().getChildFile ("Source/assets/fonts/Oxanium-Bold.ttf");
            }
            if (bFile.existsAsFile())
            {
                juce::MemoryBlock mb;
                bFile.loadFileAsData (mb);
                oxaniumBoldTypeface = juce::Typeface::createSystemTypefaceFor (mb.getData(), mb.getSize());
            }
        }

        if (oxaniumRegularTypeface == nullptr)
        {
            juce::File rFile = fontDir.getChildFile ("Oxanium-Regular.ttf");
            if (!rFile.existsAsFile())
            {
                rFile = juce::File::getCurrentWorkingDirectory().getChildFile ("Source/assets/fonts/Oxanium-Regular.ttf");
            }
            if (rFile.existsAsFile())
            {
                juce::MemoryBlock mb;
                rFile.loadFileAsData (mb);
                oxaniumRegularTypeface = juce::Typeface::createSystemTypefaceFor (mb.getData(), mb.getSize());
            }
        }
    }

    juce::Typeface::Ptr oxaniumBoldTypeface;
    juce::Typeface::Ptr oxaniumRegularTypeface;

    JUCE_DECLARE_NON_COPYABLE (FontManager)
};

} // namespace time_dilation
