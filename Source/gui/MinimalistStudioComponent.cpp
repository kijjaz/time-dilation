#include "MinimalistStudioComponent.h"

namespace time_dilation
{

MinimalistStudioComponent::MinimalistStudioComponent (TimeDilationEngine& e)
    : engine (e)
{
    // Setup Device Rack Rotary Dials
    auto setupRotary = [this] (juce::Slider& s, juce::Label& l, const juce::String& text, double min, double max, double init) {
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 16);
        s.setRange (min, max, 0.01);
        s.setValue (init);

        addAndMakeVisible (l);
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
    };

    setupRotary (attackSlider, attackLabel, "ATTACK", 0.01, 2.0, 0.05);
    setupRotary (releaseSlider, releaseLabel, "RELEASE", 0.05, 5.0, 0.4);
    setupRotary (cutoffSlider, cutoffLabel, "CUTOFF", 100.0, 12000.0, 2500.0);
    setupRotary (gammaSlider, gammaLabel, "TRACK GAMMA", -4.0, 4.0, 1.0);

    gammaSlider.onValueChange = [this] {
        if (selectedTrackIdx >= 0 && selectedTrackIdx < static_cast<int>(engine.getTracks().size()))
        {
            engine.updateTrackGamma (selectedTrackIdx, (float) gammaSlider.getValue());
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
            repaint();
        }
    };

    addAndMakeVisible (auditionButton);
    auditionButton.onClick = [this] {
        engine.play();
        engine.triggerAuditionNote();
    };

    addAndMakeVisible (addTrackButton);
    addTrackButton.onClick = [this] {
        int count = static_cast<int>(engine.getTracks().size()) + 1;
        juce::Colour colors[] = { juce::Colour (0xfff59e0b), juce::Colour (0xff8b5cf6), juce::Colour (0xff06b6d4), juce::Colour (0xffec4899) };
        engine.addTrack ("Track " + juce::String (count), colors[count % 4]);
        repaint();
    };

    addAndMakeVisible (addSubTrackButton);
    addSubTrackButton.onClick = [this] {
        if (!engine.getTracks().empty())
        {
            int count = static_cast<int>(engine.getTracks().size()) + 1;
            engine.addTrack ("Sub-Track " + juce::String (count), juce::Colour (0xffa78bfa), selectedTrackIdx);
            repaint();
        }
    };

    startTimerHz (30);
}

MinimalistStudioComponent::~MinimalistStudioComponent()
{
    stopTimer();
}

void MinimalistStudioComponent::timerCallback()
{
    repaint();
}

void MinimalistStudioComponent::mouseDown (const juce::MouseEvent& e)
{
    // Click track list row to select track
    const float trackRowH = 50.0f;
    float startY = 40.0f;
    int clickedIdx = static_cast<int>((e.position.y - startY) / trackRowH);

    if (clickedIdx >= 0 && clickedIdx < static_cast<int>(engine.getTracks().size()))
    {
        selectedTrackIdx = clickedIdx;
        gammaSlider.setValue (engine.getTracks()[selectedTrackIdx].timeDilation);
        auto m = engine.getTracks()[selectedTrackIdx].warpMode;
        warpModeButton.setButtonText (m == WarpMode::Varispeed ? "MODE: VARISPEED TAPE" : "MODE: TIME STRETCH");
        repaint();
    }
}

void MinimalistStudioComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0f141d));

    // Top Section: Track List Header
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRect (0, 0, getWidth(), 35);

    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.drawText ("TRACK LIST & TIME DILATION", 15, 8, 220, 20, juce::Justification::left);

    // Render Track Rows
    const auto& tracks = engine.getTracks();
    const float trackRowH = 50.0f;
    float y = 40.0f;

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        const auto& t = tracks[i];
        bool isSel = (static_cast<int>(i) == selectedTrackIdx);
        float indent = (t.parentTrackIndex >= 0) ? 20.0f : 0.0f;

        // Row background
        g.setColour (isSel ? juce::Colour (0xff1e293b) : (i % 2 == 0 ? juce::Colour (0xff141a26) : juce::Colour (0xff10141d)));
        g.fillRect (10.0f, y, (float) getWidth() - 200.0f, trackRowH - 4.0f);

        if (isSel)
        {
            g.setColour (juce::Colour (0xfff59e0b));
            g.drawRect (10.0f, y, (float) getWidth() - 200.0f, trackRowH - 4.0f, 1.5f);
        }

        // Color tag
        g.setColour (t.color);
        g.fillRect (10.0f + indent, y, 6.0f, trackRowH - 4.0f);

        // Track Name & Status
        g.setColour (juce::Colour (0xffffffff));
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.drawText (t.name, 24.0f + indent, y + 6.0f, 180.0f, 18.0f, juce::Justification::left);

        g.setColour (juce::Colour (0xff94a3b8));
        g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
        juce::String modeText = (t.warpMode == WarpMode::Varispeed) ? "VARISPEED TAPE" : "TIME STRETCH";
        g.drawText (modeText + " | VOL: " + juce::String (t.volume, 2) + " | GAMMA: " + juce::String (t.timeDilation, 2) + "x", 24.0f + indent, y + 26.0f, 300.0f, 16.0f, juce::Justification::left);

        // Level meter bar
        g.setColour (juce::Colour (0xff10b981));
        g.fillRect (getWidth() - 320.0f, y + 15.0f, t.currentAmplitude * 100.0f, 12.0f);

        y += trackRowH;
    }

    // Bottom Section: Device Rack Panel Background
    const float deviceRackY = getHeight() - 170.0f;
    g.setColour (juce::Colour (0xff141a26));
    g.fillRect (0.0f, deviceRackY, (float) getWidth(), 170.0f);

    g.setColour (juce::Colour (0xff334155));
    g.drawLine (0.0f, deviceRackY, (float) getWidth(), deviceRackY, 1.0f);

    if (selectedTrackIdx >= 0 && selectedTrackIdx < static_cast<int>(tracks.size()))
    {
        g.setColour (juce::Colour (0xfff59e0b));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText ("DEVICE RACK: " + tracks[selectedTrackIdx].name.toUpperCase() + " (SYNTH & TIME ENGINE)", 15, deviceRackY + 8, 400, 18, juce::Justification::left);
    }
}

void MinimalistStudioComponent::resized()
{
    const float deviceRackY = getHeight() - 170.0f;
    const float knobY = deviceRackY + 35.0f;
    const float knobSize = 75.0f;

    attackSlider.setBounds (20, knobY, knobSize, knobSize);
    attackLabel.setBounds (20, knobY + knobSize + 2.0f, knobSize, 16);

    releaseSlider.setBounds (110, knobY, knobSize, knobSize);
    releaseLabel.setBounds (110, knobY + knobSize + 2.0f, knobSize, 16);

    cutoffSlider.setBounds (200, knobY, knobSize, knobSize);
    cutoffLabel.setBounds (200, knobY + knobSize + 2.0f, knobSize, 16);

    gammaSlider.setBounds (290, knobY, knobSize, knobSize);
    gammaLabel.setBounds (290, knobY + knobSize + 2.0f, knobSize, 16);

    warpModeButton.setBounds (400, knobY + 10.0f, 160, 28);
    auditionButton.setBounds (400, knobY + 45.0f, 160, 28);

    // Right Sidebar Controls
    addTrackButton.setBounds (getWidth() - 180, 45, 160, 28);
    addSubTrackButton.setBounds (getWidth() - 180, 80, 160, 28);
}

} // namespace time_dilation
