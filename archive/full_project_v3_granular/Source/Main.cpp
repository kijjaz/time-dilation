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
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise (const juce::String& /*commandLine*/) override
    {
        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                  .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            processor = std::make_unique<TimeDilationAudioProcessor>();
            setContentOwned (processor->createEditor(), true);

            setResizable (true, true);
            setResizeLimits (800, 500, 1920, 1080);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        std::unique_ptr<TimeDilationAudioProcessor> processor;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

} // namespace time_dilation

START_JUCE_APPLICATION (time_dilation::TimeDilationApplication)
