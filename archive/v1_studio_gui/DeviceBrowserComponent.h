#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

class DeviceBrowserComponent : public juce::Component
{
public:
    explicit DeviceBrowserComponent (TimeDilationEngine& engine);
    ~DeviceBrowserComponent() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    TimeDilationEngine& engine;

    juce::TabbedComponent tabComponent { juce::TabbedButtonBar::TabsAtTop };

    // Device Category Panels
    juce::ListBox midiFxList;
    juce::ListBox instrumentsList;
    juce::ListBox audioFxList;
    juce::ListBox timeModulatorsList;

    // File Browser for Sound Samples
    juce::TimeSliceThread thread { "SampleBrowserThread" };
    juce::DirectoryContentsList directoryList { nullptr, thread };
    std::unique_ptr<juce::FileTreeComponent> fileTree;

    juce::StringArray midiFxNames { "Arpeggiator", "Chord Generator", "Scale Quantizer", "Velocity Scaler", "Transposer" };
    juce::StringArray instrumentNames { "PolySynth FM / Subtractive", "Granular Sampler Synth", "Wavetable Synth", "Relativistic Percussion Synth" };
    juce::StringArray audioFxNames { "Relativistic Doppler Delay", "Cubic Hermite Resampler", "Black Hole Reverb", "Gravitational Filter", "Time-Warp Chorus" };
    juce::StringArray timeModulatorNames { "BlackHoleExp LFO", "TachyonPulse LFO", "LorenzChaos Modulator", "Sine Gamma Modulator", "Sidechain Amplitude Warp", "Custom GammaScript" };

    struct DeviceListBoxModel : public juce::ListBoxModel
    {
        juce::StringArray items;
        std::function<void(int)> onDoubleClicked;

        int getNumRows() override { return items.size(); }
        void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
        {
            if (rowIsSelected) g.fillAll (juce::Colour (0xff263147));
            else g.fillAll (juce::Colour (0xff10141d));

            g.setColour (rowIsSelected ? juce::Colour (0xfff59e0b) : juce::Colour (0xffcbd5e1));
            g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
            g.drawText (items[rowNumber], 8, 0, width - 16, height, juce::Justification::left);
        }
        void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override
        {
            if (onDoubleClicked) onDoubleClicked (row);
        }
    };

    DeviceListBoxModel midiFxModel;
    DeviceListBoxModel instrumentsModel;
    DeviceListBoxModel audioFxModel;
    DeviceListBoxModel timeModulatorsModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviceBrowserComponent)
};

} // namespace time_dilation
