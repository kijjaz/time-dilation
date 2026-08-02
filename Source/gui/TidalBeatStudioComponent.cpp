#include "TidalBeatStudioComponent.h"

namespace time_dilation
{

TidalBeatStudioComponent::TidalBeatStudioComponent (TidalBeatEngine& e)
    : beatEngine (e)
{
    addAndMakeVisible (modeButton);
    modeButton.onClick = [this] {
        if (beatEngine.getTimeMode() == TimeMode::BeatMode)
        {
            beatEngine.setTimeMode (TimeMode::CycleMode);
            modeButton.setButtonText ("CYCLE MODE (TidalCycles)");
        }
        else
        {
            beatEngine.setTimeMode (TimeMode::BeatMode);
            modeButton.setButtonText ("BEAT MODE (Tidal 2.0)");
        }
        repaint();
    };

    addAndMakeVisible (playButton);
    playButton.onClick = [this] {
        beatEngine.togglePlay (!beatEngine.isPlaying());
        playButton.setButtonText (beatEngine.isPlaying() ? "STOP PERMUTATIONS" : "PLAY PERMUTATIONS");
    };

    addAndMakeVisible (bpmSlider);
    bpmSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 55, 16);
    bpmSlider.setRange (60.0, 240.0, 1.0);
    bpmSlider.setValue (135.0);
    bpmSlider.onValueChange = [this] { beatEngine.setBpm (bpmSlider.getValue()); };

    addAndMakeVisible (bpmLabel);
    bpmLabel.setText ("TEMPO (BPM)", juce::dontSendNotification);
    bpmLabel.setJustificationType (juce::Justification::centred);

    auto setupMotif = [this] (juce::TextButton& btn, int id) {
        addAndMakeVisible (btn);
        btn.onClick = [this, id] {
            beatEngine.setMotifPattern (id);
            repaint();
        };
    };

    setupMotif (motif1Button, 0);
    setupMotif (motif2Button, 1);
    setupMotif (motif3Button, 2);
    setupMotif (motif4Button, 3);

    addAndMakeVisible (exportButton);
    exportButton.onClick = [this] {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon, "CARNATIC BEATMODE ACTIVE", "TidalBeat 2.0 engine is generating live Carnatic Konnakol permutations!");
    };

    startTimerHz (30);
}

TidalBeatStudioComponent::~TidalBeatStudioComponent()
{
    stopTimer();
}

void TidalBeatStudioComponent::timerCallback()
{
    repaint();
}

void TidalBeatStudioComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff090d16));

    // Top Header Banner
    g.setColour (juce::Colour (0xff111827));
    g.fillRect (0, 0, getWidth(), 45);

    g.setColour (juce::Colour (0xff00e5ff));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("TIDAL 2.0 BEATMODE & CARNATIC SOLKATTU WORKSTATION", 20, 12, 540, 20, juce::Justification::left);

    g.setColour (juce::Colour (0xff94a3b8));
    g.setFont (juce::FontOptions (11.0f, juce::Font::plain));
    g.drawText ("NUMERICAL RHYTHMIC PERMUTATIONS & ADI TALA 8-BEAT CYCLES", 570, 14, 380, 20, juce::Justification::left);

    // Main Beat Matrix Canvas
    const float matrixX = 20.0f;
    const float matrixY = 55.0f;
    const float matrixW = getWidth() - 40.0f;
    const float matrixH = 240.0f;

    g.setColour (juce::Colour (0xff141c2e));
    g.fillRoundedRectangle (matrixX, matrixY, matrixW, matrixH, 8.0f);

    g.setColour (juce::Colour (0xff00e5ff));
    g.drawRoundedRectangle (matrixX, matrixY, matrixW, matrixH, 8.0f, 1.5f);

    // Draw Adi Tala 8-Beat Pulse Subdivisions
    const int totalBeats = 16;
    const float beatWidth = matrixW / static_cast<float>(totalBeats);

    for (int b = 0; b < totalBeats; ++b)
    {
        float bx = matrixX + b * beatWidth;
        g.setColour (b % 4 == 0 ? juce::Colour (0xff334155) : juce::Colour (0xff1e293b));
        g.drawLine (bx, matrixY, bx, matrixY + matrixH, (b % 4 == 0) ? 1.5f : 0.8f);

        g.setColour (juce::Colour (0xff64748b));
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText (juce::String (b + 1), bx + 4, matrixY + 4, beatWidth - 4, 14, juce::Justification::left);
    }

    // Draw Active Carnatic Solkattu Events
    const auto& events = beatEngine.getActiveEvents();
    const static juce::Colour sylColours[] = {
        juce::Colour (0xff00ff66), // Tha - Green
        juce::Colour (0xffffb000), // Dhi - Amber
        juce::Colour (0xffa855f7), // Thom - Purple
        juce::Colour (0xff00e5ff), // Nam - Cyan
        juce::Colour (0xfff43f5e), // Tarikita - Red
        juce::Colour (0xffe11d48)  // Thakadimi - Dark Red
    };

    const static juce::String sylNames[] = { "Tha", "Dhi", "Thom", "Nam", "Tarikita", "Thakadimi" };

    for (const auto& ev : events)
    {
        float ex = matrixX + (ev.beatPosition / static_cast<float>(totalBeats)) * matrixW;
        float ew = (ev.durationBeats / static_cast<float>(totalBeats)) * matrixW - 3.0f;
        float ey = matrixY + 30.0f + (ev.syllableIndex % 4) * 45.0f;
        float eh = 36.0f;

        if (ex + ew <= matrixX + matrixW)
        {
            juce::Colour c = sylColours[ev.syllableIndex % 6];
            g.setColour (c.withAlpha (0.85f));
            g.fillRoundedRectangle (ex, ey, ew, eh, 4.0f);

            g.setColour (juce::Colours::black);
            g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
            g.drawText (sylNames[ev.syllableIndex % 6] + " (" + juce::String (ev.countValue) + ")", ex + 4, ey, ew - 4, eh, juce::Justification::centred);
        }
    }

    // Draw Live Beat Cursor Line
    float cursorX = matrixX + (beatEngine.getCurrentBeatPosition() / static_cast<float>(totalBeats)) * matrixW;
    g.setColour (juce::Colour (0xffffffff));
    g.drawLine (cursorX, matrixY, cursorX, matrixY + matrixH, 3.0f);

    // Bottom Device Controls Container
    const float rackY = getHeight() - 170.0f;
    g.setColour (juce::Colour (0xff111827));
    g.fillRect (0.0f, rackY, (float) getWidth(), 170.0f);

    g.setColour (juce::Colour (0xff1e293b));
    g.drawLine (0.0f, rackY, (float) getWidth(), rackY, 1.0f);

    g.setColour (juce::Colour (0xff00e5ff));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("CARNATIC MOTIF SELECTION & TIMING ENGINE", 20, rackY + 8, 400, 18, juce::Justification::left);
}

void TidalBeatStudioComponent::resized()
{
    const float rackY = getHeight() - 170.0f;

    modeButton.setBounds (20, rackY + 38, 200, 32);
    playButton.setBounds (230, rackY + 38, 160, 32);

    bpmSlider.setBounds (410, rackY + 32, 85, 85);
    bpmLabel.setBounds (410, rackY + 32 + 85 + 2, 85, 16);

    const float btnX = 520;
    motif1Button.setBounds (btnX, rackY + 35, 170, 28);
    motif2Button.setBounds (btnX + 180, rackY + 35, 170, 28);
    motif3Button.setBounds (btnX, rackY + 70, 170, 28);
    motif4Button.setBounds (btnX + 180, rackY + 70, 260, 28);

    exportButton.setBounds (getWidth() - 250, rackY + 115, 220, 32);
}

} // namespace time_dilation
