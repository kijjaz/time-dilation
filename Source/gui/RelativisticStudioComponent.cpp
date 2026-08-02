#include "RelativisticStudioComponent.h"

namespace time_dilation
{

RelativisticStudioComponent::RelativisticStudioComponent (TimeDilationEngine& e)
    : engine (e), dataflowCanvas (e)
{
    addAndMakeVisible (dataflowCanvas);

    addAndMakeVisible (masterGammaSlider);
    masterGammaSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    masterGammaSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    masterGammaSlider.setRange (-4.0, 4.0, 0.05);
    masterGammaSlider.setValue (1.0);
    masterGammaSlider.onValueChange = [this] {
        engine.setMasterDilation ((float) masterGammaSlider.getValue());
    };

    addAndMakeVisible (masterGammaLabel);
    masterGammaLabel.setText ("MASTER GAMMA", juce::dontSendNotification);

    addAndMakeVisible (auditionButton);
    auditionButton.onClick = [this] {
        engine.play();
        engine.triggerAuditionNote();
    };

    addAndMakeVisible (addSynthButton);
    addSynthButton.onClick = [this] {
        dataflowCanvas.addSynthNode();
    };

    addAndMakeVisible (addMixerBusButton);
    addMixerBusButton.onClick = [this] {
        dataflowCanvas.addMixerBusNode();
    };

    addAndMakeVisible (addTrackButton);
    addTrackButton.onClick = [this] {
        int count = static_cast<int>(engine.getTracks().size()) + 1;
        juce::Colour colors[] = { juce::Colour (0xfff59e0b), juce::Colour (0xff8b5cf6), juce::Colour (0xff06b6d4), juce::Colour (0xffec4899) };
        engine.addTrack ("Track " + juce::String (count), colors[count % 4]);
        dataflowCanvas.repaint();
    };

    addAndMakeVisible (addSubTrackButton);
    addSubTrackButton.onClick = [this] {
        if (!engine.getTracks().empty())
        {
            int count = static_cast<int>(engine.getTracks().size()) + 1;
            engine.addTrack ("Sub-Track " + juce::String (count), juce::Colour (0xffa78bfa), selectedTrackIdx);
            dataflowCanvas.repaint();
        }
    };

    addAndMakeVisible (warpModeButton);
    warpModeButton.onClick = [this] {
        if (selectedTrackIdx >= 0 && selectedTrackIdx < static_cast<int>(engine.getTracks().size()))
        {
            auto currentMode = engine.getTracks()[selectedTrackIdx].warpMode;
            auto newMode = (currentMode == WarpMode::Varispeed) ? WarpMode::Granular : WarpMode::Varispeed;
            engine.updateTrackWarpMode (selectedTrackIdx, newMode);
            warpModeButton.setButtonText (newMode == WarpMode::Varispeed ? "MODE: VARISPEED TAPE" : "MODE: TIME STRETCH");
            dataflowCanvas.repaint();
        }
    };

    startTimerHz (30);
}

RelativisticStudioComponent::~RelativisticStudioComponent()
{
    stopTimer();
}

void RelativisticStudioComponent::timerCallback()
{
    repaint();
}

void RelativisticStudioComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0f141d));

    // Render Master Time Indicator Banner
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRect (0, 0, getWidth(), 40);

    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    g.drawText ("MODULAR DATAFLOW CANVAS", 15, 10, 240, 20, juce::Justification::left);

    // Negative Time Warning / Indicator
    float currentGamma = engine.getMasterDilation();
    if (currentGamma < 0.0f)
    {
        g.setColour (juce::Colour (0xffef4444)); // Red glowing negative time badge
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText ("RETROGRADE TIME SCRUBBING (GAMMA < 0)", 270, 10, 280, 20, juce::Justification::left);
    }
    else
    {
        g.setColour (juce::Colour (0xff38bdf8));
        g.setFont (juce::FontOptions (12.0f, juce::Font::plain));
        g.drawText ("FORWARD TIME FLOW", 270, 10, 160, 20, juce::Justification::left);
    }
}

void RelativisticStudioComponent::resized()
{
    const int topBarH = 40;
    const int sidebarW = 180;

    auditionButton.setBounds (getWidth() - sidebarW + 10, topBarH + 10, 160, 28);
    addSynthButton.setBounds (getWidth() - sidebarW + 10, topBarH + 45, 160, 28);
    addMixerBusButton.setBounds (getWidth() - sidebarW + 10, topBarH + 80, 160, 28);
    addTrackButton.setBounds (getWidth() - sidebarW + 10, topBarH + 115, 160, 28);
    addSubTrackButton.setBounds (getWidth() - sidebarW + 10, topBarH + 150, 160, 28);
    warpModeButton.setBounds (getWidth() - sidebarW + 10, topBarH + 185, 160, 28);

    masterGammaLabel.setBounds (getWidth() - sidebarW + 10, topBarH + 230, 160, 20);
    masterGammaSlider.setBounds (getWidth() - sidebarW + 40, topBarH + 255, 100, 100);

    dataflowCanvas.setBounds (0, topBarH, getWidth() - sidebarW, getHeight() - topBarH);
}

} // namespace time_dilation
