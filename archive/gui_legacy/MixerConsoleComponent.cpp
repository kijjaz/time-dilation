#include "MixerConsoleComponent.h"

namespace time_dilation
{

MixerConsoleComponent::MixerConsoleComponent (TimeDilationEngine& e)
    : engine (e)
{
    rebuildStrips();
    startTimerHz (20);
}

MixerConsoleComponent::~MixerConsoleComponent()
{
    stopTimer();
}

void MixerConsoleComponent::timerCallback()
{
    const auto& tracks = engine.getTracks();
    if (channelStrips.size() != tracks.size())
    {
        rebuildStrips();
        resized();
    }

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        if (i < channelStrips.size())
        {
            channelStrips[i]->peakLevel = tracks[i].currentAmplitude;
        }
    }
    repaint();
}

void MixerConsoleComponent::rebuildStrips()
{
    channelStrips.clear();
    const auto& tracks = engine.getTracks();

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        auto strip = std::make_unique<ChannelStripControls>();
        int trackIdx = static_cast<int>(i);

        // Volume Fader
        addAndMakeVisible (strip->volumeSlider);
        strip->volumeSlider.setSliderStyle (juce::Slider::LinearVertical);
        strip->volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        strip->volumeSlider.setRange (0.0, 1.2, 0.02);
        strip->volumeSlider.setValue (tracks[i].volume);
        strip->volumeSlider.onValueChange = [this, trackIdx, s = strip.get()] {
            engine.updateTrackVolume (trackIdx, (float) s->volumeSlider.getValue());
        };

        // Pan Knob
        addAndMakeVisible (strip->panSlider);
        strip->panSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        strip->panSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        strip->panSlider.setRange (-1.0, 1.0, 0.05);
        strip->panSlider.setValue (tracks[i].pan);
        strip->panSlider.onValueChange = [this, trackIdx, s = strip.get()] {
            engine.updateTrackPan (trackIdx, (float) s->panSlider.getValue());
        };

        // Mute / Solo / Arm
        addAndMakeVisible (strip->muteButton);
        strip->muteButton.onClick = [this, trackIdx] { engine.toggleMute (trackIdx); };

        addAndMakeVisible (strip->soloButton);
        strip->soloButton.onClick = [this, trackIdx] { engine.toggleSolo (trackIdx); };

        addAndMakeVisible (strip->armButton);

        channelStrips.push_back (std::move (strip));
    }
}

void MixerConsoleComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0f172a));

    const auto& tracks = engine.getTracks();
    const float stripWidth = 70.0f;
    float x = 10.0f;

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        // Channel background
        g.setColour (juce::Colour (0xff1e293b));
        g.fillRoundedRectangle (x, 6.0f, stripWidth - 6.0f, getHeight() - 12.0f, 4.0f);

        // Track Name & Color Strip
        g.setColour (tracks[i].color);
        g.fillRect (x, 6.0f, stripWidth - 6.0f, 4.0f);

        g.setColour (juce::Colour (0xffffffff));
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText (tracks[i].name, x + 2.0f, 12.0f, stripWidth - 10.0f, 16.0f, juce::Justification::centred);

        // Peak VU Meter
        if (i < channelStrips.size())
        {
            float peakH = (getHeight() - 90.0f) * channelStrips[i]->peakLevel;
            g.setColour (juce::Colour (0xff38bdf8));
            g.fillRoundedRectangle (x + stripWidth - 12.0f, getHeight() - 40.0f - peakH, 3.0f, peakH, 1.5f);
        }

        x += stripWidth;
    }
}

void MixerConsoleComponent::resized()
{
    const float stripWidth = 70.0f;
    float x = 10.0f;

    for (size_t i = 0; i < channelStrips.size(); ++i)
    {
        auto& strip = channelStrips[i];
        strip->panSlider.setBounds (x + 18, 30, 28, 28);
        strip->volumeSlider.setBounds (x + 15, 62, 24, getHeight() - 100);
        strip->muteButton.setBounds (x + 6, getHeight() - 32, 16, 16);
        strip->soloButton.setBounds (x + 24, getHeight() - 32, 16, 16);
        strip->armButton.setBounds (x + 42, getHeight() - 32, 16, 16);

        x += stripWidth;
    }
}

} // namespace time_dilation
