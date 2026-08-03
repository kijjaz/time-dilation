#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

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
        auto smoochFile = fontDir.getChildFile ("SmoochSans.ttf");
        if (smoochFile.existsAsFile())
        {
            juce::MemoryBlock mb;
            smoochFile.loadFileAsData (mb);
            smoochTypeface = juce::Typeface::createSystemTypefaceFor (mb.getData(), mb.getSize());
        }

        auto notoFile = fontDir.getChildFile ("NotoSans.ttf");
        if (notoFile.existsAsFile())
        {
            juce::MemoryBlock mb;
            notoFile.loadFileAsData (mb);
            notoTypeface = juce::Typeface::createSystemTypefaceFor (mb.getData(), mb.getSize());
        }
    }

    juce::Typeface::Ptr getSmoochTypeface() const { return smoochTypeface; }
    juce::Typeface::Ptr getNotoTypeface() const { return notoTypeface; }

    juce::Font getSciFiFont (float height = 14.0f, int style = juce::Font::plain)
    {
        if (smoochTypeface != nullptr)
        {
            return juce::Font (juce::FontOptions (smoochTypeface).withHeight (height));
        }
        return juce::FontOptions (height, style);
    }

    juce::Font getFallbackFont (float height = 12.0f, int style = juce::Font::plain)
    {
        if (notoTypeface != nullptr)
        {
            return juce::Font (juce::FontOptions (notoTypeface).withHeight (height));
        }
        return juce::FontOptions (height, style);
    }

private:
    FontManager() = default;
    juce::Typeface::Ptr smoochTypeface;
    juce::Typeface::Ptr notoTypeface;

    JUCE_DECLARE_NON_COPYABLE (FontManager)
};

} // namespace time_dilation
