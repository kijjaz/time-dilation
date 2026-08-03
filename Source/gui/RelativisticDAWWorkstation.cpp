#include "RelativisticDAWWorkstation.h"
#include "AudioSettingsComponent.h"

namespace time_dilation
{

RelativisticDAWWorkstation::RelativisticDAWWorkstation (TimeDilationEngine& e)
    : engine (e), timelineClips (e)
{
    addAndMakeVisible (timelineClips);

    // Setup Top Transport Controls
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

    addAndMakeVisible (audioSettingsButton);
    audioSettingsButton.onClick = [this] {
        auto* dialog = new juce::DialogWindow::LaunchOptions();
        dialog->dialogTitle = "AUDIO INTERFACE & SAMPLE RATE SETTINGS";
        dialog->content.setOwned (new AudioSettingsComponent (engine.getTracktionEngine().getDeviceManager().deviceManager));
        dialog->dialogBackgroundColour = juce::Colour (0xff0f141d);
        dialog->escapeKeyTriggersCloseButton = true;
        dialog->useNativeTitleBar = true;
        dialog->resizable = true;
        dialog->launchAsync();
    };

    // Sidebar Track Buttons
    addAndMakeVisible (addTrackButton);
    addTrackButton.onClick = [this] {
        int count = static_cast<int>(engine.getTracks().size()) + 1;
        juce::Colour colors[] = { juce::Colour (0xfff59e0b), juce::Colour (0xff8b5cf6), juce::Colour (0xff06b6d4), juce::Colour (0xffec4899) };
        engine.addTrack ("Track " + juce::String (count), colors[count % 4]);
        rebuildTrackUI();
        repaint();
    };

    addAndMakeVisible (addSubTrackButton);
    addSubTrackButton.onClick = [this] {
        if (!engine.getTracks().empty())
        {
            int count = static_cast<int>(engine.getTracks().size()) + 1;
            engine.addTrack ("Sub-Track " + juce::String (count), juce::Colour (0xffa78bfa), selectedTrackIdx);
            rebuildTrackUI();
            repaint();
        }
    };

    // Setup Bottom Device Rack Dials
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
    setupRotary (cutoffSlider, cutoffLabel, "CUTOFF", 100.0, 12000.0, 2800.0);
    setupRotary (trackGammaSlider, trackGammaLabel, "TRACK GAMMA", -4.0, 4.0, 1.0);
    setupRotary (lfoSpeedSlider, lfoSpeedLabel, "TIME LFO MOD", 0.1, 20.0, 1.0);

    trackGammaSlider.onValueChange = [this] {
        if (selectedTrackIdx >= 0 && selectedTrackIdx < static_cast<int>(engine.getTracks().size()))
        {
            engine.updateTrackGamma (selectedTrackIdx, (float) trackGammaSlider.getValue());
            if (selectedTrackIdx < static_cast<int>(trackUIs.size()) && trackUIs[selectedTrackIdx].gammaSlider)
            {
                trackUIs[selectedTrackIdx].gammaSlider->setValue (trackGammaSlider.getValue(), juce::dontSendNotification);
            }
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
            rebuildTrackUI();
            repaint();
        }
    };

    rebuildTrackUI();
    startTimerHz (30);
}

RelativisticDAWWorkstation::~RelativisticDAWWorkstation()
{
    stopTimer();
}

void RelativisticDAWWorkstation::rebuildTrackUI()
{
    trackUIs.clear();

    const auto& tracks = engine.getTracks();
    for (size_t i = 0; i < tracks.size(); ++i)
    {
        TrackUIComponents ui;
        int trackIdx = static_cast<int>(i);

        // Volume Slider
        ui.volSlider = std::make_unique<juce::Slider>();
        addAndMakeVisible (*ui.volSlider);
        ui.volSlider->setSliderStyle (juce::Slider::LinearHorizontal);
        ui.volSlider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        ui.volSlider->setRange (0.0, 1.0, 0.01);
        ui.volSlider->setValue (tracks[i].volume);
        ui.volSlider->onValueChange = [this, trackIdx] {
            if (trackIdx < static_cast<int>(engine.getTracks().size()))
                engine.updateTrackVolume (trackIdx, (float) trackUIs[trackIdx].volSlider->getValue());
        };

        // Gamma Slider
        ui.gammaSlider = std::make_unique<juce::Slider>();
        addAndMakeVisible (*ui.gammaSlider);
        ui.gammaSlider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        ui.gammaSlider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        ui.gammaSlider->setRange (-4.0, 4.0, 0.05);
        ui.gammaSlider->setValue (tracks[i].timeDilation);
        ui.gammaSlider->onValueChange = [this, trackIdx] {
            if (trackIdx < static_cast<int>(engine.getTracks().size()))
            {
                engine.updateTrackGamma (trackIdx, (float) trackUIs[trackIdx].gammaSlider->getValue());
                if (trackIdx == selectedTrackIdx)
                    trackGammaSlider.setValue (trackUIs[trackIdx].gammaSlider->getValue(), juce::dontSendNotification);
            }
        };

        // Mute Button
        ui.muteBtn = std::make_unique<juce::TextButton> ("M");
        addAndMakeVisible (*ui.muteBtn);
        ui.muteBtn->setClickingTogglesState (true);
        ui.muteBtn->setToggleState (tracks[i].mute, juce::dontSendNotification);
        ui.muteBtn->onClick = [this, trackIdx] {
            engine.toggleMute (trackIdx);
        };

        // Solo Button
        ui.soloBtn = std::make_unique<juce::TextButton> ("S");
        addAndMakeVisible (*ui.soloBtn);
        ui.soloBtn->setClickingTogglesState (true);
        ui.soloBtn->setToggleState (tracks[i].solo, juce::dontSendNotification);
        ui.soloBtn->onClick = [this, trackIdx] {
            engine.toggleSolo (trackIdx);
        };

        // Warp Mode Button
        ui.warpBtn = std::make_unique<juce::TextButton> (tracks[i].warpMode == WarpMode::Varispeed ? "VARISPEED" : "STRETCH");
        addAndMakeVisible (*ui.warpBtn);
        ui.warpBtn->onClick = [this, trackIdx] {
            auto currentMode = engine.getTracks()[trackIdx].warpMode;
            auto newMode = (currentMode == WarpMode::Varispeed) ? WarpMode::Granular : WarpMode::Varispeed;
            engine.updateTrackWarpMode (trackIdx, newMode);
            trackUIs[trackIdx].warpBtn->setButtonText (newMode == WarpMode::Varispeed ? "VARISPEED" : "STRETCH");
            if (trackIdx == selectedTrackIdx)
                warpModeButton.setButtonText (newMode == WarpMode::Varispeed ? "MODE: VARISPEED TAPE" : "MODE: TIME STRETCH");
        };

        trackUIs.push_back (std::move (ui));
    }

    resized();
}

void RelativisticDAWWorkstation::timerCallback()
{
    repaint();
}

void RelativisticDAWWorkstation::mouseDown (const juce::MouseEvent& e)
{
    const float headerH = 45.0f;
    const float trackRowH = 55.0f;
    float startY = headerH + 25.0f;

    if (e.position.y >= startY && e.position.y < getHeight() - 170.0f)
    {
        int clickedIdx = static_cast<int>((e.position.y - startY) / trackRowH);
        if (clickedIdx >= 0 && clickedIdx < static_cast<int>(engine.getTracks().size()))
        {
            selectedTrackIdx = clickedIdx;
            trackGammaSlider.setValue (engine.getTracks()[selectedTrackIdx].timeDilation, juce::dontSendNotification);
            auto m = engine.getTracks()[selectedTrackIdx].warpMode;
            warpModeButton.setButtonText (m == WarpMode::Varispeed ? "MODE: VARISPEED TAPE" : "MODE: TIME STRETCH");
            repaint();
        }
    }
}

void RelativisticDAWWorkstation::mouseDrag (const juce::MouseEvent& /*e*/)
{
}

void RelativisticDAWWorkstation::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0e17));

    // Top Header Banner
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRect (0, 0, getWidth(), 45);

    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("TIME DILATION DAW", 20, 12, 220, 20, juce::Justification::left);

    float currentGamma = engine.getMasterDilation();
    if (currentGamma < 0.0f)
    {
        g.setColour (juce::Colour (0xffef4444));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("RETROGRADE TIME SCRUBBING (GAMMA < 0)", 250, 12, 260, 20, juce::Justification::left);
    }

    // Time Ruler
    const float headerH = 45.0f;
    const float trackListW = 380.0f;
    const float timelineW = getWidth() - trackListW - 10.0f;

    g.setColour (juce::Colour (0xff141a26));
    g.fillRect (trackListW, headerH, timelineW, 25.0f);

    g.setColour (juce::Colour (0xff64748b));
    g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
    for (int b = 0; b < 32; b += 4)
    {
        float x = trackListW + b * 25.0f;
        g.drawText ("BAR " + juce::String (b / 4 + 1), x + 2.0f, headerH + 4.0f, 40.0f, 16.0f, juce::Justification::left);
        g.drawLine (x, headerH, x, headerH + 25.0f, 1.0f);
    }

    // Playhead Line
    double coordTime = engine.getCoordinateTime();
    float playheadX = trackListW + static_cast<float>(fmod (coordTime, 32.0)) * 25.0f;
    g.setColour (juce::Colour (0xfff59e0b));
    g.drawLine (playheadX, headerH, playheadX, getHeight() - 170.0f, 2.0f);

    // Track Rows Background & Waveform Representation
    const auto& tracks = engine.getTracks();
    const float trackRowH = 55.0f;
    float y = headerH + 25.0f;

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        const auto& t = tracks[i];
        bool isSel = (static_cast<int>(i) == selectedTrackIdx);
        float indent = (t.parentTrackIndex >= 0) ? 18.0f : 0.0f;

        // Track Header Background
        g.setColour (isSel ? juce::Colour (0xff1e293b) : (i % 2 == 0 ? juce::Colour (0xff121824) : juce::Colour (0xff0f141d)));
        g.fillRect (10.0f, y, trackListW - 15.0f, trackRowH - 4.0f);

        if (isSel)
        {
            g.setColour (juce::Colour (0xfff59e0b));
            g.drawRect (10.0f, y, trackListW - 15.0f, trackRowH - 4.0f, 1.5f);
        }

        // Color Strip
        g.setColour (t.color);
        g.fillRect (10.0f + indent, y, 5.0f, trackRowH - 4.0f);

        // Track Name
        g.setColour (juce::Colour (0xffffffff));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (t.name, 22.0f + indent, y + 6.0f, 120.0f, 16.0f, juce::Justification::left);

        // Timeline Waveform Track Lane
        g.setColour (juce::Colour (0xff141a26));
        g.fillRect (trackListW, y, timelineW, trackRowH - 4.0f);

        // Simulated Audio Waveform / Step Blocks
        g.setColour (t.color.withAlpha (0.4f));
        for (int step = 0; step < 16 && step < static_cast<int>(t.steps.size()); ++step)
        {
            if (t.steps[step])
            {
                float stepX = trackListW + step * 50.0f;
                g.fillRoundedRectangle (stepX + 2.0f, y + 6.0f, 46.0f, trackRowH - 16.0f, 3.0f);
            }
        }

        y += trackRowH;
    }

    // Bottom Device Rack Panel Background
    const float deviceRackY = getHeight() - 170.0f;
    g.setColour (juce::Colour (0xff121824));
    g.fillRect (0.0f, deviceRackY, (float) getWidth(), 170.0f);

    g.setColour (juce::Colour (0xff334155));
    g.drawLine (0.0f, deviceRackY, (float) getWidth(), deviceRackY, 1.0f);

    if (selectedTrackIdx >= 0 && selectedTrackIdx < static_cast<int>(tracks.size()))
    {
        g.setColour (juce::Colour (0xfff59e0b));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText ("DEVICE RACK: " + tracks[selectedTrackIdx].name.toUpperCase() + " (TIME MODULATORS & SYNTH RACK)", 20, deviceRackY + 8, 480, 18, juce::Justification::left);
    }
}

void RelativisticDAWWorkstation::resized()
{
    const int headerH = 45;
    const float trackListW = 380.0f;

    playButton.setBounds (getWidth() - 540, 10, 50, 25);
    pauseButton.setBounds (getWidth() - 485, 10, 50, 25);
    stopButton.setBounds (getWidth() - 430, 10, 50, 25);
    auditionButton.setBounds (getWidth() - 375, 10, 130, 25);

    bpmLabel.setBounds (getWidth() - 235, 10, 32, 25);
    bpmSlider.setBounds (getWidth() - 200, 10, 60, 25);

    masterGammaLabel.setBounds (getWidth() - 130, 4, 90, 16);
    masterGammaSlider.setBounds (getWidth() - 130, 18, 110, 24);

    audioSettingsButton.setBounds (10, getHeight() - 165, 110, 22);

    // Position per-track UIs
    const float trackRowH = 55.0f;
    float y = headerH + 25.0f;

    for (size_t i = 0; i < trackUIs.size(); ++i)
    {
        auto& ui = trackUIs[i];
        float indent = (engine.getTracks()[i].parentTrackIndex >= 0) ? 18.0f : 0.0f;

        if (ui.muteBtn) ui.muteBtn->setBounds (150.0f + indent, y + 6.0f, 22, 20);
        if (ui.soloBtn) ui.soloBtn->setBounds (175.0f + indent, y + 6.0f, 22, 20);
        if (ui.warpBtn) ui.warpBtn->setBounds (202.0f + indent, y + 6.0f, 68, 20);
        if (ui.gammaSlider) ui.gammaSlider->setBounds (275.0f + indent, y + 4.0f, 40, 40);
        if (ui.volSlider) ui.volSlider->setBounds (22.0f + indent, y + 28.0f, 140, 18);

        y += trackRowH;
    }

    // Position Bottom Device Rack Controls
    const float deviceRackY = getHeight() - 170.0f;
    const float knobY = deviceRackY + 35.0f;
    const float knobSize = 75.0f;

    attackSlider.setBounds (130, knobY, knobSize, knobSize);
    attackLabel.setBounds (130, knobY + knobSize + 2.0f, knobSize, 16);

    releaseSlider.setBounds (220, knobY, knobSize, knobSize);
    releaseLabel.setBounds (220, knobY + knobSize + 2.0f, knobSize, 16);

    cutoffSlider.setBounds (310, knobY, knobSize, knobSize);
    cutoffLabel.setBounds (310, knobY + knobSize + 2.0f, knobSize, 16);

    trackGammaSlider.setBounds (400, knobY, knobSize, knobSize);
    trackGammaLabel.setBounds (400, knobY + knobSize + 2.0f, knobSize, 16);

    lfoSpeedSlider.setBounds (490, knobY, knobSize, knobSize);
    addTrackButton.setBounds (getWidth() - 170, deviceRackY + 40.0f, 150, 28);
    addSubTrackButton.setBounds (getWidth() - 170, deviceRackY + 75.0f, 150, 28);

    timelineClips.setBounds (trackListW, headerH + 25.0f, getWidth() - trackListW - 10.0f, getHeight() - headerH - 25.0f - 170.0f);
}

} // namespace time_dilation
