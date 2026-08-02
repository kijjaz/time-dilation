#include "TrackInspectorComponent.h"

namespace time_dilation
{

TrackInspectorComponent::TrackInspectorComponent (TimeDilationEngine& e)
    : engine (e)
{
    // Volume
    addAndMakeVisible (volumeSlider);
    volumeSlider.setRange (0.0, 1.2, 0.02);
    volumeSlider.onValueChange = [this] {
        engine.updateTrackVolume (currentTrackIndex, (float) volumeSlider.getValue());
    };
    addAndMakeVisible (volumeLabel);
    volumeLabel.setText ("VOLUME", juce::dontSendNotification);

    // Pan
    addAndMakeVisible (panSlider);
    panSlider.setRange (-1.0, 1.0, 0.05);
    panSlider.onValueChange = [this] {
        engine.updateTrackPan (currentTrackIndex, (float) panSlider.getValue());
    };
    addAndMakeVisible (panLabel);
    panLabel.setText ("PAN", juce::dontSendNotification);

    // Gamma
    addAndMakeVisible (gammaSlider);
    gammaSlider.setRange (0.1, 4.0, 0.05);
    gammaSlider.onValueChange = [this] {
        engine.updateTrackGamma (currentTrackIndex, (float) gammaSlider.getValue());
    };
    addAndMakeVisible (gammaLabel);
    gammaLabel.setText ("TRACK GAMMA", juce::dontSendNotification);

    // Warp Mode
    addAndMakeVisible (warpModeBox);
    warpModeBox.addItem ("VARISPEED", 1);
    warpModeBox.addItem ("GRANULAR", 2);
    warpModeBox.addItem ("DOPPLER", 3);
    warpModeBox.onChange = [this] {
        engine.updateTrackWarpMode (currentTrackIndex, static_cast<WarpMode>(warpModeBox.getSelectedId() - 1));
    };
    addAndMakeVisible (warpModeLabel);
    warpModeLabel.setText ("WARP MODE", juce::dontSendNotification);

    // Loop Controls
    addAndMakeVisible (loopToggle);
    loopToggle.onClick = [this] {
        if (currentTrackIndex >= 0 && currentTrackIndex < static_cast<int>(engine.getTracks().size()))
        {
            engine.getTracksMutable()[currentTrackIndex].isLooping = loopToggle.getToggleState();
        }
    };

    addAndMakeVisible (loopStartSlider);
    loopStartSlider.setRange (0.0, 64.0, 0.5);
    loopStartSlider.onValueChange = [this] {
        if (currentTrackIndex >= 0 && currentTrackIndex < static_cast<int>(engine.getTracks().size()))
        {
            engine.getTracksMutable()[currentTrackIndex].loopStartTau = loopStartSlider.getValue();
        }
    };

    addAndMakeVisible (loopEndSlider);
    loopEndSlider.setRange (0.5, 128.0, 0.5);
    loopEndSlider.onValueChange = [this] {
        if (currentTrackIndex >= 0 && currentTrackIndex < static_cast<int>(engine.getTracks().size()))
        {
            engine.getTracksMutable()[currentTrackIndex].loopEndTau = loopEndSlider.getValue();
        }
    };

    addAndMakeVisible (loopLabel);
    loopLabel.setText ("LOOP RANGE (TAU)", juce::dontSendNotification);

    // Mute / Solo
    addAndMakeVisible (muteButton);
    muteButton.onClick = [this] { engine.toggleMute (currentTrackIndex); };

    addAndMakeVisible (soloButton);
    soloButton.onClick = [this] { engine.toggleSolo (currentTrackIndex); };
}

void TrackInspectorComponent::setSelectedTrack (int trackIndex)
{
    currentTrackIndex = trackIndex;
    const auto& tracks = engine.getTracks();
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        const auto& t = tracks[trackIndex];
        volumeSlider.setValue (t.volume, juce::dontSendNotification);
        panSlider.setValue (t.pan, juce::dontSendNotification);
        gammaSlider.setValue (t.timeDilation, juce::dontSendNotification);
        warpModeBox.setSelectedId (static_cast<int>(t.warpMode) + 1, juce::dontSendNotification);
        loopToggle.setToggleState (t.isLooping, juce::dontSendNotification);
        loopStartSlider.setValue (t.loopStartTau, juce::dontSendNotification);
        loopEndSlider.setValue (t.loopEndTau, juce::dontSendNotification);
    }
}

void TrackInspectorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff10141d));
    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));

    juce::String trackName = (currentTrackIndex >= 0 && currentTrackIndex < static_cast<int>(engine.getTracks().size()))
                           ? engine.getTracks()[currentTrackIndex].name
                           : "NO TRACK SELECTED";

    g.drawText ("TRACK INSPECTOR — " + trackName, 10, 6, 300, 20, juce::Justification::left);
}

void TrackInspectorComponent::resized()
{
    int y = 30;

    volumeLabel.setBounds (10, y, 60, 20);
    volumeSlider.setBounds (75, y, 100, 20);

    panLabel.setBounds (185, y, 40, 20);
    panSlider.setBounds (230, y, 80, 20);

    gammaLabel.setBounds (320, y, 90, 20);
    gammaSlider.setBounds (410, y, 100, 20);

    y += 28;

    warpModeLabel.setBounds (10, y, 80, 20);
    warpModeBox.setBounds (95, y, 95, 20);

    muteButton.setBounds (200, y, 50, 20);
    soloButton.setBounds (255, y, 50, 20);

    loopToggle.setBounds (320, y, 110, 20);
    loopStartSlider.setBounds (435, y, 50, 20);
    loopEndSlider.setBounds (490, y, 50, 20);
}

} // namespace time_dilation
