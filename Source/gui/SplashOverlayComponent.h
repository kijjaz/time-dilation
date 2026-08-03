#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <BinaryData.h>

namespace time_dilation
{

class SplashOverlayComponent : public juce::Component,
                               public juce::Timer
{
public:
    SplashOverlayComponent()
    {
        setInterceptsMouseClicks (true, true);

        // 1. Try loading high-resolution PNG image first (BinaryData or File)
        int pngSize = 0;
        const char* pngData = BinaryData::getNamedResource ("logo_banner_svg_png", pngSize);
        if (pngData != nullptr && pngSize > 0)
        {
            pngImage = juce::ImageFileFormat::loadFrom (pngData, pngSize);
        }

        if (!pngImage.isValid())
        {
            std::vector<juce::File> candidatePngFiles = {
                juce::File ("/Users/kijjaz/Desktop/Antigravity/2026/20260801 Time Dilation DAW/Source/assets/logo_banner.svg.png"),
                juce::File::getCurrentWorkingDirectory().getChildFile ("Source").getChildFile ("assets").getChildFile ("logo_banner.svg.png"),
                juce::File::getCurrentWorkingDirectory().getChildFile ("web").getChildFile ("logo_banner.png")
            };
            for (const auto& pngFile : candidatePngFiles)
            {
                if (pngFile.existsAsFile())
                {
                    pngImage = juce::ImageFileFormat::loadFrom (pngFile);
                    if (pngImage.isValid())
                        break;
                }
            }
        }

        // 2. SVG Fallback
        if (!pngImage.isValid())
        {
            int svgSize = 0;
            const char* svgData = BinaryData::getNamedResource ("logo_banner_svg", svgSize);
            if (svgData != nullptr && svgSize > 0)
            {
                auto xml = juce::XmlDocument::parse (juce::String::createStringFromData (svgData, svgSize));
                if (xml != nullptr)
                {
                    svgDrawable = juce::Drawable::createFromSVG (*xml);
                }
            }

            if (svgDrawable == nullptr)
            {
                std::vector<juce::File> candidateSvgFiles = {
                    juce::File ("/Users/kijjaz/Desktop/Antigravity/2026/20260801 Time Dilation DAW/web/logo_banner.svg"),
                    juce::File ("/Users/kijjaz/Desktop/Antigravity/2026/20260801 Time Dilation DAW/Source/assets/logo_banner.svg"),
                    juce::File::getCurrentWorkingDirectory().getChildFile ("web").getChildFile ("logo_banner.svg"),
                    juce::File::getCurrentWorkingDirectory().getChildFile ("Source").getChildFile ("assets").getChildFile ("logo_banner.svg")
                };

                for (const auto& svgFile : candidateSvgFiles)
                {
                    if (svgFile.existsAsFile())
                    {
                        svgDrawable = juce::Drawable::createFromSVGFile (svgFile);
                        if (svgDrawable != nullptr)
                            break;
                    }
                }
            }
        }

        // Start timer at 30 FPS (~33ms per frame)
        startTimer (33);
    }

    ~SplashOverlayComponent() override
    {
        stopTimer();
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        // Click anywhere to fade quickly instantly (~200ms)
        fastFade = true;
    }

    void timerCallback() override
    {
        if (fastFade)
        {
            // Rapid fade out (~200ms)
            alpha -= 0.15f;
        }
        else
        {
            // Smooth 3-second auto fade out
            alpha -= 0.011f;
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
        if (alpha <= 0.0f)
            return;

        g.setOpacity (alpha);

        // Deep Warm Obsidian Background
        g.fillAll (juce::Colour (0xff040207));

        // Radial Glow Background
        juce::ColourGradient bgGlow (juce::Colour (0xff1a061e), (float) getWidth() * 0.44f, (float) getHeight() * 0.5f,
                                    juce::Colour (0xff040207), (float) getWidth() * 0.85f, (float) getHeight() * 0.5f, true);
        g.setGradientFill (bgGlow);
        g.fillRect (getLocalBounds());

        // Render Logo Banner
        auto logoBounds = getLocalBounds().reduced (35).toFloat();

        if (pngImage.isValid())
        {
            g.drawImage (pngImage, logoBounds, juce::RectanglePlacement::centred);
        }
        else if (svgDrawable != nullptr)
        {
            svgDrawable->drawWithin (g, logoBounds, juce::RectanglePlacement::centred, alpha);
        }
        else
        {
            // Fallback Typography
            g.setColour (juce::Colour (0xffffffff).withAlpha (alpha));
            g.setFont (juce::FontOptions (32.0f, juce::Font::bold));
            g.drawText ("TIME DILATION DAW", getLocalBounds().withTrimmedBottom (30), juce::Justification::centred, true);

            g.setColour (juce::Colour (0xfff59e0b).withAlpha (alpha * 0.9f));
            g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
            g.drawText ("RELATIVISTIC AUDIO ENGINE", getLocalBounds().withTrimmedTop (40), juce::Justification::centred, true);
        }

        // Click Dismiss Hint
        g.setColour (juce::Colour (0xfff59e0b).withAlpha (alpha * 0.65f));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("CLICK ANYWHERE TO DISMISS", getLocalBounds().removeFromBottom (35), juce::Justification::centred, true);
    }

private:
    float alpha = 1.0f;
    bool fastFade = false;
    std::unique_ptr<juce::Drawable> svgDrawable;
    juce::Image pngImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SplashOverlayComponent)
};

} // namespace time_dilation
