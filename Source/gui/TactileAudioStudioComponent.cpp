#include "TactileAudioStudioComponent.h"

namespace time_dilation
{

TactileAudioStudioComponent::TactileAudioStudioComponent (TimeDilationEngine& e)
    : engine (e)
{
    addAndMakeVisible (playButton);
    playButton.onClick = [this] { engine.play(); };

    addAndMakeVisible (pauseButton);
    pauseButton.onClick = [this] { engine.pause(); };

    addAndMakeVisible (stopButton);
    stopButton.onClick = [this] { engine.stop(); };

    addAndMakeVisible (bounceButton);
    bounceButton.onClick = [this] {
        auto desktop = juce::File::getSpecialLocation (juce::File::userDesktopDirectory);
        auto outputFile = desktop.getChildFile ("TimeDilation_Export_" + juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S") + ".wav");
        if (engine.renderToDisk (outputFile))
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon, "BOUNCE SUCCESS", "Exported mix to: " + outputFile.getFullPathName());
        }
    };

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
        rebuildTrackUI();
        repaint();
    };

    rebuildTrackUI();
    startTimerHz (30);
}

TactileAudioStudioComponent::~TactileAudioStudioComponent()
{
    stopTimer();
}

bool TactileAudioStudioComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& file : files)
    {
        if (file.endsWithIgnoreCase (".wav") || file.endsWithIgnoreCase (".mp3") || file.endsWithIgnoreCase (".aif") || file.endsWithIgnoreCase (".flac"))
            return true;
    }
    return false;
}

void TactileAudioStudioComponent::filesDropped (const juce::StringArray& files, int /*x*/, int y)
{
    const float headerH = 45.0f;
    const float trackRowH = 65.0f;
    int targetTrackIdx = static_cast<int>((y - headerH - 30.0f) / trackRowH);

    if (targetTrackIdx < 0) targetTrackIdx = 0;
    if (targetTrackIdx >= static_cast<int>(engine.getTracks().size()))
    {
        engine.addTrack ("Track " + juce::String (engine.getTracks().size() + 1), juce::Colour (0xff06b6d4));
        targetTrackIdx = static_cast<int>(engine.getTracks().size()) - 1;
        rebuildTrackUI();
    }

    for (const auto& file : files)
    {
        juce::File audioFile (file);
        if (audioFile.existsAsFile())
        {
            engine.importAudioFile (targetTrackIdx, audioFile);
            engine.play();
            break;
        }
    }
    repaint();
}

void TactileAudioStudioComponent::rebuildTrackUI()
{
    trackRows.clear();

    const auto& tracks = engine.getTracks();
    for (size_t i = 0; i < tracks.size(); ++i)
    {
        TrackControlRow row;
        int trackIdx = static_cast<int>(i);

        // Gamma Rotary Knob
        row.gammaSlider = std::make_unique<juce::Slider>();
        addAndMakeVisible (*row.gammaSlider);
        row.gammaSlider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        row.gammaSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 45, 14);
        row.gammaSlider->setRange (-4.0, 4.0, 0.05);
        row.gammaSlider->setValue (tracks[i].timeDilation);
        row.gammaSlider->onValueChange = [this, trackIdx] {
            if (trackIdx < static_cast<int>(engine.getTracks().size()))
                engine.updateTrackGamma (trackIdx, (float) trackRows[trackIdx].gammaSlider->getValue());
        };

        // Volume Linear Slider
        row.volSlider = std::make_unique<juce::Slider>();
        addAndMakeVisible (*row.volSlider);
        row.volSlider->setSliderStyle (juce::Slider::LinearHorizontal);
        row.volSlider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        row.volSlider->setRange (0.0, 1.0, 0.01);
        row.volSlider->setValue (tracks[i].volume);
        row.volSlider->onValueChange = [this, trackIdx] {
            if (trackIdx < static_cast<int>(engine.getTracks().size()))
                engine.updateTrackVolume (trackIdx, (float) trackRows[trackIdx].volSlider->getValue());
        };

        // Loop Toggle Button
        row.loopBtn = std::make_unique<juce::TextButton> ("LOOP");
        addAndMakeVisible (*row.loopBtn);
        row.loopBtn->setClickingTogglesState (true);
        row.loopBtn->setToggleState (tracks[i].isLooping, juce::dontSendNotification);
        row.loopBtn->onClick = [this, trackIdx] {
            if (trackIdx < static_cast<int>(engine.getTracks().size()))
            {
                bool newState = !engine.getTracks()[trackIdx].isLooping;
                engine.getTracksMutable()[trackIdx].isLooping = newState;
                trackRows[trackIdx].loopBtn->setToggleState (newState, juce::dontSendNotification);
            }
        };

        // Mute Button
        row.muteBtn = std::make_unique<juce::TextButton> ("M");
        addAndMakeVisible (*row.muteBtn);
        row.muteBtn->setClickingTogglesState (true);
        row.muteBtn->setToggleState (tracks[i].mute, juce::dontSendNotification);
        row.muteBtn->onClick = [this, trackIdx] { engine.toggleMute (trackIdx); };

        // Solo Button
        row.soloBtn = std::make_unique<juce::TextButton> ("S");
        addAndMakeVisible (*row.soloBtn);
        row.soloBtn->setClickingTogglesState (true);
        row.soloBtn->setToggleState (tracks[i].solo, juce::dontSendNotification);
        row.soloBtn->onClick = [this, trackIdx] { engine.toggleSolo (trackIdx); };

        // Warp Mode Button
        row.warpBtn = std::make_unique<juce::TextButton> (tracks[i].warpMode == WarpMode::Varispeed ? "VARISPEED" : "STRETCH");
        addAndMakeVisible (*row.warpBtn);
        row.warpBtn->onClick = [this, trackIdx] {
            auto currentMode = engine.getTracks()[trackIdx].warpMode;
            auto newMode = (currentMode == WarpMode::Varispeed) ? WarpMode::Granular : WarpMode::Varispeed;
            engine.updateTrackWarpMode (trackIdx, newMode);
            trackRows[trackIdx].warpBtn->setButtonText (newMode == WarpMode::Varispeed ? "VARISPEED" : "STRETCH");
        };

        trackRows.push_back (std::move (row));
    }

    resized();
}

void TactileAudioStudioComponent::timerCallback()
{
    repaint();
}

void TactileAudioStudioComponent::mouseDown (const juce::MouseEvent& e)
{
    const float headerH = 45.0f;
    const float trackRowH = 65.0f;
    float startY = headerH + 30.0f;

    int clickedIdx = static_cast<int>((e.position.y - startY) / trackRowH);
    if (clickedIdx >= 0 && clickedIdx < static_cast<int>(engine.getTracks().size()))
    {
        selectedTrackIdx = clickedIdx;
        repaint();
    }
}

void TactileAudioStudioComponent::mouseDrag (const juce::MouseEvent& /*e*/)
{
}

void TactileAudioStudioComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0e17));

    // Top Banner Bar
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRect (0, 0, getWidth(), 45);

    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("TIME DILATION AUDIO WORKSTATION", 20, 12, 340, 20, juce::Justification::left);

    // Render Track Lanes
    const auto& tracks = engine.getTracks();
    const float headerH = 45.0f;
    const float trackListW = 340.0f;
    const float timelineW = getWidth() - trackListW - 10.0f;
    const float trackRowH = 65.0f;

    // Time Ruler
    g.setColour (juce::Colour (0xff141a26));
    g.fillRect (trackListW, headerH, timelineW, 25.0f);
    g.setColour (juce::Colour (0xff64748b));
    g.setFont (juce::FontOptions (10.0f, juce::Font::plain));

    for (int sec = 0; sec < 30; sec += 2)
    {
        float x = trackListW + sec * 30.0f;
        g.drawText (juce::String (sec) + "s", x + 2.0f, headerH + 4.0f, 30.0f, 16.0f, juce::Justification::left);
        g.drawLine (x, headerH, x, headerH + 25.0f, 1.0f);
    }

    float y = headerH + 30.0f;

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        const auto& t = tracks[i];
        bool isSel = (static_cast<int>(i) == selectedTrackIdx);

        // Header Card
        g.setColour (isSel ? juce::Colour (0xff1e293b) : (i % 2 == 0 ? juce::Colour (0xff121824) : juce::Colour (0xff0f141d)));
        g.fillRect (10.0f, y, trackListW - 15.0f, trackRowH - 5.0f);

        if (isSel)
        {
            g.setColour (juce::Colour (0xfff59e0b));
            g.drawRect (10.0f, y, trackListW - 15.0f, trackRowH - 5.0f, 1.5f);
        }

        g.setColour (t.color);
        g.fillRect (10.0f, y, 5.0f, trackRowH - 5.0f);

        g.setColour (juce::Colour (0xffffffff));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (t.name, 20.0f, y + 6.0f, 110.0f, 16.0f, juce::Justification::left);

        // Timeline Track Lane
        g.setColour (juce::Colour (0xff141a26));
        g.fillRect (trackListW, y, timelineW, trackRowH - 5.0f);

        // Draw Loaded Audio Waveform or Drop Prompt
        if (t.hasAudioFile && t.importedAudioBuffer.getNumSamples() > 0)
        {
            float durationSec = static_cast<float>(t.importedAudioBuffer.getNumSamples()) / 44100.0f;
            float waveW = durationSec * 30.0f;
            drawWaveformForBuffer (g, t.importedAudioBuffer, { trackListW + 2.0f, y + 4.0f, waveW, trackRowH - 13.0f }, t.color);
        }
        else
        {
            g.setColour (juce::Colour (0xff334155));
            g.setFont (juce::FontOptions (11.0f, juce::Font::italic));
            g.drawText ("DRAG & DROP WAV/MP3 FILE HERE", trackListW + 20.0f, y + 18.0f, 300.0f, 20.0f, juce::Justification::left);
        }

        // Draw Per-Track Independent Playhead Cursor (properTime tau)
        float headX = trackListW + static_cast<float>(t.properTime) * 30.0f;
        g.setColour (t.color);
        g.drawLine (headX, y, headX, y + trackRowH - 5.0f, 2.0f);

        // Draw Loop Markers if enabled
        if (t.isLooping)
        {
            float loopStartX = trackListW + static_cast<float>(t.loopStartTau) * 30.0f;
            float loopEndX = trackListW + static_cast<float>(t.loopEndTau) * 30.0f;

            g.setColour (juce::Colour (0xff10b981).withAlpha (0.15f));
            g.fillRect (loopStartX, y, loopEndX - loopStartX, trackRowH - 5.0f);

            g.setColour (juce::Colour (0xff10b981));
            g.drawVerticalLine (static_cast<int>(loopStartX), y, y + trackRowH - 5.0f);
            g.drawVerticalLine (static_cast<int>(loopEndX), y, y + trackRowH - 5.0f);
        }

        y += trackRowH;
    }
}

void TactileAudioStudioComponent::drawWaveformForBuffer (juce::Graphics& g, const juce::AudioBuffer<float>& buffer, juce::Rectangle<float> bounds, juce::Colour color)
{
    g.setColour (color.withAlpha (0.2f));
    g.fillRoundedRectangle (bounds, 3.0f);

    g.setColour (color);
    const float* samples = buffer.getReadPointer (0);
    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0) return;

    juce::Path p;
    p.startNewSubPath (bounds.getX(), bounds.getCentreY());

    const int widthPixels = static_cast<int>(bounds.getWidth());
    const int samplesPerPixel = juce::jmax (1, numSamples / widthPixels);

    for (int x = 0; x < widthPixels; ++x)
    {
        int startSample = x * samplesPerPixel;
        float maxVal = 0.0f;
        for (int s = 0; s < samplesPerPixel && (startSample + s) < numSamples; ++s)
        {
            maxVal = juce::jmax (maxVal, std::abs (samples[startSample + s]));
        }

        float yHeight = maxVal * (bounds.getHeight() * 0.45f);
        p.lineTo (bounds.getX() + x, bounds.getCentreY() - yHeight);
    }
    g.strokePath (p, juce::PathStrokeType (1.2f));
}

void TactileAudioStudioComponent::resized()
{
    const float headerH = 45.0f;
    const float trackListW = 340.0f;

    playButton.setBounds (getWidth() - 560, 10, 50, 25);
    pauseButton.setBounds (getWidth() - 505, 10, 50, 25);
    stopButton.setBounds (getWidth() - 450, 10, 50, 25);
    bounceButton.setBounds (getWidth() - 395, 10, 160, 25);

    masterGammaLabel.setBounds (getWidth() - 130, 4, 90, 16);
    masterGammaSlider.setBounds (getWidth() - 130, 18, 110, 24);

    addTrackButton.setBounds (10, getHeight() - 35, 140, 26);

    // Position per-track UI rows
    const float trackRowH = 65.0f;
    float y = headerH + 30.0f;

    for (size_t i = 0; i < trackRows.size(); ++i)
    {
        auto& row = trackRows[i];

        if (row.muteBtn) row.muteBtn->setBounds (135.0f, y + 6.0f, 20, 18);
        if (row.soloBtn) row.soloBtn->setBounds (158.0f, y + 6.0f, 20, 18);
        if (row.loopBtn) row.loopBtn->setBounds (181.0f, y + 6.0f, 42, 18);
        if (row.warpBtn) row.warpBtn->setBounds (226.0f, y + 6.0f, 65, 18);
        if (row.gammaSlider) row.gammaSlider->setBounds (293.0f, y + 4.0f, 38, 52);
        if (row.volSlider) row.volSlider->setBounds (20.0f, y + 32.0f, 130, 18);

        y += trackRowH;
    }
}

} // namespace time_dilation
