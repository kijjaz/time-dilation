#include "TimelineSequencerComponent.h"

namespace time_dilation
{

TimelineSequencerComponent::TimelineSequencerComponent (TimeDilationEngine& e)
    : engine (e),
      keyboardComponent (e.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Play / Pause / Stop Buttons
    addAndMakeVisible (playButton);
    playButton.onClick = [this] { engine.play(); };

    addAndMakeVisible (pauseButton);
    pauseButton.onClick = [this] { engine.pause(); };

    addAndMakeVisible (stopButton);
    stopButton.onClick = [this] { engine.stop(); };

    // BPM Slider
    addAndMakeVisible (bpmSlider);
    bpmSlider.setRange (40.0, 240.0, 1.0);
    bpmSlider.setValue (engine.getBpm());
    bpmSlider.onValueChange = [this] { engine.setBpm ((float) bpmSlider.getValue()); };

    addAndMakeVisible (bpmLabel);
    bpmLabel.setText ("BPM", juce::dontSendNotification);

    // Master Gamma Dilation Slider
    addAndMakeVisible (masterGammaSlider);
    masterGammaSlider.setRange (0.1, 4.0, 0.05);
    masterGammaSlider.setValue (engine.getMasterDilation());
    masterGammaSlider.onValueChange = [this] { engine.setMasterDilation ((float) masterGammaSlider.getValue()); };

    addAndMakeVisible (masterGammaLabel);
    masterGammaLabel.setText ("MASTER GAMMA", juce::dontSendNotification);

    // Virtual MIDI Keyboard
    addAndMakeVisible (keyboardComponent);

    rebuildTrackControls();
    startTimerHz (20);
}

TimelineSequencerComponent::~TimelineSequencerComponent()
{
    stopTimer();
}

bool TimelineSequencerComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        juce::File file (f);
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".mp3" || ext == ".flac" || ext == ".aiff")
            return true;
    }
    return false;
}

void TimelineSequencerComponent::filesDropped (const juce::StringArray& files, int /*x*/, int y)
{
    int trackIndex = (y - 50) / 45;
    trackIndex = juce::jlimit (0, static_cast<int>(engine.getTracks().size()) - 1, trackIndex);

    for (const auto& f : files)
    {
        juce::File file (f);
        if (engine.importAudioFile (trackIndex, file))
        {
            rebuildTrackControls();
            repaint();
            break;
        }
    }
}

void TimelineSequencerComponent::rebuildTrackControls()
{
    rowControls.clear();
    const auto& tracks = engine.getTracks();

    for (size_t t = 0; t < tracks.size(); ++t)
    {
        auto row = std::make_unique<TrackRowControls>();
        int trackIdx = static_cast<int>(t);

        // Track Gamma Slider
        addAndMakeVisible (row->gammaSlider);
        row->gammaSlider.setRange (0.1, 4.0, 0.05);
        row->gammaSlider.setValue (tracks[t].timeDilation);
        row->gammaSlider.onValueChange = [this, trackIdx, r = row.get()] {
            engine.updateTrackGamma (trackIdx, (float) r->gammaSlider.getValue());
        };

        // Volume Slider
        addAndMakeVisible (row->volumeSlider);
        row->volumeSlider.setRange (0.0, 1.2, 0.02);
        row->volumeSlider.setValue (tracks[t].volume);
        row->volumeSlider.onValueChange = [this, trackIdx, r = row.get()] {
            engine.updateTrackVolume (trackIdx, (float) r->volumeSlider.getValue());
        };

        // WarpMode ComboBox
        addAndMakeVisible (row->warpModeBox);
        row->warpModeBox.addItem ("VARISPEED", 1);
        row->warpModeBox.addItem ("GRANULAR", 2);
        row->warpModeBox.addItem ("DOPPLER", 3);
        row->warpModeBox.setSelectedId (static_cast<int>(tracks[t].warpMode) + 1, juce::dontSendNotification);
        row->warpModeBox.onChange = [this, trackIdx, r = row.get()] {
            engine.updateTrackWarpMode (trackIdx, static_cast<WarpMode>(r->warpModeBox.getSelectedId() - 1));
        };

        // Mute / Solo Buttons
        addAndMakeVisible (row->muteButton);
        row->muteButton.onClick = [this, trackIdx] { engine.toggleMute (trackIdx); };

        addAndMakeVisible (row->soloButton);
        row->soloButton.onClick = [this, trackIdx] { engine.toggleSolo (trackIdx); };

        // 16 Step Sequencer Buttons
        for (int step = 0; step < 16; ++step)
        {
            auto stepBtn = std::make_unique<juce::TextButton> (juce::String (step + 1));
            addAndMakeVisible (*stepBtn);
            stepBtn->setClickingTogglesState (true);
            stepBtn->setToggleState (tracks[t].steps[step], juce::dontSendNotification);

            stepBtn->onClick = [this, trackIdx, step, btn = stepBtn.get()] {
                engine.updateTrackStep (trackIdx, step, btn->getToggleState());
            };

            row->stepButtons.push_back (std::move (stepBtn));
        }

        rowControls.push_back (std::move (row));
    }
}

void TimelineSequencerComponent::timerCallback()
{
    repaint();
}

void TimelineSequencerComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff10141d));

    auto bounds = getLocalBounds();
    g.setColour (juce::Colour (0xff263147));
    g.drawRect (bounds, 1);

    // Render Track Names, Sub-Track Indentations & Proper Time (tau) Metering
    const auto& tracks = engine.getTracks();
    int yOffset = 50;

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        float indent = (tracks[i].parentTrackIndex >= 0) ? 20.0f : 0.0f;

        g.setColour (tracks[i].color);
        g.fillEllipse (10.0f + indent, yOffset + 12.0f, 10.0f, 10.0f);

        g.setColour (juce::Colour (0xffffffff));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (tracks[i].name, 26 + indent, yOffset + 8, 140 - indent, 20, juce::Justification::left);

        // Render Proper Time (tau) vs Velocity Meter
        g.setColour (juce::Colour (0xfff59e0b));
        g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
        g.drawText (juce::String::formatted ("tau: %.2fs (%.2fx)", tracks[i].properTime, tracks[i].timeVelocity),
                    180, yOffset + 28, 120, 14, juce::Justification::left);

        yOffset += 45;
    }
}

void TimelineSequencerComponent::resized()
{
    // Layout Header Controls
    playButton.setBounds (10, 8, 60, 26);
    pauseButton.setBounds (75, 8, 60, 26);
    stopButton.setBounds (140, 8, 60, 26);

    bpmLabel.setBounds (220, 8, 40, 26);
    bpmSlider.setBounds (260, 8, 100, 26);

    masterGammaLabel.setBounds (380, 8, 100, 26);
    masterGammaSlider.setBounds (480, 8, 120, 26);

    // Layout Track Rows
    int y = 50;
    for (size_t i = 0; i < rowControls.size(); ++i)
    {
        auto& row = rowControls[i];
        row->gammaSlider.setBounds (190, y + 6, 75, 24);
        row->volumeSlider.setBounds (270, y + 6, 55, 24);
        row->warpModeBox.setBounds (330, y + 6, 85, 24);
        row->muteButton.setBounds (420, y + 6, 24, 24);
        row->soloButton.setBounds (448, y + 6, 24, 24);

        int stepX = 480;
        for (int s = 0; s < 16; ++s)
        {
            row->stepButtons[s]->setBounds (stepX, y + 6, 22, 24);
            stepX += 24;
        }

        y += 45;
    }

    // Layout Virtual Keyboard at bottom
    keyboardComponent.setBounds (10, getHeight() - 65, getWidth() - 20, 55);
}

} // namespace time_dilation
