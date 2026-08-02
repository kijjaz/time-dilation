#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace time_dilation
{

class TimeDilationApplication : public juce::JUCEApplication
{
public:
    TimeDilationApplication() {}

    const juce::String getApplicationName() override { return "Time Dilation DAW"; }
    const juce::String getApplicationVersion() override { return "0.0.1"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    static TimeDilationApplication* getInstance()
    {
        return dynamic_cast<TimeDilationApplication*> (juce::JUCEApplication::getInstance());
    }

    juce::AudioDeviceManager& getAudioDeviceManager() { return audioDeviceManager; }

    void initialise (const juce::String& /*commandLine*/) override
    {
        audioDeviceManager.initialiseWithDefaultDevices (2, 2);
        createNewWindow ("Time Dilation DAW");
    }

    void shutdown() override
    {
        windows.clear();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        if (commandLine.contains ("--audio-setup"))
        {
            showAudioDeviceSetupDialog();
        }
        else
        {
            createNewWindow ("Time Dilation DAW");
        }
    }

    static void showAudioDeviceSetupDialog()
    {
        if (auto* app = getInstance())
        {
            auto selector = std::make_unique<juce::AudioDeviceSelectorComponent> (
                app->audioDeviceManager,
                0, 2,  // min/max input channels
                2, 2,  // min/max output channels
                true,  // show MIDI input options
                true,  // show MIDI output options
                true,  // show channels as stereo pairs
                false  // hide advanced options
            );
            selector->setSize (520, 460);

            juce::DialogWindow::LaunchOptions opt;
            opt.dialogTitle = "AUDIO INTERFACE & HARDWARE SETUP";
            opt.content.setOwned (selector.release());
            opt.componentToCentreAround = nullptr;
            opt.dialogBackgroundColour = juce::Colour (0xff0d1322);
            opt.escapeKeyTriggersCloseButton = true;
            opt.useNativeTitleBar = true;
            opt.resizable = false;
            opt.launchAsync();
        }
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name, int examplePatchId = 0)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                  .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            processor = std::make_unique<TimeDilationAudioProcessor>();
            auto* editor = processor->createEditor();
            setContentOwned (editor, true);

            setResizable (true, true);
            setResizeLimits (800, 500, 10000, 10000);
            centreWithSize (960, 640);

            if (examplePatchId > 0)
            {
                if (auto* studioEd = dynamic_cast<TimeDilationAudioProcessorEditor*> (editor))
                {
                    if (auto* canvas = studioEd->getCanvasComponent())
                    {
                        if (examplePatchId == 10) canvas->getNodeGraph().loadTimeWarpExamplePatch();
                        else if (examplePatchId == 11) canvas->getNodeGraph().loadTimeRetroExamplePatch();
                        else if (examplePatchId == 12) canvas->getNodeGraph().loadTimeStasisExamplePatch();
                        else if (examplePatchId == 13) canvas->getNodeGraph().loadTimeSingularityExamplePatch();
                        else if (examplePatchId == 14) canvas->getNodeGraph().loadTimeQuantizeExamplePatch();
                        else if (examplePatchId == 15) canvas->getNodeGraph().loadTimeTransportExamplePatch();
                        canvas->repaint();
                    }
                }
            }

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            if (auto* app = TimeDilationApplication::getInstance())
            {
                app->closeWindow (this);
            }
        }

    private:
        std::unique_ptr<TimeDilationAudioProcessor> processor;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    MainWindow* createNewWindow (const juce::String& title, int examplePatchId = 0)
    {
        auto* w = new MainWindow (title, examplePatchId);
        int offset = (windows.size() % 8) * 30;
        w->setTopLeftPosition (w->getX() + offset, w->getY() + offset);
        windows.add (w);
        return w;
    }

    void closeWindow (MainWindow* targetWindow)
    {
        windows.removeObject (targetWindow, true);
        if (windows.isEmpty())
        {
            quit();
        }
    }

private:
    juce::AudioDeviceManager audioDeviceManager;
    juce::OwnedArray<MainWindow> windows;
};

} // namespace time_dilation

START_JUCE_APPLICATION (time_dilation::TimeDilationApplication)
