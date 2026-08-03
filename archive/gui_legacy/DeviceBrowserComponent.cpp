#include "DeviceBrowserComponent.h"

namespace time_dilation
{

DeviceBrowserComponent::DeviceBrowserComponent (TimeDilationEngine& e)
    : engine (e)
{
    thread.startThread();

    // File Browser for Samples
    auto userHome = juce::File::getSpecialLocation (juce::File::userHomeDirectory);
    directoryList.setDirectory (userHome, true, true);
    fileTree = std::make_unique<juce::FileTreeComponent> (directoryList);

    // Set Models
    midiFxModel.items = midiFxNames;
    midiFxList.setModel (&midiFxModel);

    instrumentsModel.items = instrumentNames;
    instrumentsList.setModel (&instrumentsModel);

    audioFxModel.items = audioFxNames;
    audioFxList.setModel (&audioFxModel);

    timeModulatorsModel.items = timeModulatorNames;
    timeModulatorsList.setModel (&timeModulatorsModel);

    // Double-click insert callbacks
    midiFxModel.onDoubleClicked = [this] (int /*row*/) {
        if (!engine.getTracks().empty())
        {
            // Insert MIDI FX onto active track
        }
    };

    instrumentsModel.onDoubleClicked = [this] (int /*row*/) {
        if (!engine.getTracks().empty())
        {
            // Attach Instrument to active track
        }
    };

    audioFxModel.onDoubleClicked = [this] (int row) {
        if (!engine.getTracks().empty())
        {
            if (row == 0) engine.updateTrackWarpMode (0, WarpMode::DopplerDelay);
            else if (row == 1) engine.updateTrackWarpMode (0, WarpMode::Varispeed);
        }
    };

    timeModulatorsModel.onDoubleClicked = [this] (int row) {
        if (!engine.getTracks().empty())
        {
            if (row == 0) engine.getTracksMutable()[0].gammaLfo.setWaveform (LfoWaveform::BlackHoleExp);
            else if (row == 1) engine.getTracksMutable()[0].gammaLfo.setWaveform (LfoWaveform::TachyonPulse);
            else if (row == 2) engine.getTracksMutable()[0].gammaLfo.setWaveform (LfoWaveform::LorenzChaos);
        }
    };

    // Add Category Tabs
    addAndMakeVisible (tabComponent);
    tabComponent.addTab ("MIDI FX", juce::Colour (0xff171d2b), &midiFxList, false);
    tabComponent.addTab ("SYNTHS", juce::Colour (0xff171d2b), &instrumentsList, false);
    tabComponent.addTab ("AUDIO FX", juce::Colour (0xff171d2b), &audioFxList, false);
    tabComponent.addTab ("TIME MOD", juce::Colour (0xff171d2b), &timeModulatorsList, false);
    tabComponent.addTab ("SAMPLES", juce::Colour (0xff171d2b), fileTree.get(), false);
}

void DeviceBrowserComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff10141d));
    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("BROWSER / LIBRARY", 10, 6, 200, 20, juce::Justification::left);
}

void DeviceBrowserComponent::resized()
{
    tabComponent.setBounds (5, 28, getWidth() - 10, getHeight() - 34);
}

} // namespace time_dilation
