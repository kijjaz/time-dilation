#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <functional>
#include "dsp/RelativisticNodeGraph.h"
#include "dsp/RelativisticSequencers.h"

namespace time_dilation
{

static void runTerminalExampleTests()
{
    std::cout << "\n=======================================================\n";
    std::cout << " TIME DILATION DAW - HEADLESS CLI WAVEFORM VALIDATOR \n";
    std::cout << "=======================================================\n\n";

    struct PatchTest
    {
        std::string name;
        std::function<void(RelativisticNodeGraph&)> loader;
    };

    std::vector<PatchTest> tests = {
        { "01. Time Warp ([time.warp~])",       [] (RelativisticNodeGraph& g) { g.loadTimeWarpExamplePatch(); } },
        { "02. Time Retrograde ([time.retro~])", [] (RelativisticNodeGraph& g) { g.loadTimeRetroExamplePatch(); } },
        { "03. Time Stasis ([time.stasis~])",   [] (RelativisticNodeGraph& g) { g.loadTimeStasisExamplePatch(); } },
        { "04. Singularity ([time.singularity~])",[] (RelativisticNodeGraph& g) { g.loadTimeSingularityExamplePatch(); } },
        { "05. Time Quantize ([time.quantize~])",[] (RelativisticNodeGraph& g) { g.loadTimeQuantizeExamplePatch(); } },
        { "06. Master Transport ([time.transport])", [] (RelativisticNodeGraph& g) { g.loadTimeTransportExamplePatch(); } },
        { "07. Wavetable ([table])",              [] (RelativisticNodeGraph& g) { g.loadTableExamplePatch(); } },
        { "08. Tidal Mini-Notation ([tidal])",    [] (RelativisticNodeGraph& g) {
            g.clearGraph();
            int nTidal = g.addNode ("tidal", 100, 100);
            int nOsc   = g.addNode ("osc~",  420, 100);
            int nFilt  = g.addNode ("filter~", 420, 240);
            int nGain  = g.addNode ("gain~",  420, 380);
            int nOut   = g.addNode ("out~",  420, 520);

            auto nodeTidal = std::dynamic_pointer_cast<TidalPatternSequencerNode> (g.getNodeById (nTidal));
            if (nodeTidal) nodeTidal->setPatternString ("scale 'minor' '0 [3 5] 7 [10 12]*1.5 ~ [7 5]*0.8'");

            auto nodeOsc = g.getNodeById (nOsc);
            if (nodeOsc) nodeOsc->setParameter ("gain", 0.35f);

            auto nodeFilt = g.getNodeById (nFilt);
            if (nodeFilt) nodeFilt->setParameter ("cutoff", 2400.0f);

            auto nodeGain = g.getNodeById (nGain);
            if (nodeGain) nodeGain->setParameter ("gain", 0.45f);

            auto nodeOut = g.getNodeById (nOut);
            if (nodeOut) nodeOut->setParameter ("volume", 0.6f);

            g.addConnection (nTidal, 0, nOsc, 1);   // pitch -> osc freq
            g.addConnection (nTidal, 6, nOsc, 0);   // timeOut -> osc timeIn
            g.addConnection (nOsc, 0, nFilt, 1);    // osc out~ -> filter in~
            g.addConnection (nFilt, 0, nGain, 1);   // filter out~ -> gain in~
            g.addConnection (nGain, 0, nOut, 1);    // gain out~ -> out inL~
            g.addConnection (nGain, 0, nOut, 2);    // gain out~ -> out inR~
        } },
        { "09. Rhythmic Time Warping ([drumseq] Dilation)", [] (RelativisticNodeGraph& g) { g.loadRhythmicTimeWarpingExamplePatch(); } },
        { "10. Sound Pitch Warping ([fbdrum~] Doppler Pitch)", [] (RelativisticNodeGraph& g) { g.loadSoundPitchWarpingExamplePatch(); } }
    };

    double sampleRate = 44100.0;
    int blockSize = 512;
    int totalBlocks = (int)((sampleRate * 2.0) / blockSize); // 2.0 seconds audio

    for (const auto& test : tests)
    {
        RelativisticNodeGraph graph;
        graph.prepare (sampleRate, blockSize);
        graph.setAudioEngineEnabled (true);
        test.loader (graph);

        juce::AudioBuffer<float> blockBuffer (2, blockSize);
        juce::AudioBuffer<float> renderBuffer (2, (int)(sampleRate * 2.0));
        renderBuffer.clear();

        double totalEnergy = 0.0;
        float peakAmp = 0.0f;
        int nonZeroSamples = 0;

        for (int b = 0; b < totalBlocks; ++b)
        {
            blockBuffer.clear();
            graph.process (blockBuffer, blockSize);

            int startSample = b * blockSize;
            for (int ch = 0; ch < 2; ++ch)
            {
                renderBuffer.copyFrom (ch, startSample, blockBuffer, ch, 0, blockSize);
            }
        }

        // Export WAV file for terminal & visual plot inspection
        juce::String safeFileName = juce::String (test.name).retainCharacters ("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_").toLowerCase();
        juce::File scratchDir ("/Users/kijjaz/.gemini/antigravity-ide/brain/ea63aba6-26f6-42de-ab6d-10356b670a9d/scratch");
        scratchDir.createDirectory();
        juce::File wavFile = scratchDir.getChildFile ("waveform_" + safeFileName + ".wav");

        juce::WavAudioFormat wavFormat;
        if (auto fileStream = std::unique_ptr<juce::FileOutputStream> (wavFile.createOutputStream()))
        {
            if (auto writer = std::unique_ptr<juce::AudioFormatWriter> (wavFormat.createWriterFor (fileStream.get(), sampleRate, 2, 16, {}, 0)))
            {
                fileStream.release();
                writer->writeFromAudioSampleBuffer (renderBuffer, 0, renderBuffer.getNumSamples());
            }
        }

        const float* samplesL = renderBuffer.getReadPointer (0);
        int totalSamples = renderBuffer.getNumSamples();

        for (int s = 0; s < totalSamples; ++s)
        {
            float mag = std::abs (samplesL[s]);
            totalEnergy += mag * mag;
            if (mag > peakAmp) peakAmp = mag;
            if (mag > 0.00001f) nonZeroSamples++;
        }

        double rms = std::sqrt (totalEnergy / totalSamples);
        double rmsDb = (rms > 0.00001) ? 20.0 * std::log10 (rms) : -100.0;
        double peakDb = (peakAmp > 0.00001f) ? 20.0 * std::log10 (peakAmp) : -100.0;

        // Print ASCII Waveform Bar
        std::string asciiWave = "";
        int waveWidth = 32;
        int numPeaks = (int)(rms * waveWidth * 4.0);
        numPeaks = std::clamp (numPeaks, 0, waveWidth);

        for (int w = 0; w < waveWidth; ++w)
        {
            if (w < numPeaks) asciiWave += "=";
            else asciiWave += ".";
        }

        std::cout << "PATCH: " << test.name << "\n";
        std::cout << "   Waveform: [" << asciiWave << "]\n";
        std::cout << "   RMS: " << std::fixed << std::setprecision(1) << rmsDb << " dBFS | Peak: " << peakDb << " dBFS | Active Samples: " << nonZeroSamples << "/" << totalSamples << "\n";
        std::cout << "   Status: " << (nonZeroSamples > 100 ? "PASS (SOUND DETECTED)" : "SILENT") << "\n";

        // Diagnostic breakdown of nodes in graph
        for (const auto& node : graph.getNodes())
        {
            float inAudioMag = 0.0f, outAudioMag = 0.0f;
            const auto& ins = node->getInlets();
            const auto& outs = node->getOutlets();

            if (ins.size() > 1 && ins[1].type == NodePortType::Audio)
                inAudioMag = ins[1].audioData.getMagnitude (0, blockSize);
            else if (!ins.empty() && ins[0].type == NodePortType::Audio)
                inAudioMag = ins[0].audioData.getMagnitude (0, blockSize);

            if (!outs.empty() && outs[0].type == NodePortType::Audio)
                outAudioMag = outs[0].audioData.getMagnitude (0, blockSize);

            std::cout << "     Node [" << node->getTypeName() << "] id=" << node->getId() << " -> inAudioMag=" << inAudioMag << ", outAudioMag=" << outAudioMag << ", gamma=" << node->getEffectiveGamma() << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "=======================================================\n";
    std::cout << " TEST SUITE COMPLETE \n";
    std::cout << "=======================================================\n\n";
}

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

    void initialise (const juce::String& commandLine) override
    {
        if (commandLine.contains ("--test") || commandLine.contains ("--headless"))
        {
            runTerminalExampleTests();
            juce::JUCEApplication::quit();
            return;
        }

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
            auto appIconImage = juce::ImageFileFormat::loadFrom (BinaryData::logo_icon_svg_png, BinaryData::logo_icon_svg_pngSize);
            if (appIconImage.isValid())
            {
                setIcon (appIconImage);
            }
            processor = std::make_unique<TimeDilationAudioProcessor>();

            if (auto* app = TimeDilationApplication::getInstance())
            {
                processorPlayer.setProcessor (processor.get());
                app->getAudioDeviceManager().addAudioCallback (&processorPlayer);
                app->getAudioDeviceManager().addMidiInputDeviceCallback ({}, &processorPlayer);
            }

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

        ~MainWindow() override
        {
            if (auto* app = TimeDilationApplication::getInstance())
            {
                app->getAudioDeviceManager().removeAudioCallback (&processorPlayer);
                app->getAudioDeviceManager().removeMidiInputDeviceCallback ({}, &processorPlayer);
            }
            processorPlayer.setProcessor (nullptr);
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
        juce::AudioProcessorPlayer processorPlayer;
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
