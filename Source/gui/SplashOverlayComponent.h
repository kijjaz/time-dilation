#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <BinaryData.h>
#include "FontManager.h"
#include "../VersionInfo.h"

namespace time_dilation
{

class SplashOverlayComponent : public juce::Component,
                               public juce::Timer
{
public:
    SplashOverlayComponent()
    {
        setInterceptsMouseClicks (true, true);

        // 1. Load high-res PNG image from BinaryData
        int pngSize = 0;
        const char* pngData = BinaryData::getNamedResource ("logo_banner_svg_png", pngSize);
        if (pngData != nullptr && pngSize > 0)
        {
            pngImage = juce::ImageFileFormat::loadFrom (pngData, pngSize);
        }

        // 2. File Fallback if BinaryData PNG is missing
        if (!pngImage.isValid())
        {
            auto exeFile = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
            std::vector<juce::File> candidatePngFiles = {
                juce::File ("/Users/kijjaz/Desktop/Antigravity/2026/20260801 Time Dilation DAW/Source/assets/logo_banner.svg.png"),
                juce::File ("/Users/kijjaz/Desktop/Antigravity/2026/20260801 Time Dilation DAW/web/logo_banner.png"),
                exeFile.getParentDirectory().getChildFile ("logo_banner.png")
            };

            for (const auto& f : candidatePngFiles)
            {
                if (f.existsAsFile())
                {
                    pngImage = juce::ImageFileFormat::loadFrom (f);
                    if (pngImage.isValid()) break;
                }
            }
        }

        startTimer (33); // ~30 FPS smooth fade animation
    }

    ~SplashOverlayComponent() override
    {
        stopTimer();
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        fastFade = true;
    }

    void timerCallback() override
    {
        pulsePhase += 0.05f;
        if (pulsePhase > juce::MathConstants<float>::twoPi) pulsePhase -= juce::MathConstants<float>::twoPi;

        if (fastFade)
        {
            alpha -= 0.12f;
        }
        else
        {
            alpha -= 0.008f; // ~4-second display with smooth fade out
        }

        if (alpha <= 0.0f)
        {
            alpha = 0.0f;
            stopTimer();
            setVisible (false);
        }

        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        if (alpha <= 0.0f) return;

        g.setOpacity (alpha);

        // Solid Obsidian Black Background (#070a12)
        g.fillAll (juce::Colour (0xff070a12));

        // Draw Pristine PNG Splash Image Centered
        if (pngImage.isValid())
        {
            auto bounds = getLocalBounds().toFloat();
            float targetW = std::min (bounds.getWidth() * 0.92f, 1000.0f);
            float aspect = pngImage.getHeight() / static_cast<float>(pngImage.getWidth());
            float targetH = targetW * aspect;

            if (targetH > bounds.getHeight() * 0.9f)
            {
                targetH = bounds.getHeight() * 0.9f;
                targetW = targetH / aspect;
            }

            float x = (bounds.getWidth() - targetW) * 0.5f;
            float y = (bounds.getHeight() - targetH) * 0.5f;

            g.drawImage (pngImage, juce::Rectangle<float> (x, y, targetW, targetH), juce::RectanglePlacement::centred);
        }
        else
        {
            g.setColour (juce::Colour (0xfff8fafc));
            g.setFont (FontManager::getInstance().getOxaniumFont (28.0f, true));
            g.drawText ("TIME DILATION DAW", getLocalBounds(), juce::Justification::centred);
        }
    }

private:
    float alpha = 1.0f;
    float pulsePhase = 0.0f;
    bool fastFade = false;
    std::unique_ptr<juce::Drawable> svgDrawable;
    juce::Image pngImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SplashOverlayComponent)
};

} // namespace time_dilation
