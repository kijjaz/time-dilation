#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <BinaryData.h>
#include "FontManager.h"

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

        // Deep Sci-Fi Carbon Background
        g.fillAll (juce::Colour (0xff070a12));

        float cx = getWidth() * 0.5f;
        float cy = getHeight() * 0.5f;

        // Dynamic Pulsing Radial Glow
        float glowRadius = std::min (cx, cy) * (0.85f + 0.05f * std::sin (pulsePhase));
        juce::ColourGradient bgGlow (juce::Colour (0xff1e1b4b).withAlpha (0.6f), cx, cy,
                                    juce::Colour (0xff070a12), glowRadius, cy, true);
        g.setGradientFill (bgGlow);
        g.fillRect (getLocalBounds());

        // Glassmorphic Center Card
        float cardW = std::min (680.0f, getWidth() * 0.85f);
        float cardH = std::min (380.0f, getHeight() * 0.75f);
        float cardX = cx - cardW * 0.5f;
        float cardY = cy - cardH * 0.5f;

        g.setColour (juce::Colour (0xee0d1322));
        g.fillRoundedRectangle (cardX, cardY, cardW, cardH, 12.0f);

        // Relativistic Gold & Cyan Outline
        g.setColour (juce::Colour (0xff06b6d4).withAlpha (0.7f));
        g.drawRoundedRectangle (cardX, cardY, cardW, cardH, 12.0f, 1.5f);
        g.setColour (juce::Colour (0xfff59e0b).withAlpha (0.4f + 0.3f * std::sin (pulsePhase)));
        g.drawRoundedRectangle (cardX - 2.0f, cardY - 2.0f, cardW + 4.0f, cardH + 4.0f, 14.0f, 2.0f);

        // Render Clean PNG Logo Image Centered
        juce::Rectangle<float> bannerArea (cardX + 20.0f, cardY + 20.0f, cardW - 40.0f, cardH - 100.0f);

        if (pngImage.isValid())
        {
            g.drawImage (pngImage, bannerArea, juce::RectanglePlacement::centred);
        }
        else
        {
            g.setColour (juce::Colour (0xfff8fafc));
            g.setFont (FontManager::getInstance().getOxaniumFont (28.0f, true));
            g.drawText ("TIME DILATION DAW", cardX, cardY + 60.0f, cardW, 40.0f, juce::Justification::centred);
        }

        // Subtitle & Version Tagline
        g.setColour (juce::Colour (0xfff59e0b).withAlpha (alpha * 0.95f));
        g.setFont (FontManager::getInstance().getOxaniumFont (11.5f, true));
        g.drawText ("RELATIVISTIC TIME DILATION WORKSTATION  |  VERSION 0.0.1 (JUCE 7 C++20)", cardX + 15.0f, cardY + cardH - 68.0f, cardW - 30.0f, 20.0f, juce::Justification::centred);

        // Dismiss Pill Button Hint
        float pillW = 220.0f;
        float pillH = 28.0f;
        float pillX = cx - pillW * 0.5f;
        float pillY = cardY + cardH - 42.0f;

        g.setColour (juce::Colour (0xff1e293b).withAlpha (0.9f));
        g.fillRoundedRectangle (pillX, pillY, pillW, pillH, 14.0f);
        g.setColour (juce::Colour (0xff06b6d4).withAlpha (0.8f));
        g.drawRoundedRectangle (pillX, pillY, pillW, pillH, 14.0f, 1.0f);

        g.setColour (juce::Colour (0xfff8fafc).withAlpha (alpha * 0.9f));
        g.setFont (FontManager::getInstance().getOxaniumFont (10.5f, true));
        g.drawText ("CLICK ANYWHERE TO DISMISS", pillX, pillY, pillW, pillH, juce::Justification::centred);
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
