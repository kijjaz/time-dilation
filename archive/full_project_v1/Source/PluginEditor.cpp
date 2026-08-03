#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace time_dilation
{

TimeDilationAudioProcessorEditor::TimeDilationAudioProcessorEditor (TimeDilationAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      studioComponent (p.getEngine())
{
    auto appDir = juce::File::getCurrentWorkingDirectory();
    FontManager::getInstance().loadFonts (appDir.getChildFile ("Source").getChildFile ("assets").getChildFile ("fonts"));

    juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

    addAndMakeVisible (studioComponent);

    // Transport Bar Controls
    addAndMakeVisible (playButton);
    playButton.onClick = [this] { audioProcessor.getEngine().play(); };

    addAndMakeVisible (pauseButton);
    pauseButton.onClick = [this] { audioProcessor.getEngine().pause(); };

    addAndMakeVisible (stopButton);
    stopButton.onClick = [this] { audioProcessor.getEngine().stop(); };

    addAndMakeVisible (bpmSlider);
    bpmSlider.setRange (40.0, 240.0, 1.0);
    bpmSlider.setValue (audioProcessor.getEngine().getBpm());
    bpmSlider.onValueChange = [this] { audioProcessor.getEngine().setBpm ((float) bpmSlider.getValue()); };

    addAndMakeVisible (bpmLabel);
    bpmLabel.setText ("BPM", juce::dontSendNotification);

    addAndMakeVisible (masterGammaSlider);
    masterGammaSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    masterGammaSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 55, 16);
    masterGammaSlider.setRange (-4.0, 4.0, 0.05);
    masterGammaSlider.setValue (1.0);
    masterGammaSlider.onValueChange = [this] { audioProcessor.getEngine().setMasterDilation ((float) masterGammaSlider.getValue()); };

    addAndMakeVisible (masterGammaLabel);
    masterGammaLabel.setText ("MASTER GAMMA", juce::dontSendNotification);

    addAndMakeVisible (audioSettingsButton);
    audioSettingsButton.onClick = [this] {
        auto* dialog = new juce::DialogWindow::LaunchOptions();
        dialog->dialogTitle = "AUDIO INTERFACE & SAMPLE RATE SETTINGS";
        dialog->content.setOwned (new AudioSettingsComponent (audioProcessor.getEngine().getTracktionEngine().getDeviceManager().deviceManager));
        dialog->dialogBackgroundColour = juce::Colour (0xff0f141d);
        dialog->escapeKeyTriggersCloseButton = true;
        dialog->useNativeTitleBar = true;
        dialog->resizable = true;
        dialog->launchAsync();
    };

    setResizable (true, true);
    setResizeLimits (800, 500, 3840, 2160);
    setSize (1180, 740);
}

TimeDilationAudioProcessorEditor::~TimeDilationAudioProcessorEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}

void TimeDilationAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0e17));

    // Top Header Banner
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRect (0, 0, getWidth(), 45);

    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("TIME DILATION DAW", 20, 12, 220, 20, juce::Justification::left);

    g.setColour (juce::Colour (0xff64748b));
    g.setFont (juce::FontOptions (11.0f, juce::Font::plain));
    g.drawText ("PRODUCED BY KIJJAZ - MINIMALIST STUDIO WORKSTATION", 240, 14, 340, 20, juce::Justification::left);
}

void TimeDilationAudioProcessorEditor::resized()
{
    const int headerH = 45;

    playButton.setBounds (getWidth() - 520, 10, 50, 25);
    pauseButton.setBounds (getWidth() - 465, 10, 50, 25);
    stopButton.setBounds (getWidth() - 410, 10, 50, 25);

    bpmLabel.setBounds (getWidth() - 350, 10, 32, 25);
    bpmSlider.setBounds (getWidth() - 315, 10, 60, 25);

    masterGammaLabel.setBounds (getWidth() - 240, 4, 100, 16);
    masterGammaSlider.setBounds (getWidth() - 240, 18, 120, 24);

    audioSettingsButton.setBounds (getWidth() - 100, 10, 90, 25);

    studioComponent.setBounds (0, headerH, getWidth(), getHeight() - headerH);
}

} // namespace time_dilation
