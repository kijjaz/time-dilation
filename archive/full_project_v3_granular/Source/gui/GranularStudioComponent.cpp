#include "GranularStudioComponent.h"

namespace time_dilation
{

GranularStudioComponent::GranularStudioComponent (GranularEngine& e)
    : granularEngine (e)
{
    auto setupRotary = [this] (juce::Slider& s, juce::Label& l, const juce::String& text, double min, double max, double init) {
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 55, 16);
        s.setRange (min, max, 0.1);
        s.setValue (init);

        addAndMakeVisible (l);
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
    };

    setupRotary (grainSizeSlider, grainSizeLabel, "GRAIN SIZE (MS)", 10.0, 500.0, 80.0);
    grainSizeSlider.onValueChange = [this] { granularEngine.setGrainSizeMs ((float) grainSizeSlider.getValue()); };

    setupRotary (densitySlider, densityLabel, "DENSITY (HZ)", 1.0, 100.0, 30.0);
    densitySlider.onValueChange = [this] { granularEngine.setGrainDensity ((float) densitySlider.getValue()); };

    setupRotary (pitchSlider, pitchLabel, "PITCH (SEMI)", -24.0, 24.0, 0.0);
    pitchSlider.onValueChange = [this] { granularEngine.setPitchSemitones ((float) pitchSlider.getValue()); };

    setupRotary (pitchJitterSlider, pitchJitterLabel, "PITCH JITTER", 0.0, 12.0, 0.2);
    pitchJitterSlider.onValueChange = [this] { granularEngine.setPitchJitter ((float) pitchJitterSlider.getValue()); };

    setupRotary (spraySlider, sprayLabel, "POS SPRAY", 0.0, 0.5, 0.1);
    spraySlider.onValueChange = [this] { granularEngine.setPositionSpray ((float) spraySlider.getValue()); };

    setupRotary (panSpreadSlider, panSpreadLabel, "STEREO PAN", 0.0, 1.0, 0.5);
    panSpreadSlider.onValueChange = [this] { granularEngine.setStereoPanSpread ((float) panSpreadSlider.getValue()); };

    addAndMakeVisible (exportButton);
    exportButton.onClick = [this] {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon, "GRANULAR RESYNTHESIS ACTIVE", "Granular cloud is processing live in real-time!");
    };

    startTimerHz (30);
}

GranularStudioComponent::~GranularStudioComponent()
{
    stopTimer();
}

void GranularStudioComponent::timerCallback()
{
    repaint();
}

bool GranularStudioComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& file : files)
    {
        if (file.endsWithIgnoreCase (".wav") || file.endsWithIgnoreCase (".mp3") || file.endsWithIgnoreCase (".aif") || file.endsWithIgnoreCase (".flac"))
            return true;
    }
    return false;
}

void GranularStudioComponent::filesDropped (const juce::StringArray& files, int /*x*/, int /*y*/)
{
    for (const auto& file : files)
    {
        juce::File audioFile (file);
        if (audioFile.existsAsFile())
        {
            granularEngine.loadSample (audioFile);
            break;
        }
    }
    repaint();
}

void GranularStudioComponent::mouseDown (const juce::MouseEvent& e)
{
    const float waveY = 55.0f;
    const float waveH = 220.0f;

    if (e.position.y >= waveY && e.position.y <= waveY + waveH)
    {
        float normPos = (e.position.x - 20.0f) / (getWidth() - 40.0f);
        granularEngine.setScanPosition (normPos);
        repaint();
    }
}

void GranularStudioComponent::mouseDrag (const juce::MouseEvent& e)
{
    const float waveY = 55.0f;
    const float waveH = 220.0f;

    if (e.position.y >= waveY && e.position.y <= waveY + waveH)
    {
        float normPos = (e.position.x - 20.0f) / (getWidth() - 40.0f);
        granularEngine.setScanPosition (normPos);
        repaint();
    }
}

void GranularStudioComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0e17));

    // Top Header Status Banner
    g.setColour (juce::Colour (0xff121824));
    g.fillRect (0, 0, getWidth(), 45);

    g.setColour (juce::Colour (0xff00ff66));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("GRANULAR TEXTURE & RESYNTHESIS MACHINE", 20, 12, 440, 20, juce::Justification::left);

    g.setColour (juce::Colour (0xff94a3b8));
    g.setFont (juce::FontOptions (11.0f, juce::Font::plain));
    g.drawText ("DRONE, PAD & ATMOSPHERIC SOUND DESIGN LABORATORY", 470, 14, 380, 20, juce::Justification::left);

    // Waveform Display Container
    const float waveX = 20.0f;
    const float waveY = 55.0f;
    const float waveW = getWidth() - 40.0f;
    const float waveH = 220.0f;

    g.setColour (juce::Colour (0xff141a26));
    g.fillRoundedRectangle (waveX, waveY, waveW, waveH, 6.0f);

    g.setColour (juce::Colour (0xff00ff66));
    g.drawRoundedRectangle (waveX, waveY, waveW, waveH, 6.0f, 1.5f);

    if (granularEngine.hasSample())
    {
        drawWaveform (g, { waveX + 5.0f, waveY + 5.0f, waveW - 10.0f, waveH - 10.0f });

        // Draw Interactive Grain Pointer Cursor
        float cursorX = waveX + granularEngine.getScanPosition() * waveW;
        g.setColour (juce::Colour (0xffffb000));
        g.drawLine (cursorX, waveY, cursorX, waveY + waveH, 2.5f);
        g.fillEllipse (cursorX - 6.0f, waveY + waveH * 0.5f - 6.0f, 12.0f, 12.0f);
    }
    else
    {
        g.setColour (juce::Colour (0xff64748b));
        g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
        g.drawText ("DRAG & DROP ANY WAV/MP3 AUDIO FILE HERE TO BEGIN GRANULAR RESYNTHESIS", waveX, waveY, waveW, waveH, juce::Justification::centred);
    }

    // Bottom Device Controls Container
    const float rackY = getHeight() - 190.0f;
    g.setColour (juce::Colour (0xff121824));
    g.fillRect (0.0f, rackY, (float) getWidth(), 190.0f);

    g.setColour (juce::Colour (0xff334155));
    g.drawLine (0.0f, rackY, (float) getWidth(), rackY, 1.0f);

    g.setColour (juce::Colour (0xff00ff66));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("GRAIN CLOUD GENERATOR & SPATIAL PARAMETERS", 20, rackY + 8, 400, 18, juce::Justification::left);
}

void GranularStudioComponent::drawWaveform (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const auto& buffer = granularEngine.getSampleBuffer();
    if (buffer.getNumSamples() <= 0) return;

    g.setColour (juce::Colour (0xff00ff66).withAlpha (0.35f));
    const float* samples = buffer.getReadPointer (0);
    const int numSamples = buffer.getNumSamples();

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
    g.strokePath (p, juce::PathStrokeType (1.5f));
}

void GranularStudioComponent::resized()
{
    const float rackY = getHeight() - 190.0f;
    const float knobY = rackY + 38.0f;
    const float knobSize = 85.0f;

    grainSizeSlider.setBounds (30, knobY, knobSize, knobSize);
    grainSizeLabel.setBounds (30, knobY + knobSize + 2.0f, knobSize, 16);

    densitySlider.setBounds (140, knobY, knobSize, knobSize);
    densityLabel.setBounds (140, knobY + knobSize + 2.0f, knobSize, 16);

    pitchSlider.setBounds (250, knobY, knobSize, knobSize);
    pitchLabel.setBounds (250, knobY + knobSize + 2.0f, knobSize, 16);

    pitchJitterSlider.setBounds (360, knobY, knobSize, knobSize);
    pitchJitterLabel.setBounds (360, knobY + knobSize + 2.0f, knobSize, 16);

    spraySlider.setBounds (470, knobY, knobSize, knobSize);
    sprayLabel.setBounds (470, knobY + knobSize + 2.0f, knobSize, 16);

    panSpreadSlider.setBounds (580, knobY, knobSize, knobSize);
    panSpreadLabel.setBounds (580, knobY + knobSize + 2.0f, knobSize, 16);

    playButton.setBounds (getWidth() - 250, rackY + 40.0f, 220, 32);
    exportButton.setBounds (getWidth() - 250, rackY + 85.0f, 220, 32);
}

} // namespace time_dilation
