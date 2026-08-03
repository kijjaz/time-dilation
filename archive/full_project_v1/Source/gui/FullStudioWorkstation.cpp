#include "FullStudioWorkstation.h"

namespace time_dilation
{

FullStudioWorkstation::FullStudioWorkstation (TimeDilationEngine& e)
    : engine (e), pianoRollComponent (e)
{
    addAndMakeVisible (pianoRollComponent);

    addAndMakeVisible (playButton);
    playButton.onClick = [this] { engine.play(); };

    addAndMakeVisible (pauseButton);
    pauseButton.onClick = [this] { engine.pause(); };

    addAndMakeVisible (stopButton);
    stopButton.onClick = [this] { engine.stop(); };

    addAndMakeVisible (auditionButton);
    auditionButton.onClick = [this] {
        engine.play();
        engine.triggerAuditionNote();
    };

    addAndMakeVisible (bpmSlider);
    bpmSlider.setRange (40.0, 240.0, 1.0);
    bpmSlider.setValue (engine.getBpm());
    bpmSlider.onValueChange = [this] { engine.setBpm ((float) bpmSlider.getValue()); };

    addAndMakeVisible (bpmLabel);
    bpmLabel.setText ("BPM", juce::dontSendNotification);

    addAndMakeVisible (masterGammaSlider);
    masterGammaSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    masterGammaSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 55, 16);
    masterGammaSlider.setRange (-4.0, 4.0, 0.05);
    masterGammaSlider.setValue (1.0);
    masterGammaSlider.onValueChange = [this] { engine.setMasterDilation ((float) masterGammaSlider.getValue()); };

    addAndMakeVisible (masterGammaLabel);
    masterGammaLabel.setText ("MASTER GAMMA", juce::dontSendNotification);

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

    startTimerHz (30);
}

FullStudioWorkstation::~FullStudioWorkstation()
{
    stopTimer();
}

void FullStudioWorkstation::timerCallback()
{
    repaint();
}

void FullStudioWorkstation::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0e17));

    // Top Header Banner
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRect (0, 0, getWidth(), 40);

    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    g.drawText ("TIME DILATION WORKSTATION", 15, 10, 240, 20, juce::Justification::left);

    float currentGamma = engine.getMasterDilation();
    if (currentGamma < 0.0f)
    {
        g.setColour (juce::Colour (0xffef4444));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("RETROGRADE TIME (GAMMA < 0)", 260, 10, 220, 20, juce::Justification::left);
    }

    // Draw Channel Strip Cards in Lower Panel
    const float stripY = getHeight() - 200;
    const float stripW = 160.0f;
    const auto& tracks = engine.getTracks();

    g.setColour (juce::Colour (0xff141a26));
    g.fillRect (0.0f, stripY, (float) getWidth(), 200.0f);

    float x = 10.0f;
    for (size_t i = 0; i < tracks.size(); ++i)
    {
        const auto& t = tracks[i];
        bool isSel = (static_cast<int>(i) == selectedTrackIdx);

        g.setColour (isSel ? juce::Colour (0xff1e293b) : juce::Colour (0xff0f141d));
        g.fillRoundedRectangle (x, stripY + 10.0f, stripW, 180.0f, 4.0f);

        g.setColour (t.color);
        g.fillRect (x, stripY + 10.0f, stripW, 20.0f);

        g.setColour (juce::Colour (0xffffffff));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (t.name, x + 6.0f, stripY + 12.0f, stripW - 12.0f, 16.0f, juce::Justification::left);

        // Fader bar
        g.setColour (juce::Colour (0xff1e293b));
        g.fillRect (x + 20.0f, stripY + 40.0f, 8.0f, 120.0f);
        g.setColour (t.color);
        g.fillRect (x + 20.0f, stripY + 40.0f + (1.0f - t.volume) * 120.0f, 8.0f, t.volume * 120.0f);

        // Meter
        g.setColour (juce::Colour (0xff10b981));
        g.fillRect (x + 35.0f, stripY + 40.0f + (1.0f - t.currentAmplitude) * 120.0f, 6.0f, t.currentAmplitude * 120.0f);

        g.setColour (juce::Colour (0xff94a3b8));
        g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
        g.drawText ("GAMMA: " + juce::String (t.timeDilation, 2) + "x", x + 50.0f, stripY + 45.0f, 100.0f, 16.0f, juce::Justification::left);
        g.drawText (t.warpMode == WarpMode::Varispeed ? "VARISPEED" : "PITCH-LOCK", x + 50.0f, stripY + 65.0f, 100.0f, 16.0f, juce::Justification::left);

        x += stripW + 10.0f;
    }
}

void FullStudioWorkstation::resized()
{
    const int topBarH = 40;
    const int sidebarW = 180;

    playButton.setBounds (300, 8, 45, 24);
    pauseButton.setBounds (350, 8, 45, 24);
    stopButton.setBounds (400, 8, 45, 24);
    auditionButton.setBounds (450, 8, 110, 24);

    bpmLabel.setBounds (570, 8, 30, 24);
    bpmSlider.setBounds (605, 8, 60, 24);

    addTrackButton.setBounds (getWidth() - sidebarW + 10, topBarH + 10, 160, 26);
    addSubTrackButton.setBounds (getWidth() - sidebarW + 10, topBarH + 42, 160, 26);
    warpModeButton.setBounds (getWidth() - sidebarW + 10, topBarH + 74, 160, 26);

    masterGammaLabel.setBounds (getWidth() - sidebarW + 10, topBarH + 115, 160, 18);
    masterGammaSlider.setBounds (getWidth() - sidebarW + 40, topBarH + 135, 90, 90);

    const int bottomPanelH = 200;
    pianoRollComponent.setBounds (0, topBarH, getWidth() - sidebarW, getHeight() - topBarH - bottomPanelH);
}

} // namespace time_dilation
