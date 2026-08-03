#include "RelativisticCanvasComponent.h"
#include "RelativisticNodeObjects.h"
#include "RelativisticSequencers.h"
#include "RelativisticTimeline.h"
#include "TimelineEditorComponent.h"
#include "ProjectFileManager.h"
#include "FontManager.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <BinaryData.h>


namespace time_dilation
{

RelativisticLookAndFeel::RelativisticLookAndFeel()
{
    juce::Typeface::Ptr sciFiTypeface = juce::Typeface::createSystemTypefaceFor (BinaryData::SmoochSans_ttf, BinaryData::SmoochSans_ttfSize);
    if (sciFiTypeface != nullptr)
    {
        setDefaultSansSerifTypeface (sciFiTypeface);
    }

    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff141c2e));
    setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff1e293b));
    setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff8fafc));
    setColour (juce::TextButton::textColourOnId, juce::Colour (0xfff59e0b));

    setColour (juce::Slider::thumbColourId, juce::Colour (0xfff59e0b));
    setColour (juce::Slider::trackColourId, juce::Colour (0xff06b6d4));
    setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0f172a));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff334155));
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff0a0f1d));
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xfff8fafc));

    setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0a0f1d));
    setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff334155));
    setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour (0xff06b6d4));
    setColour (juce::TextEditor::textColourId, juce::Colour (0xfff8fafc));

    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff0f172a));
    setColour (juce::PopupMenu::headerTextColourId, juce::Colour (0xfff59e0b));
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xfff8fafc));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff1e293b));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (0xff38bdf8));
}

void RelativisticLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                                     bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    juce::Colour bg = backgroundColour;

    if (shouldDrawButtonAsDown)
        bg = bg.brighter (0.2f);
    else if (shouldDrawButtonAsHighlighted)
        bg = bg.brighter (0.1f);

    g.setColour (bg);
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (juce::Colour (0xff334155));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void RelativisticLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                                 float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                                 const juce::Slider::SliderStyle /*style*/, juce::Slider& slider)
{
    float trackH = 6.0f;
    float trackY = y + (height - trackH) * 0.5f;

    // Track Background Trough
    g.setColour (juce::Colour (0xff090d16));
    g.fillRoundedRectangle (static_cast<float>(x), trackY, static_cast<float>(width), trackH, 3.0f);

    // Active Filled Progress Bar
    float fillW = std::clamp (sliderPos - x, 0.0f, static_cast<float>(width));
    g.setColour (juce::Colour (0xff06b6d4)); // Cyber Cyan Active Track
    g.fillRoundedRectangle (static_cast<float>(x), trackY, fillW, trackH, 3.0f);

    // 2026 Sleek Glassmorphic Pill Thumb Cap
    float thumbW = 10.0f;
    float thumbH = 18.0f;
    float thumbX = sliderPos - thumbW * 0.5f;
    float thumbY = y + (height - thumbH) * 0.5f;

    g.setColour (juce::Colour (0xfff59e0b)); // Relativistic Gold Thumb
    g.fillRoundedRectangle (thumbX, thumbY, thumbW, thumbH, 4.0f);
    g.setColour (juce::Colours::white);
    g.drawRoundedRectangle (thumbX, thumbY, thumbW, thumbH, 4.0f, 1.0f);
}

RelativisticCanvasComponent::RelativisticCanvasComponent (RelativisticNodeGraph& g)
    : nodeGraph (g)
{
    setLookAndFeel (&customLookAndFeel);
    setWantsKeyboardFocus (true);
    nodeGraph.createDefaultPatch();
    nodeGraph.pushUndoState();

    // Top Menu Bar Buttons
    addAndMakeVisible (btnMenuFile);
    btnMenuFile.onClick = [this] { showMenuFile(); };

    addAndMakeVisible (btnMenuEdit);
    btnMenuEdit.onClick = [this] { showMenuEdit(); };

    addAndMakeVisible (btnMenuView);
    btnMenuView.onClick = [this] { showMenuView(); };

    addAndMakeVisible (btnMenuObjects);
    btnMenuObjects.setButtonText ("Objects");
    btnMenuObjects.onClick = [this] { showMenuObjects(); };

    addAndMakeVisible (btnMenuAudio);
    btnMenuAudio.onClick = [this] { showMenuAudio(); };

    addAndMakeVisible (btnMenuHelp);
    btnMenuHelp.onClick = [this] { showMenuHelp(); };

    addAndMakeVisible (btnToggleMode);
    btnToggleMode.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f766e));
    btnToggleMode.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff38bdf8));
    btnToggleMode.onClick = [this] {
        isEditMode = !isEditMode;
        btnToggleMode.setButtonText (isEditMode ? "MODE: EDIT (Cmd+E)" : "MODE: PLAY (Cmd+E)");
        btnToggleMode.setColour (juce::TextButton::buttonColourId, isEditMode ? juce::Colour (0xff0f766e) : juce::Colour (0xff1e1b4b));
        btnToggleMode.setColour (juce::TextButton::textColourOffId, isEditMode ? juce::Colour (0xff38bdf8) : juce::Colour (0xffeab308));
        selectedNodeIds.clear();
        rebuildInspector();
        repaint();
    };

    addAndMakeVisible (btnUndo);
    btnUndo.setButtonText ("UNDO");
    btnUndo.onClick = [this] {
        if (nodeGraph.undo()) repaint();
    };

    addAndMakeVisible (btnRedo);
    btnRedo.setButtonText ("REDO");
    btnRedo.onClick = [this] {
        if (nodeGraph.redo()) repaint();
    };

    addAndMakeVisible (btnDuplicate);
    btnDuplicate.setButtonText ("DUP (Cmd-D)");
    btnDuplicate.onClick = [this] {
        if (!selectedNodeIds.empty())
        {
            std::vector<int> sel (selectedNodeIds.begin(), selectedNodeIds.end());
            auto newIds = nodeGraph.duplicateNodes (sel);
            selectedNodeIds.clear();
            selectedNodeIds.insert (newIds.begin(), newIds.end());
            repaint();
        }
    };

    addAndMakeVisible (btnCopy);
    btnCopy.setButtonText ("COPY (Cmd-C)");
    btnCopy.onClick = [this] {
        if (!selectedNodeIds.empty())
        {
            std::vector<int> sel (selectedNodeIds.begin(), selectedNodeIds.end());
            clipboardTree = nodeGraph.copyNodes (sel);
        }
    };

    addAndMakeVisible (btnPaste);
    btnPaste.setButtonText ("PASTE (Cmd-V)");
    btnPaste.onClick = [this] {
        if (clipboardTree.isValid())
        {
            auto newIds = nodeGraph.pasteNodes (clipboardTree);
            selectedNodeIds.clear();
            selectedNodeIds.insert (newIds.begin(), newIds.end());
            repaint();
        }
    };

    addAndMakeVisible (btnRemoveCable);
    btnRemoveCable.onClick = [this] {
        if (selectedConnectionId > 0)
        {
            nodeGraph.pushUndoState();
            nodeGraph.removeConnection (selectedConnectionId);
            selectedConnectionId = 0;
            repaint();
        }
        else if (!selectedNodeIds.empty())
        {
            std::vector<int> sel (selectedNodeIds.begin(), selectedNodeIds.end());
            nodeGraph.cutNodes (sel);
            selectedNodeIds.clear();
            repaint();
        }
    };

    addAndMakeVisible (btnAudioPower);
    btnAudioPower.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff334155));
    btnAudioPower.setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff43f5e));
    btnAudioPower.onClick = [this] {
        bool newState = !nodeGraph.isAudioEngineEnabled();
        nodeGraph.setAudioEngineEnabled (newState);
        btnAudioPower.setButtonText (newState ? "AUDIO: ON (ACTIVE)" : "AUDIO: OFF (SAFE)");
        btnAudioPower.setColour (juce::TextButton::buttonColourId, newState ? juce::Colour (0xff059669) : juce::Colour (0xff334155));
        btnAudioPower.setColour (juce::TextButton::textColourOffId, newState ? juce::Colour (0xff00ff66) : juce::Colour (0xfff43f5e));
        repaint();
    };

    addAndMakeVisible (btnSavePatch);
    btnSavePatch.onClick = [this] { savePatch(); };

    addAndMakeVisible (btnLoadPatch);
    btnLoadPatch.onClick = [this] { loadPatch(); };

    // Dual Inspector Tab Switches
    addAndMakeVisible (inspectorTitleLabel);
    inspectorTitleLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    inspectorTitleLabel.setColour (juce::Label::textColourId, juce::Colour (0xfff59e0b));
    inspectorTitleLabel.setJustificationType (juce::Justification::centredLeft);

    addAndMakeVisible (btnTabTopDown);
    btnTabTopDown.onClick = [this] {
        isBottomUpMode = false;
        btnTabTopDown.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffeab308));
        btnTabBottomUp.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
        rebuildInspector();
        resized();
        repaint();
    };

    addAndMakeVisible (btnTabBottomUp);
    btnTabBottomUp.onClick = [this] {
        isBottomUpMode = true;
        btnTabBottomUp.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffa855f7));
        btnTabTopDown.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
        rebuildInspector();
        resized();
        repaint();
    };

    addAndMakeVisible (btnInsertTapDropdown);
    btnInsertTapDropdown.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f766e));
    btnInsertTapDropdown.onClick = [this] {
        juce::PopupMenu m;
        m.addSectionHeader ("--- WIRELESS TAP POINTS ---");
        int itemId = 1;
        std::vector<std::string> tapSnippets;

        for (const auto& n : nodeGraph.getNodes())
        {
            std::string label = n->getLabel();
            std::string nodeTap = "tap('" + label + "')";
            m.addItem (itemId++, "[NODE]: " + label, true);
            tapSnippets.push_back (nodeTap);

            for (const auto& p : n->getParameterDefs())
            {
                std::string paramTap = "tap('" + label + "." + p.key + "')";
                m.addItem (itemId++, "   ↳ " + p.name + " (" + paramTap + ")", true);
                tapSnippets.push_back (paramTap);
            }
            m.addSeparator();
        }

        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (btnInsertTapDropdown),
            [this, tapSnippets] (int choice) {
                if (choice > 0 && choice <= static_cast<int>(tapSnippets.size()))
                {
                    std::string snippet = tapSnippets[choice - 1];
                    juce::SystemClipboard::copyTextToClipboard (snippet);
                    formulaEditor.insertTextAtCaret (" + " + snippet);
                }
            });
    };

    addAndMakeVisible (formulaEditor);
    formulaEditor.setMultiLine (true);
    formulaEditor.setReturnKeyStartsNewLine (true);
    formulaEditor.setFont (juce::FontOptions (12.0f));
    formulaEditor.setText ("// C++ / DSP Math Expression Script\n// Edit live formulas here:\n\ngamma = 1.0 + 1.5 * sin(t * 2.5);\ncutoff = 440.0 * pow(2.0, note / 12.0);");

    addAndMakeVisible (btnApplyFormula);
    btnApplyFormula.onClick = [this] {
        int primaryId = !selectedNodeIds.empty() ? *selectedNodeIds.begin() : 0;
        auto n = nodeGraph.getNodeById (primaryId);
        if (n)
        {
            n->setFormulaScript (formulaEditor.getText().toStdString());
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon, "FORMULA APPLIED", "Updated live C++ math script for node " + juce::String (primaryId));
        }
    };

    addChildComponent (inlineLabelEditor);
    inlineLabelEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff1e293b));
    inlineLabelEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xffeab308));
    inlineLabelEditor.onReturnKey = [this] {
        int primaryId = !selectedNodeIds.empty() ? *selectedNodeIds.begin() : 0;
        auto n = nodeGraph.getNodeById (primaryId);
        if (n)
        {
            n->setLabel (inlineLabelEditor.getText().toStdString());
            inlineLabelEditor.setVisible (false);
            repaint();
        }
    };
    inlineLabelEditor.onFocusLost = [this] {
        inlineLabelEditor.setVisible (false);
    };

    initObjectCatalog();

    addAndMakeVisible (btnAddObject);
    btnAddObject.onClick = [this] {
        spawnInlineObjectEditor ({ getWidth() * 0.4f, getHeight() * 0.4f });
    };

    addAndMakeVisible (btnClear);
    btnClear.onClick = [this] {
        nodeGraph.clearGraph();
        repaint();
    };

    addAndMakeVisible (btnToggleCord);
    btnToggleCord.setButtonText ("CORDS: ORGANIC");
    btnToggleCord.onClick = [this] {
        if (cableStyle == CableStyle::Organic)
        {
            cableStyle = CableStyle::SmoothS;
            btnToggleCord.setButtonText ("CORDS: SMOOTH S");
        }
        else if (cableStyle == CableStyle::SmoothS)
        {
            cableStyle = CableStyle::Straight;
            btnToggleCord.setButtonText ("CORDS: STRAIGHT");
        }
        else
        {
            cableStyle = CableStyle::Organic;
            btnToggleCord.setButtonText ("CORDS: ORGANIC");
        }
        repaint();
    };

    addAndMakeVisible (btnHorizonReadout);
    btnHorizonReadout.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0d1322));
    btnHorizonReadout.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff06b6d4));

    addAndMakeVisible (btnResetHorizon);
    btnResetHorizon.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
    btnResetHorizon.setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff59e0b));
    btnResetHorizon.onClick = [this] {
        nodeGraph.resetCausalityHorizon();
        repaint();
    };

    startTimerHz (30);
}

void RelativisticCanvasComponent::showHelpDialog (const juce::String& topic, const juce::String& content)
{
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon, topic, content);
}

void RelativisticCanvasComponent::savePatchAs()
{
    auto fc = std::make_shared<juce::FileChooser> ("Save Time Dilation Project File As...",
                                                   juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                                                   "*.tdaw;*.tdawproj;*.xml");
    fc->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
        [this, fc] (const juce::FileChooser& chooser) {
            auto result = chooser.getResult();
            if (result != juce::File())
            {
                if (result.getFileExtension().isEmpty())
                    result = result.withFileExtension ("tdaw");

                bool success = ProjectFileManager::getInstance().saveProjectBundle (result, nodeGraph);
                if (success)
                {
                    currentProjectFile = result;
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::InfoIcon,
                        "Project Saved",
                        "Successfully saved patch to:\n" + result.getFullPathName());
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::WarningIcon,
                        "Save Failed",
                        "Could not save project file to:\n" + result.getFullPathName());
                }
            }
        });
}

void RelativisticCanvasComponent::savePatch()
{
    if (currentProjectFile.existsAsFile() || currentProjectFile.isDirectory())
    {
        bool success = ProjectFileManager::getInstance().saveProjectBundle (currentProjectFile, nodeGraph);
        if (success)
        {
            juce::AlertWindow::showMessageBoxAsync (
                juce::AlertWindow::InfoIcon,
                "Project Saved",
                "Successfully saved patch to:\n" + currentProjectFile.getFullPathName());
        }
        else
        {
            savePatchAs();
        }
    }
    else
    {
        savePatchAs();
    }
}

void RelativisticCanvasComponent::loadPatch()
{
    auto fc = std::make_shared<juce::FileChooser> ("Open Time Dilation Project File...",
                                                   juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                                                   "*.tdaw;*.tdawproj;*.xml;project.xml");
    fc->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectDirectories,
        [this, fc] (const juce::FileChooser& chooser) {
            auto result = chooser.getResult();
            if (result != juce::File())
            {
                bool success = ProjectFileManager::getInstance().loadProjectBundle (result, nodeGraph);
                if (success)
                {
                    currentProjectFile = result;
                    selectedNodeIds.clear();
                    selectedConnectionId = 0;
                    rebuildInspector();
                    repaint();
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::InfoIcon,
                        "Project Loaded",
                        "Successfully loaded patch from:\n" + result.getFullPathName());
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::WarningIcon,
                        "Load Failed",
                        "Could not parse valid project file at:\n" + result.getFullPathName());
                }
            }
        });
}

void RelativisticCanvasComponent::exportAudioWav()
{
    auto fc = std::make_shared<juce::FileChooser> ("Export Rendered Audio WAV File As...",
                                                   juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                                                   "*.wav");
    fc->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
        [this, fc] (const juce::FileChooser& chooser) {
            auto result = chooser.getResult();
            if (result != juce::File())
            {
                if (result.getFileExtension().isEmpty())
                    result = result.withFileExtension ("wav");

                double renderRate = 44100.0;
                int renderSamples = static_cast<int>(renderRate * 10.0); // 10 seconds of high-fidelity offline rendering
                int blockSize = 512;

                juce::AudioBuffer<float> renderBuffer (2, renderSamples);
                renderBuffer.clear();

                nodeGraph.prepare (renderRate, blockSize);

                juce::AudioBuffer<float> blockBuffer (2, blockSize);
                int samplesRendered = 0;
                while (samplesRendered < renderSamples)
                {
                    int numToProcess = std::min (blockSize, renderSamples - samplesRendered);
                    blockBuffer.clear();
                    nodeGraph.process (blockBuffer, numToProcess);

                    renderBuffer.copyFrom (0, samplesRendered, blockBuffer, 0, 0, numToProcess);
                    renderBuffer.copyFrom (1, samplesRendered, blockBuffer, 1, 0, numToProcess);

                    samplesRendered += numToProcess;
                }

                juce::WavAudioFormat wavFormat;
                std::unique_ptr<juce::AudioFormatWriter> writer (wavFormat.createWriterFor (
                    result.createOutputStream().release(),
                    renderRate,
                    2,
                    16,
                    {},
                    0));

                if (writer != nullptr)
                {
                    writer->writeFromAudioSampleBuffer (renderBuffer, 0, renderSamples);
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::InfoIcon,
                        "Audio Export Complete",
                        "Successfully rendered 10.0 seconds of 44.1 kHz 16-bit WAV audio to:\n" + result.getFullPathName());
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::WarningIcon,
                        "Audio Export Failed",
                        "Could not create output WAV file stream at:\n" + result.getFullPathName());
                }
            }
        });
}

void RelativisticCanvasComponent::exportCppScript()
{
    auto fc = std::make_shared<juce::FileChooser> ("Export Generated C++ DSP Graph Formulas As...",
                                                   juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                                                   "*.cpp;*.h;*.txt");
    fc->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
        [this, fc] (const juce::FileChooser& chooser) {
            auto result = chooser.getResult();
            if (result != juce::File())
            {
                if (result.getFileExtension().isEmpty())
                    result = result.withFileExtension ("cpp");

                juce::String code;
                code << "// ==============================================================================\n";
                code << "// Time Dilation DAW (v4.0) — Exported C++ Relativistic Audio Graph\n";
                code << "// Generated Automatically from Active Modular Canvas Graph\n";
                code << "// ==============================================================================\n\n";
                code << "#include <JuceHeader.h>\n#include <cmath>\n#include <vector>\n#include <string>\n\n";
                code << "class ExportedRelativisticAudioGraph\n{\npublic:\n";
                code << "    ExportedRelativisticAudioGraph() {}\n\n";
                code << "    void prepare (double sampleRate, int samplesPerBlock)\n    {\n";
                code << "        fs = sampleRate;\n";
                code << "        blockSize = samplesPerBlock;\n";
                code << "        tau = 0.0;\n    }\n\n";
                code << "    void processBlock (juce::AudioBuffer<float>& buffer)\n    {\n";
                code << "        int numSamples = buffer.getNumSamples();\n";
                code << "        for (int s = 0; s < numSamples; ++s)\n        {\n";
                code << "            double dt = 1.0 / fs;\n";
                code << "            tau += currentGamma * dt; // Dynamic Coordinate Time Integration\n\n";

                code << "            // --- NODE DSP FORMULAS ---\n";
                for (const auto& node : nodeGraph.getNodes())
                {
                    code << "            // Node #" << node->getId() << " [" << juce::String (node->getTypeName()) << "] label: \"" << juce::String (node->getLabel()) << "\"\n";
                    std::string formula = node->getDefaultFormulaScript();
                    juce::StringArray lines;
                    lines.addLines (juce::String (formula));
                    for (const auto& line : lines)
                    {
                        code << "            " << line << "\n";
                    }
                    code << "\n";
                }

                code << "        }\n    }\n\nprivate:\n";
                code << "    double fs = 44100.0;\n";
                code << "    int blockSize = 512;\n";
                code << "    double tau = 0.0;\n";
                code << "    double currentGamma = 1.0;\n";
                code << "};\n";

                if (result.replaceWithText (code))
                {
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::InfoIcon,
                        "C++ DSP Export Complete",
                        "Successfully exported authentic C++ DSP graph formulas to:\n" + result.getFullPathName());
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::WarningIcon,
                        "C++ DSP Export Failed",
                        "Could not write file to:\n" + result.getFullPathName());
                }
            }
        });
}

void RelativisticCanvasComponent::showMenuFile()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- PROJECT FILE MANAGEMENT ---");
    m.addItem (1, "New Patch", true);
    m.addItem (5, "New Window (Cmd+N)", true);
    m.addItem (2, "Open Patch...", true);
    m.addItem (3, "Save Patch", true);
    m.addItem (4, "Save Patch As...", true);
    m.addSeparator();

    juce::PopupMenu subExport;
    subExport.addItem (10, "Export Audio File (WAV)...", true);
    subExport.addItem (11, "Export C++ Formula Script...", true);
    m.addSubMenu ("Export...", subExport);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuFile),
        [this] (int result) {
            if (result == 1)
            {
                nodeGraph.clearGraph();
                currentProjectFile = juce::File();
                rebuildInspector();
                repaint();
            }
            else if (result == 5)
            {
                juce::MessageManager::callAsync ([] {
                    if (auto* appInstance = juce::JUCEApplication::getInstance())
                        appInstance->anotherInstanceStarted ("");
                });
            }
            else if (result == 2)
            {
                loadPatch();
            }
            else if (result == 3)
            {
                savePatch();
            }
            else if (result == 4)
            {
                savePatchAs();
            }
            else if (result == 10)
            {
                exportAudioWav();
            }
            else if (result == 11)
            {
                exportCppScript();
            }
        });
}

void RelativisticCanvasComponent::showMenuEdit()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- EDIT ACTIONS ---");
    m.addItem (1, "Undo (Cmd+Z)", nodeGraph.canUndo());
    m.addItem (2, "Redo (Cmd+Shift+Z)", nodeGraph.canRedo());
    m.addSeparator();
    m.addItem (3, "Cut (Cmd+X)", !selectedNodeIds.empty());
    m.addItem (4, "Copy (Cmd+C)", !selectedNodeIds.empty());
    m.addItem (5, "Paste (Cmd+V)", true);
    m.addItem (6, "Duplicate (Cmd+D)", !selectedNodeIds.empty());
    m.addSeparator();
    m.addItem (7, "Select All (Cmd+A)", true);
    m.addItem (8, "Clear Patch", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuEdit),
        [this] (int result) {
            if (result == 1) nodeGraph.undo();
            else if (result == 2) nodeGraph.redo();
            else if (result == 4) btnCopy.triggerClick();
            else if (result == 5) btnPaste.triggerClick();
            else if (result == 6) btnDuplicate.triggerClick();
            else if (result == 7) {
                selectedNodeIds.clear();
                for (const auto& n : nodeGraph.getNodes()) selectedNodeIds.insert (n->getId());
                rebuildInspector();
            }
            else if (result == 8) { nodeGraph.clearGraph(); }
            repaint();
        });
}

void RelativisticCanvasComponent::showMenuView()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- CANVAS & INTERFACE VIEW ---");

    juce::PopupMenu subZoom;
    subZoom.addItem (100, "Zoom In (Cmd+=)", true);
    subZoom.addItem (101, "Zoom Out (Cmd+-)", true);
    subZoom.addItem (102, "Reset Zoom 100% (Cmd+0)", true);
    subZoom.addSeparator();
    subZoom.addItem (103, "50%", true, std::abs (zoomLevel - 0.5f) < 0.05f);
    subZoom.addItem (104, "75%", true, std::abs (zoomLevel - 0.75f) < 0.05f);
    subZoom.addItem (105, "100% (Normal)", true, std::abs (zoomLevel - 1.0f) < 0.05f);
    subZoom.addItem (106, "125%", true, std::abs (zoomLevel - 1.25f) < 0.05f);
    subZoom.addItem (107, "150%", true, std::abs (zoomLevel - 1.5f) < 0.05f);
    subZoom.addItem (108, "200%", true, std::abs (zoomLevel - 2.0f) < 0.05f);
    m.addSubMenu ("Canvas Zoom Level (" + juce::String (static_cast<int>(zoomLevel * 100.0f)) + "%)", subZoom);

    m.addSeparator();
    m.addItem (2, "Show Canvas Grid (Dot Matrix)", true, showGrid);
    m.addItem (3, "Snap to Grid", true, snapToGrid);

    juce::PopupMenu subGridSize;
    subGridSize.addItem (20, "12 px (Fine)", true, gridSize == 12.0f);
    subGridSize.addItem (21, "24 px (Standard)", true, gridSize == 24.0f);
    subGridSize.addItem (22, "48 px (Coarse)", true, gridSize == 48.0f);
    m.addSubMenu ("Grid Spacing Size", subGridSize);

    m.addSeparator();
    juce::PopupMenu subCords;
    subCords.addItem (10, "Organic Catenary Cables", true, cableStyle == CableStyle::Organic);
    subCords.addItem (11, "Smooth S-Curve Cables", true, cableStyle == CableStyle::SmoothS);
    subCords.addItem (12, "Straight Pure Data-Style", true, cableStyle == CableStyle::Straight);
    m.addSubMenu ("Patch Cord Style", subCords);

    m.addSeparator();
    m.addItem (1, "Recenter Canvas View & Reset Zoom", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuView),
        [this] (int result) {
            if (result == 1) { panX = 0.0f; panY = 0.0f; resetZoom(); }
            else if (result == 2) { showGrid = !showGrid; }
            else if (result == 3) { snapToGrid = !snapToGrid; }
            else if (result == 20) { gridSize = 12.0f; }
            else if (result == 21) { gridSize = 24.0f; }
            else if (result == 22) { gridSize = 48.0f; }
            else if (result == 100) { zoomIn(); }
            else if (result == 101) { zoomOut(); }
            else if (result == 102) { resetZoom(); }
            else if (result == 103) { setZoomLevel (0.5f); }
            else if (result == 104) { setZoomLevel (0.75f); }
            else if (result == 105) { setZoomLevel (1.0f); }
            else if (result == 106) { setZoomLevel (1.25f); }
            else if (result == 107) { setZoomLevel (1.5f); }
            else if (result == 108) { setZoomLevel (2.0f); }
            else if (result == 10) { cableStyle = CableStyle::Organic; btnToggleCord.setButtonText ("CORDS: ORGANIC"); }
            else if (result == 11) { cableStyle = CableStyle::SmoothS; btnToggleCord.setButtonText ("CORDS: SMOOTH S"); }
            else if (result == 12) { cableStyle = CableStyle::Straight; btnToggleCord.setButtonText ("CORDS: STRAIGHT"); }
            repaint();
        });
}

void RelativisticCanvasComponent::showMenuObjects()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- WORKSTATION OBJECT PALETTE ---");
    m.addItem (1, "Add New Object... (Cmd+1 / N / Double-Click)", true);

    juce::PopupMenu subMonitors;
    subMonitors.addItem (100, "[time.scope]\tRelativistic Time Monitor & Telemetry Visualizer", true);
    subMonitors.addItem (101, "[time.display]\tDigital Time & Dilation Gauge Display", true);
    subMonitors.addItem (102, "[time.monitor]\tCoordinate Time Stream Inspector", true);
    m.addSubMenu ("Time Data Monitors & Telemetry", subMonitors);

    juce::PopupMenu subControl;
    subControl.addItem (110, "[number]\tControl Number Box (Click & Drag Value)", true);
    subControl.addItem (111, "[bang]\tControl Trigger Pulse (1.0 Spike)", true);
    subControl.addItem (112, "[bang~]\tAudio Rate 1-Sample Impulse Spike", true);
    subControl.addItem (114, "[counter]\tSmart Value Counter (Low, High, Step, Carry)", true);
    subControl.addItem (113, "[table]\tInteractive Wavetable & Sample Canvas", true);
    subControl.addItem (115, "[tap]\tControl Wireless Signal Tap", true);
    subControl.addItem (116, "[tap~]\tAudio Wireless Signal Tap", true);
    m.addSubMenu ("Control & Interactors (Pd-Style)", subControl);

    juce::PopupMenu subTime;
    subTime.addItem (10, "[time.warp~]\tDilated Coordinate Clock", true);
    subTime.addItem (11, "[time.retro~]\tRetrograde Time Reverser", true);
    subTime.addItem (12, "[time.quantize~]\tMetric Grid Quantizer", true);
    subTime.addItem (13, "[time.metro~]\tRelativistic Metronome", true);
    subTime.addItem (14, "[time.stasis~]\tGravitational Time Freeze", true);
    subTime.addItem (15, "[time.singularity~]\tEvent Horizon Redshift", true);
    subTime.addItem (16, "[time.transport]\tMulti-Instance Transport", true);
    m.addSubMenu ("Relativistic Time Engines", subTime);

    juce::PopupMenu subAudio;
    subAudio.addItem (20, "[osc~]\tPolyBLEP VA Oscillator", true);
    subAudio.addItem (21, "[phasor~]\tRamp Phase Generator", true);
    subAudio.addItem (22, "[sampler~]\tAudio Buffer Sampler", true);
    subAudio.addItem (23, "[filter~]\tState-Variable Filter", true);
    subAudio.addItem (41, "[svfilter~]\tDual SVF Filter", true);
    subAudio.addItem (42, "[drive~]\tOverdrive Distortion", true);
    subAudio.addItem (43, "[reverb~]\tAlgorithmic Reverb", true);
    subAudio.addItem (44, "[crush~]\tBitcrusher / SR Reducer", true);
    subAudio.addItem (45, "[adsr~]\tADSR Envelope Generator", true);
    subAudio.addItem (24, "[delay~]\tFeedback Delay Line", true);
    subAudio.addItem (25, "[dac~]\tMaster Audio DAC", true);
    subAudio.addItem (26, "[gain~]\tAudio Signal Scaler", true);
    subAudio.addItem (27, "[out~]\tMaster Output & Live VU Meters", true);
    subAudio.addItem (28, "[env~]\tEnvelope Follower", true);
    m.addSubMenu ("Audio Processors & Effects", subAudio);

    juce::PopupMenu subMath;
    subMath.addItem (30, "[expr]\tControl Expression ($v1, tap('prop'))", true);
    subMath.addItem (31, "[expr~]\tAudio Expression ($v1, tap('prop'))", true);
    subMath.addItem (32, "[fexpr~]\tFilter Recurrent Expression ($y1[-1])", true);
    subMath.addItem (33, "[v]\tValue Storage Control Node", true);
    subMath.addItem (34, "[z~]\t1-Sample Feedback Delay", true);
    subMath.addItem (35, "[snapshot~]\tAudio Snapshot Converter", true);
    subMath.addItem (36, "[+]\tSignal/Control Adder", true);
    subMath.addItem (37, "[*]\tSignal/Control Multiplier", true);
    subMath.addItem (38, "[mtof]\tMIDI Note -> Hz Frequency", true);
    subMath.addItem (39, "[ftom]\tHz Frequency -> MIDI Note", true);
    subMath.addItem (40, "[note]\tAlgorithmic Note Generator", true);
    m.addSubMenu ("Math & Control Nodes", subMath);

    juce::PopupMenu subTables;
    subTables.addItem (113, "[table]\tInteractive Waveform Canvas / Table", true);
    subTables.addItem (50, "[tabread~]\tTable Sample / Pitch Reader", true);
    subTables.addItem (51, "[tabwrite~]\tTable Live Audio Recorder", true);
    subTables.addItem (52, "[tabosc4~]\t4-Pt Wavetable Oscillator", true);
    subTables.addItem (22, "[sampler~]\tAudio Sampler Engine", true);
    m.addSubMenu ("Tables & Array Buffers", subTables);

    m.addSeparator();
    m.addItem (2, "Detect & Highlight Feedback Loops", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuObjects),
        [this] (int result) {
            if (result == 1) showObjectSearchMenu ({ getWidth() * 0.4f, getHeight() * 0.4f });
            else if (result == 2) {
                nodeGraph.detectFeedbackLoops();
                showHelpDialog ("Feedback Loop Audit", "Checked audio graph connections. Any feedback loop connections are highlighted in Neon Warning Red with 1-sample delay memory.");
            }
            else
            {
                std::string typeName;
                if (result == 100 || result == 101 || result == 102) typeName = "time.scope";
                else if (result == 110) typeName = "number";
                else if (result == 111) typeName = "bang";
                else if (result == 112) typeName = "bang~";
                else if (result == 113) typeName = "table";
                else if (result == 114) typeName = "counter";
                else if (result == 115) typeName = "tap";
                else if (result == 116) typeName = "tap~";
                else if (result == 10) typeName = "time.warp~";
                else if (result == 11) typeName = "time.retro~";
                else if (result == 12) typeName = "time.quantize~";
                else if (result == 13) typeName = "time.metro~";
                else if (result == 14) typeName = "time.stasis~";
                else if (result == 15) typeName = "time.singularity~";
                else if (result == 16) typeName = "time.transport";
                else if (result == 20) typeName = "osc~";
                else if (result == 21) typeName = "phasor~";
                else if (result == 22) typeName = "sampler~";
                else if (result == 23) typeName = "filter~";
                else if (result == 24) typeName = "delay~";
                else if (result == 25) typeName = "dac~";
                else if (result == 26) typeName = "gain~";
                else if (result == 27) typeName = "out~";
                else if (result == 28) typeName = "env~";
                else if (result == 30) typeName = "expr";
                else if (result == 31) typeName = "expr~";
                else if (result == 32) typeName = "fexpr~";
                else if (result == 33) typeName = "v";
                else if (result == 34) typeName = "z~";
                else if (result == 35) typeName = "snapshot~";
                else if (result == 36) typeName = "+";
                else if (result == 37) typeName = "*";
                else if (result == 38) typeName = "mtof";
                else if (result == 39) typeName = "ftom";
                else if (result == 40) typeName = "note";
                else if (result == 41) typeName = "svfilter~";
                else if (result == 42) typeName = "drive~";
                else if (result == 43) typeName = "reverb~";
                else if (result == 44) typeName = "crush~";
                else if (result == 45) typeName = "adsr~";
                else if (result == 50) typeName = "tabread~";
                else if (result == 51) typeName = "tabwrite~";
                else if (result == 52) typeName = "tabosc4~";

                if (!typeName.empty())
                {
                    nodeGraph.pushUndoState();
                    int id = nodeGraph.addNode (typeName, getWidth() * 0.4f, getHeight() * 0.4f);
                    selectedNodeIds.clear();
                    selectedNodeIds.insert (id);
                    rebuildInspector();
                    repaint();
                }
            }
        });
}

void RelativisticCanvasComponent::showMenuAudio()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- AUDIO ENGINE & HARDWARE ---");
    m.addItem (1, "Audio Engine Power Switch", true, nodeGraph.isAudioEngineEnabled());
    m.addItem (2, "Audio Interface Setup... (Inputs, Outputs & Devices)", true);
    m.addSeparator();

    juce::PopupMenu subSampleRate;
    juce::PopupMenu subBuffer;

    juce::AudioIODevice* currentDevice = nullptr;
    if (auto* app = juce::JUCEApplication::getInstance())
    {
        // Try getting active device via message manager
    }

    // Populate Hardware / Calculation Sample Rates
    subSampleRate.addItem (10, "44.1 kHz (44100 Hz)", true);
    subSampleRate.addItem (11, "48.0 kHz (48000 Hz)", true);
    subSampleRate.addItem (12, "88.2 kHz (88200 Hz)", true);
    subSampleRate.addItem (13, "96.0 kHz (96000 Hz)", true);
    subSampleRate.addItem (14, "176.4 kHz (176400 Hz)", true);
    subSampleRate.addItem (15, "192.0 kHz (192000 Hz)", true);
    subSampleRate.addSeparator();
    subSampleRate.addItem (199, "Custom / Offline Calculation Rate...", true);
    m.addSubMenu ("Sample Rate", subSampleRate);

    // Populate Hardware / Calculation Buffer Sizes
    subBuffer.addItem (20, "32 Samples (Extreme Low Latency)", true);
    subBuffer.addItem (21, "64 Samples (Ultra Low Latency)", true);
    subBuffer.addItem (22, "128 Samples (Low Latency)", true);
    subBuffer.addItem (23, "256 Samples (Pro Studio Latency)", true);
    subBuffer.addItem (24, "512 Samples (Standard)", true);
    subBuffer.addItem (25, "1024 Samples (Safe Buffer)", true);
    subBuffer.addItem (26, "2048 Samples (High Reliability)", true);
    subBuffer.addItem (27, "4096 Samples (Maximum Stability)", true);
    subBuffer.addSeparator();
    subBuffer.addItem (299, "Custom Buffer Size...", true);
    m.addSubMenu ("Buffer Size", subBuffer);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuAudio),
        [this] (int result) {
            if (result == 1) btnAudioPower.triggerClick();
            else if (result == 2)
            {
                juce::MessageManager::callAsync ([] {
                    juce::JUCEApplication::getInstance()->anotherInstanceStarted ("--audio-setup");
                });
            }
            else if (result >= 10 && result <= 15)
            {
                double rates[6] = { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };
                double sr = rates[result - 10];
                nodeGraph.prepare (sr, 512);
                showHelpDialog ("Sample Rate Configured", "Audio DSP Engine configured to " + juce::String (sr) + " Hz.");
            }
            else if (result == 199)
            {
                auto alert = std::make_unique<juce::AlertWindow> ("CUSTOM CALCULATION SAMPLE RATE", "Enter sample rate in Hz for offline math/DSP calculation (e.g. 22050, 32000, 88200, 192000, 384000):", juce::AlertWindow::QuestionIcon);
                alert->addTextEditor ("srInput", "44100", "Sample Rate (Hz):");
                alert->addButton ("Apply", 1);
                alert->addButton ("Cancel", 0);
                alert->enterModalState (true, juce::ModalCallbackFunction::create ([this, a = alert.get()] (int res) {
                    if (res == 1)
                    {
                        double customSr = a->getTextEditorContents ("srInput").getDoubleValue();
                        if (customSr > 1000.0 && customSr < 768000.0)
                        {
                            nodeGraph.prepare (customSr, 512);
                            showHelpDialog ("Custom Calculation Rate Applied", "DSP calculation engine set to " + juce::String (customSr) + " Hz.");
                        }
                    }
                }), true);
                alert.release();
            }
            else if (result >= 20 && result <= 27)
            {
                int sizes[8] = { 32, 64, 128, 256, 512, 1024, 2048, 4096 };
                int bs = sizes[result - 20];
                nodeGraph.prepare (44100.0, bs);
                showHelpDialog ("Buffer Size Configured", "Audio DSP Engine block size set to " + juce::String (bs) + " samples.");
            }
            else if (result == 299)
            {
                auto alert = std::make_unique<juce::AlertWindow> ("CUSTOM BUFFER SIZE", "Enter custom buffer size in samples (e.g. 32, 64, 128, 256, 512, 1024, 2048):", juce::AlertWindow::QuestionIcon);
                alert->addTextEditor ("bsInput", "256", "Buffer Size (Samples):");
                alert->addButton ("Apply", 1);
                alert->addButton ("Cancel", 0);
                alert->enterModalState (true, juce::ModalCallbackFunction::create ([this, a = alert.get()] (int res) {
                    if (res == 1)
                    {
                        int customBs = a->getTextEditorContents ("bsInput").getIntValue();
                        if (customBs >= 16 && customBs <= 65536)
                        {
                            nodeGraph.prepare (44100.0, customBs);
                            showHelpDialog ("Custom Buffer Size Applied", "DSP engine block size set to " + juce::String (customBs) + " samples.");
                        }
                    }
                }), true);
                alert.release();
            }
        });
}

void RelativisticCanvasComponent::showMenuHelp()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- INTERACTIVE EXAMPLE PATCHES ---");
    m.addItem (10, "[time.warp~] Continuous Warp Speed Oscillator", true);
    m.addItem (11, "[time.retro~] Retrograde Reverse Playback", true);
    m.addItem (12, "[time.stasis~] Event Horizon Stasis Freeze", true);
    m.addItem (13, "[time.singularity~] Black Hole Gravitational Warping", true);
    m.addItem (14, "[time.quantize~] Relativistic Stutter Grid", true);
    m.addItem (15, "[time.transport] Multi-Clock Transport Sync", true);
    m.addItem (16, "[table] Interactive Waveform Drawing & Wavetable Synth", true);
    m.addItem (17, "[drumseq] Rhythmic Time Warping (Steady Pitch Drums)", true);
    m.addItem (18, "[fbdrum~] Sound Pitch Warping (Doppler Pitch Shift Drums)", true);

    m.addSeparator();
    m.addSectionHeader ("--- USER HELP & WORKSTATION MANUAL ---");
    m.addItem (1, "Quick Start Guide", true);
    m.addItem (2, "Relativistic Time Dilation Architecture Manual", true);
    m.addItem (8, "Tidal Live-Coding Suite Manual ([tidal])", true);
    m.addItem (9, "Future Lookahead Causality Engine ([time.future~])", true);
    m.addItem (3, "Wireless Signal Tapping Syntax (tap())", true);
    m.addItem (4, "Pure Data Expression Scripting & C++ Math", true);
    m.addItem (5, "1-Sample Feedback Loop Protection", true);
    m.addItem (6, "Keyboard Shortcuts & Hotkeys", true);
    m.addSeparator();
    m.addItem (7, "About Time Dilation DAW (v4.0)...", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuHelp),
        [this] (int result) {
            if (result == 10) { nodeGraph.loadTimeWarpExamplePatch(); repaint(); }
            else if (result == 11) { nodeGraph.loadTimeRetroExamplePatch(); repaint(); }
            else if (result == 12) { nodeGraph.loadTimeStasisExamplePatch(); repaint(); }
            else if (result == 13) { nodeGraph.loadTimeSingularityExamplePatch(); repaint(); }
            else if (result == 14) { nodeGraph.loadTimeQuantizeExamplePatch(); repaint(); }
            else if (result == 15) { nodeGraph.loadTimeTransportExamplePatch(); repaint(); }
            else if (result == 16) { nodeGraph.loadTableExamplePatch(); repaint(); }
            else if (result == 17) { nodeGraph.loadRhythmicTimeWarpingExamplePatch(); repaint(); }
            else if (result == 18) { nodeGraph.loadSoundPitchWarpingExamplePatch(); repaint(); }
            else if (result == 1)
            {
                showHelpDialog ("Quick Start Guide",
                    "Welcome to Time Dilation DAW (v4.0)!\n\n"
                    "1. ADD OBJECTS: Double-click empty canvas or press 'N' or click '+ ADD OBJECT (N)'.\n"
                    "2. CONNECT PORTS: Drag a cable from an outlet to an inlet.\n"
                    "   - Purple Cable: Time Dilation Context (gamma, t_local)\n"
                    "   - Cyan Cable: Audio Signal Stream (DSP)\n"
                    "   - Amber Cable: Control Value / Modulation\n"
                    "3. DUAL INSPECTOR: Click any object on canvas to view and edit its parameter sliders (TOP-DOWN) or its authentic C++ DSP math script (BOTTOM-UP).\n"
                    "4. AUDIO POWER: Click 'AUDIO: OFF (SAFE)' in the top right to start audio processing.");
            }
            else if (result == 2)
            {
                showHelpDialog ("Relativistic Time Dilation Architecture Manual",
                    "RELATIVISTIC TIME ARCHITECTURE MANUAL\n\n"
                    "In Time Dilation DAW, every node on the canvas operates on its own local coordinate time (t_local).\n\n"
                    "- time.warp~: Generates dilated coordinate time clocks (gamma = 0.1x to 10.0x).\n"
                    "- time.retro~: Reverses time progression (gamma = -1.0x).\n"
                    "- time.quantize~: Quantizes time into metric grid beats.\n"
                    "- time.metro~: Relativistic metronome pulse generator.\n"
                    "- time.stasis~: Freezes time (gamma = 0.0) while holding audio state.\n"
                    "- time.singularity~: Event horizon gravitational redshift.\n"
                    "- time.future~: Future Lookahead Causality Offset Engine.\n\n"
                    "UNIVERSAL TIME INLETS:\n"
                    "Every object has a purple time inlet (timeIn). Patching any time engine into an object's timeIn port dilates or reverses that object's clock independently!");
            }
            else if (result == 8)
            {
                showHelpDialog ("Tidal Live-Coding Suite Manual ([tidal])",
                    "TIDAL LIVE-CODING SUITE MANUAL ([tidal])\n\n"
                    "The [tidal] node parses TidalCycles-style mini-notation patterns with live scale mapping and per-step relativistic time dilation!\n\n"
                    "MINI-NOTATION SYNTAX:\n"
                    "- Subdivisions: \"60 [62 64] 65 [67 69 71]\" (Subdivides beats into half-notes or triplets)\n"
                    "- Scale Degrees: \"scale 'minor' '0 [2 3] 4 [5 7]'\" (Maps indices to minor/major/pentatonic/dorian)\n"
                    "- Per-Step Gamma Warp: \"60*1.0 [62 64]*2.0 65*0.5 [67 71]*-1.0\"\n"
                    "- Rests: \"c4 ~ [e4 g4] ~ b4 ~\"\n\n"
                    "CONTROL OUTLETS:\n"
                    "Outlets: pitch, gate~, gate, gain, pan, cutoff, timeOut (Time), cyclePhase.");
            }
            else if (result == 9)
            {
                showHelpDialog ("Future Lookahead Causality Engine ([time.future~])",
                    "FUTURE LOOKAHEAD CAUSALITY OFFSET ENGINE ([time.future~])\n\n"
                    "Core Principle: Sound from the future may occur; we do not play it yet, but if we need to, we are instantly ready to do so.\n\n"
                    "LATENT PRE-RENDERING:\n"
                    "Future calculation threads pre-render audio frames into global RAM buffers. When a future lookahead node is triggered, audio plays with zero-latency instant execution!");
            }
            else if (result == 3)
            {
                showHelpDialog ("Wireless Signal Tapping Syntax (tap())",
                    "WIRELESS GLOBAL SIGNAL TAPPING SYNTAX\n\n"
                    "You can wirelessly tap signals from ANY object on the canvas inside expressions or parameter math fields without drawing patch cables!\n\n"
                    "SYNTAX EXAMPLES:\n"
                    "- tap(id): Taps node by numeric ID (e.g. tap(2))\n"
                    "- tap('label'): Taps node output by label (e.g. tap('osc~ 440 Hz'))\n"
                    "- tap('label.param'): Taps specific internal property (e.g. tap('filter~.cutoff'), tap('osc~.frequency'), tap('time.warp~.gamma'))\n\n"
                    "1-CLICK TAP UX:\n"
                    "- Click the [TAP] button next to any parameter slider in the TOP-DOWN inspector to copy tap('node.param') to your clipboard and insert it into active expressions!");
            }
            else if (result == 4)
            {
                showHelpDialog ("Pure Data Expression Scripting & C++ Math",
                    "PURE DATA EXPRESSION & C++ DSP MATH\n\n"
                    "Every object supports bottom-up code editing.\n\n"
                    "VARIABLES:\n"
                    "- $v1, $v2: Inlet signal 1 & 2\n"
                    "- $t: Dynamic local coordinate time in seconds\n"
                    "- tap('node.param'): Wireless signal tap\n"
                    "- storedValue / prevVal: Retained state across ticks\n\n"
                    "MATH FUNCTIONS:\n"
                    "sin(), cos(), tan(), pow(), sqrt(), abs(), min(), max(), clamp(), exp(), log()");
            }
            else if (result == 5)
            {
                showHelpDialog ("1-Sample Feedback Loop Protection",
                    "1-SAMPLE FEEDBACK LOOP PROTECTION\n\n"
                    "When audio connections form a cyclic loop, Time Dilation DAW automatically flags the loop:\n\n"
                    "- VISUAL WARNING: The cable glows in Neon Red with a [FEEDBACK: 1-SMP DELAY] badge.\n"
                    "- DSP STABILITY: The feedback path reads from a 1-sample memory buffer (Out(n-1)), guaranteeing 100% DSP stability with zero buffer starvation or audio crashes!");
            }
            else if (result == 6)
            {
                showHelpDialog ("Keyboard Shortcuts & Hotkeys",
                    "KEYBOARD SHORTCUTS & WORKSTATION HOTKEYS\n\n"
                    "- N / Double-Click: Open Object Palette Menu\n"
                    "- Cmd+D: Duplicate Selected Nodes\n"
                    "- Cmd+C: Copy Selected Nodes to Clipboard\n"
                    "- Cmd+V: Paste Nodes from Clipboard\n"
                    "- Cmd+Z: Undo Last Patch Action\n"
                    "- Cmd+Shift+Z: Redo Last Patch Action\n"
                    "- Cmd+A: Select All Nodes\n"
                    "- Delete / Backspace: Remove Selected Nodes or Connections\n"
                    "- Shift-Click / Shift-Drag: Multi-select Nodes");
            }
            else if (result == 7)
            {
                showHelpDialog ("About Time Dilation DAW",
                    "Time Dilation DAW (Version 0.0.1)\n"
                    "Producer: Kijjaz\n\n"
                    "A state-of-the-art Relativistic Modular Workstation unifying top-down visual patching with bottom-up authentic C++ / DSP math expression coding.");
            }
        });
}

void RelativisticCanvasComponent::initObjectCatalog()
{
    allRegisteredObjects = {
        // Relativistic Time Engines
        { "time.warp~", "Dilated Coordinate Time Generator (LFO Dilation)", "TIME" },
        { "time.retro~", "Retrograde Time Reverser (-1.0x Time Flow)", "TIME" },
        { "time.stasis~", "Gravitational Time Stasis Freeze Engine", "TIME" },
        { "time.singularity~", "Event Horizon Gravitational Redshift Warp", "TIME" },
        { "time.quantize~", "Metric Grid Time Quantizer (Micro-Step Stutter)", "TIME" },
        { "time.transport", "Relativistic Master Transport Hub (BPM Clock)", "TIME" },
        { "time.metro~", "Dilated Metronome Pulse Spiker", "TIME" },
        { "time.scope", "Relativistic Time & Telemetry Visualizer Monitor", "TIME" },
        { "time.xy", "2D Time Signal XY Vector Oscilloscope Plot", "TIME" },
        { "time.future~", "Future Lookahead Causality Offset Engine", "TIME" },

        // Audio & DSP Generators & Processors
        { "spectrometer~", "Live Audio Spectrum Visualizer (Logo Gradient)", "DSP" },
        { "spectrum~", "Audio Frequency Spectrometer Visualizer", "DSP" },
        { "fft~", "Fast Fourier Transform Audio Analyzer", "DSP" },
        { "osc~", "Sine/Saw/Square Varispeed Oscillator", "DSP" },
        { "phasor~", "Linear Ramp Audio Phase Generator", "DSP" },
        { "sampler~", "Varispeed Audio Sampler & Loop Player", "DSP" },
        { "filter~", "State-Variable Filter (LP/HP/BP/Notch)", "DSP" },
        { "svfilter~", "Vadim Zavalishin TPT State-Variable Filter", "DSP" },
        { "delay~", "Feedback Delay Line (Hermite Varispeed)", "DSP" },
        { "drive~", "Non-Linear Harmonic Tube Overdrive Distortion", "DSP" },
        { "reverb~", "Stereo Algorithmic Reverb Unit", "DSP" },
        { "crush~", "Quantum Bitcrusher & Sample Reducer", "DSP" },
        { "adsr~", "Attack-Decay-Sustain-Release Envelope Generator", "DSP" },
        { "env~", "Audio Envelope Follower (Peak Detector)", "DSP" },
        { "gain~", "Relativistic Audio Signal Scaler & Time Warper", "DSP" },
        { "out~", "Master Output Fader & Live Oscilloscope CRT", "DSP" },
        { "dac~", "Audio Master Output Hardware DAC", "DSP" },
        { "fbdrum~", "Polyphonic Future Bass Drum Synthesizer", "DSP" },
        { "tabosc4~", "4-Point Hermite Interpolated Wavetable Oscillator", "DSP" },

        // Sequencers & Generative Engines
        { "seq", "Multi-Step Pattern Sequencer", "SEQ" },
        { "drumseq", "Multi-Track 16-Step Future Bass Drum Sequencer", "SEQ" },
        { "euclid", "Euclidean Rhythm Generator", "SEQ" },
        { "markov", "Stochastic Markov Chain Melodic Generator", "SEQ" },
        { "tidal", "Tidal Live-Coding Mini-Notation Sequencer", "SEQ" },
        { "timeline", "Multi-Track Timeline Clip Sequencer", "SEQ" },

        // Control Interactors & Triggers
        { "number", "Control Number Box (Click & Drag Value)", "CTRL" },
        { "num", "Control Number Box (Click & Drag Value)", "CTRL" },
        { "msg", "Message Box (Store & Send Control Value)", "CTRL" },
        { "message", "Message Box (Store & Send Control Value)", "CTRL" },
        { "bang", "Control Trigger Pulse Spiker", "CTRL" },
        { "bang~", "Audio-Rate Impulse Spike Spiker", "CTRL" },
        { "counter", "Smart Value Counter (Low, High, Step, Carry)", "CTRL" },
        { "note", "MIDI Note Pitch Generator", "CTRL" },
        { "tap", "Control Signal Wireless Tap Listener", "CTRL" },
        { "tap~", "Audio Signal Wireless Tap Listener", "CTRL" },

        // Expressions & Math
        { "expr", "Pure Data Control Expression ($v1, tap('id'))", "MATH" },
        { "expr~", "Pure Data Audio Expression ($v1, tap('id'))", "MATH" },
        { "fexpr~", "Filter Recurrent Expression ($y1[-1])", "MATH" },
        { "v", "Value Storage Control Node", "MATH" },
        { "z~", "1-Sample Feedback Delay Unit", "MATH" },
        { "snapshot~", "Audio-to-Control Sample Snapshot", "MATH" },
        { "+", "Signal & Control Adder", "MATH" },
        { "*", "Signal & Control Multiplier", "MATH" },
        { "mtof", "MIDI Pitch to Frequency Hz Converter", "MATH" },
        { "ftom", "Frequency Hz to MIDI Pitch Converter", "MATH" },

        // Tables & Data Memory
        { "table", "Wavetable / Step Value Memory Canvas", "DATA" },
        { "tabwrite~", "Write Audio Buffer to Table Memory", "DATA" },
        { "tabread~", "Read Audio Buffer from Table Memory", "DATA" }
    };
    filteredAutocompleteItems = allRegisteredObjects;
}

void RelativisticCanvasComponent::spawnInlineObjectEditor (juce::Point<float> spawnCanvasPos)
{
    if (isEditingDraftObject)
        destroyDraftObjectEditor();

    draftObjectCanvasPos = spawnCanvasPos;
    isEditingDraftObject = true;
    isDraftObjectInvalid = false;
    draftWarningText = "";

    float screenX = draftObjectCanvasPos.x + panX;
    float screenY = draftObjectCanvasPos.y + panY;

    draftObjectEditor = std::make_unique<juce::TextEditor>();
    draftObjectEditor->setBounds (static_cast<int>(screenX), static_cast<int>(screenY), 190, 34);
    draftObjectEditor->setFont (FontManager::getInstance().getOxaniumFont (15.0f, true));

    draftObjectEditor->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0d1322));
    draftObjectEditor->setColour (juce::TextEditor::textColourId, juce::Colour (0xfff8fafc));
    draftObjectEditor->setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff06b6d4));
    draftObjectEditor->setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour (0xff06b6d4));

    updateAutocompleteFilter ("");

    draftObjectEditor->onTextChange = [this] {
        if (draftObjectEditor)
        {
            updateAutocompleteFilter (draftObjectEditor->getText());
            isDraftObjectInvalid = false;
            draftWarningText = "";
            draftObjectEditor->setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff06b6d4));
            draftObjectEditor->setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour (0xff06b6d4));
            draftObjectEditor->setColour (juce::TextEditor::textColourId, juce::Colour (0xfff8fafc));
            repaint();
        }
    };

    draftObjectEditor->onReturnKey = [this] {
        commitDraftObject();
    };

    draftObjectEditor->onEscapeKey = [this] {
        destroyDraftObjectEditor();
        repaint();
    };

    draftObjectEditor->onFocusLost = [this] {
        if (draftObjectEditor)
        {
            juce::String txt = draftObjectEditor->getText().trim();
            if (txt.isEmpty())
            {
                destroyDraftObjectEditor();
                repaint();
            }
            else if (!isDraftObjectInvalid)
            {
                commitDraftObject();
            }
        }
    };

    addAndMakeVisible (*draftObjectEditor);
    draftObjectEditor->grabKeyboardFocus();
    repaint();
}

void RelativisticCanvasComponent::destroyDraftObjectEditor()
{
    if (draftObjectEditor)
    {
        removeChildComponent (draftObjectEditor.get());
        draftObjectEditor.reset();
    }
    isEditingDraftObject = false;
    isDraftObjectInvalid = false;
    draftWarningText = "";
    selectedAutocompleteIdx = 0;
}

void RelativisticCanvasComponent::updateAutocompleteFilter (const juce::String& text)
{
    filteredAutocompleteItems.clear();
    juce::String clean = text.trim().toLowerCase();

    juce::String firstWord = clean;
    if (clean.containsChar (' '))
        firstWord = clean.upToFirstOccurrenceOf (" ", false, false);

    if (firstWord.isEmpty())
    {
        filteredAutocompleteItems = allRegisteredObjects;
    }
    else
    {
        for (const auto& item : allRegisteredObjects)
        {
            juce::String tName = juce::String (item.typeName).toLowerCase();
            juce::String desc = juce::String (item.description).toLowerCase();
            if (tName.startsWith (firstWord) || tName.contains (firstWord) || desc.contains (firstWord))
            {
                filteredAutocompleteItems.push_back (item);
            }
        }
    }
    selectedAutocompleteIdx = 0;
}

void RelativisticCanvasComponent::commitDraftObject()
{
    if (!draftObjectEditor) return;

    juce::String fullText = draftObjectEditor->getText().trim();
    if (fullText.isEmpty())
    {
        destroyDraftObjectEditor();
        repaint();
        return;
    }

    juce::StringArray tokens;
    tokens.addTokens (fullText, " ", "");
    std::string typeToken = tokens[0].toStdString();

    if (selectedAutocompleteIdx >= 0 && selectedAutocompleteIdx < static_cast<int>(filteredAutocompleteItems.size()))
    {
        typeToken = filteredAutocompleteItems[selectedAutocompleteIdx].typeName;
    }

    if (!RelativisticNodeGraph::isValidObjectType (typeToken))
    {
        // Invalid object name entered! Keep box open with RED warning!
        isDraftObjectInvalid = true;
        draftWarningText = "no such object available";
        if (draftObjectEditor)
        {
            draftObjectEditor->setColour (juce::TextEditor::outlineColourId, juce::Colour (0xffef4444));
            draftObjectEditor->setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour (0xffef4444));
            draftObjectEditor->setColour (juce::TextEditor::textColourId, juce::Colour (0xfffca5a5));
        }
        repaint();
        return; // Keep open!
    }

    // Valid object!
    int newId = nodeGraph.addNode (typeToken, draftObjectCanvasPos.x, draftObjectCanvasPos.y);
    auto n = nodeGraph.getNodeById (newId);
    if (n)
    {
        if (fullText.length() > typeToken.length())
        {
            n->setLabel (fullText.toStdString());
            n->parseLabelArguments (fullText.toStdString());
        }
    }

    selectedNodeIds.clear();
    selectedNodeIds.insert (newId);
    destroyDraftObjectEditor();
    rebuildInspector();
    repaint();
}

void RelativisticCanvasComponent::showObjectSearchMenu (juce::Point<float> spawnPos)
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- RELATIVISTIC TIME ENGINES ---");
    m.addItem (1, "[time.warp~]\tDilated Coordinate Time Generator", true);
    m.addItem (2, "[time.retro~]\tRetrograde Time Reverser", true);
    m.addItem (3, "[time.quantize~]\tMetric Grid Time Quantizer", true);
    m.addItem (4, "[time.metro~]\tDilated Metronome Pulse Generator", true);
    m.addItem (24, "[time.stasis~]\tGravitational Time Stasis Freeze Engine", true);
    m.addItem (25, "[time.singularity~]\tEvent Horizon Gravitational Redshift Warp", true);
    m.addItem (40, "[time.future~]\tFuture Lookahead Causality Offset Engine", true);
    m.addItem (41, "[time.transport]\tRelativistic Master Transport Hub", true);
    m.addItem (30, "[time.scope]\tRelativistic Time & Telemetry Visualizer Monitor", true);

    m.addSeparator();
    m.addSectionHeader ("--- RELATIVISTIC SEQUENCERS & PATTERN ENGINES ---");
    m.addItem (35, "[seq]\tMulti-Step Pattern Sequencer", true);
    m.addItem (36, "[euclid]\tEuclidean Rhythm Generator", true);
    m.addItem (37, "[markov]\tStochastic Markov Chain Sequencer", true);
    m.addItem (38, "[tidal]\tTidal Live-Coding Mini-Notation Suite", true);
    m.addItem (39, "[timeline]\tMulti-Track Timeline Clip Sequencer", true);
    m.addItem (43, "[drumseq]\tMulti-Track Future Bass Drum Sequencer", true);

    m.addSeparator();
    m.addSectionHeader ("--- CONTROL INTERACTORS & TRIGGERS (PD-STYLE) ---");
    m.addItem (31, "[number]\tControl Number Box (Click & Drag Value)", true);
    m.addItem (32, "[bang]\tControl Trigger Pulse Generator", true);
    m.addItem (33, "[bang~]\tAudio-Rate Impulse Spike Generator", true);
    m.addItem (34, "[counter]\tSmart Value Counter (Low, High, Step, Carry Out)", true);

    m.addSeparator();
    m.addSectionHeader ("--- AUDIO & DSP PROCESSORS ---");
    m.addItem (5, "[osc~]\tSine/Saw/Square Oscillator", true);
    m.addItem (6, "[phasor~]\tLinear Ramp Phase Generator", true);
    m.addItem (7, "[sampler~]\tAudio Buffer Sampler", true);
    m.addItem (8, "[filter~]\tState-Variable Filter", true);
    m.addItem (9, "[delay~]\tFeedback Delay Line", true);
    m.addItem (10, "[dac~]\tAudio Master Output DAC", true);
    m.addItem (14, "[gain~]\tAudio Signal Scaler", true);
    m.addItem (15, "[out~]\tMaster Output Fader & Live Meters", true);
    m.addItem (16, "[env~]\tEnvelope Follower (Peak Detector)", true);
    m.addItem (17, "[tap]\tControl Signal Wireless Tap", true);
    m.addItem (18, "[tap~]\tAudio Signal Wireless Tap", true);
    m.addItem (42, "[fbdrum~]\tFuture Bass Drum Synthesizer Engine", true);

    m.addSeparator();
    m.addSectionHeader ("--- MATH SIGNAL EXPRESSIONS & CONTROL NODES ---");
    m.addItem (11, "[expr]\tControl Expression ($v1, tap('node.prop'))", true);
    m.addItem (12, "[expr~]\tAudio Expression ($v1, tap('node.prop'))", true);
    m.addItem (13, "[fexpr~]\tFilter Recurrent Expression ($y1[-1])", true);
    m.addItem (19, "[v]\tValue Storage Control Node", true);
    m.addItem (20, "[z~]\t1-Sample Feedback Delay Node", true);
    m.addItem (21, "[snapshot~]\tAudio-to-Control Snapshot Node", true);
    m.addItem (22, "[+]\tSignal & Control Adder", true);
    m.addItem (23, "[*]\tSignal & Control Multiplier", true);

    m.addSeparator();
    m.addSectionHeader ("--- TABLES & ARRAY DATA NODES ---");
    m.addItem (26, "[table]\tInteractive Table / Array Buffer", true);
    m.addItem (27, "[tabread~]\tTable Sample / Pitch Reader", true);
    m.addItem (28, "[tabwrite~]\tLive Audio/Data Table Recorder", true);
    m.addItem (29, "[tabosc4~]\t4-Pt Wavetable Oscillator", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (juce::Rectangle<int> (static_cast<int>(spawnPos.x), static_cast<int>(spawnPos.y), 1, 1)),
        [this, spawnPos] (int result) {
            std::string typeName;
            if (result == 1) typeName = "time.warp~";
            else if (result == 2) typeName = "time.retro~";
            else if (result == 3) typeName = "time.quantize~";
            else if (result == 4) typeName = "time.metro~";
            else if (result == 5) typeName = "osc~";
            else if (result == 6) typeName = "phasor~";
            else if (result == 7) typeName = "sampler~";
            else if (result == 8) typeName = "filter~";
            else if (result == 9) typeName = "delay~";
            else if (result == 10) typeName = "dac~";
            else if (result == 11) typeName = "expr";
            else if (result == 12) typeName = "expr~";
            else if (result == 13) typeName = "fexpr~";
            else if (result == 14) typeName = "gain~";
            else if (result == 15) typeName = "out~";
            else if (result == 16) typeName = "env~";
            else if (result == 17) typeName = "tap";
            else if (result == 18) typeName = "tap~";
            else if (result == 19) typeName = "v";
            else if (result == 20) typeName = "z~";
            else if (result == 21) typeName = "snapshot~";
            else if (result == 22) typeName = "+";
            else if (result == 23) typeName = "*";
            else if (result == 24) typeName = "time.stasis~";
            else if (result == 25) typeName = "time.singularity~";
            else if (result == 26) typeName = "table";
            else if (result == 27) typeName = "tabread~";
            else if (result == 28) typeName = "tabwrite~";
            else if (result == 29) typeName = "tabosc4~";
            else if (result == 30) typeName = "time.scope";
            else if (result == 31) typeName = "number";
            else if (result == 32) typeName = "bang";
            else if (result == 33) typeName = "bang~";
            else if (result == 34) typeName = "counter";
            else if (result == 35) typeName = "seq";
            else if (result == 36) typeName = "euclid";
            else if (result == 37) typeName = "markov";
            else if (result == 38) typeName = "tidal";
            else if (result == 39) typeName = "timeline";
            else if (result == 40) typeName = "time.future~";
            else if (result == 41) typeName = "time.transport";
            else if (result == 42) typeName = "fbdrum~";
            else if (result == 43) typeName = "drumseq";

            if (!typeName.empty())
            {
                nodeGraph.pushUndoState();
                int id = nodeGraph.addNode (typeName, spawnPos.x, spawnPos.y);
                selectedNodeIds.clear();
                selectedNodeIds.insert (id);
                rebuildInspector();
                repaint();
            }
        });
}

void RelativisticCanvasComponent::rebuildInspector()
{
    propertyRows.clear();
    methodButtons.clear();

    int primaryId = !selectedNodeIds.empty() ? *selectedNodeIds.begin() : 0;
    auto node = nodeGraph.getNodeById (primaryId);
    if (!node)
    {
        inspectorTitleLabel.setText ("INSPECTOR: NO NODE SELECTED", juce::dontSendNotification);
        formulaEditor.setText ("// Select a node on the canvas to view or edit its C++ / DSP math formula.");
        resized();
        return;
    }

    inspectorTitleLabel.setText ("INSPECTOR: [" + node->getLabel() + "]", juce::dontSendNotification);
    formulaEditor.setText (node->getFormulaScript());

    auto defs = node->getParameterDefs();
    for (const auto& def : defs)
    {
        InspectorPropertyRow row;
        row.key = def.key;
        std::string paramKey = def.key;

        row.label = std::make_unique<juce::Label>();
        row.label->setText (def.name + ":", juce::dontSendNotification);
        row.label->setFont (juce::FontOptions (12.0f, juce::Font::bold));
        addAndMakeVisible (*row.label);

        row.slider = std::make_unique<juce::Slider>();
        row.slider->setSliderStyle (juce::Slider::LinearHorizontal);
        row.slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 65, 18);
        float minR = std::min (-99999.0f, def.minValue);
        float maxR = std::max (99999.0f, def.maxValue);
        row.slider->setRange (minR, maxR, 0.01);
        row.slider->setValue (def.value);
        row.slider->onValueChange = [this, primaryId, paramKey, sl = row.slider.get()] {
            auto n = nodeGraph.getNodeById (primaryId);
            if (n)
            {
                n->setParameter (paramKey, static_cast<float>(sl->getValue()));
                repaint();
            }
        };
        addAndMakeVisible (*row.slider);

        row.exprEditor = std::make_unique<juce::TextEditor>();
        row.exprEditor->setText (def.expression.empty() ? "expr: " + def.name : def.expression);
        row.exprEditor->setFont (juce::FontOptions (11.0f));
        row.exprEditor->onReturnKey = [this, primaryId, paramKey, ed = row.exprEditor.get()] {
            nodeGraph.pushUndoState();
            auto n = nodeGraph.getNodeById (primaryId);
            if (n) n->setParamExpression (paramKey, ed->getText().toStdString());
        };
        addAndMakeVisible (*row.exprEditor);

        bool hasMod = node->hasModulationInlet (paramKey);
        row.btnModInlet = std::make_unique<juce::TextButton> (hasMod ? "[- REMOVE MOD]" : "+ MOD INLET");
        row.btnModInlet->setColour (juce::TextButton::buttonColourId, hasMod ? juce::Colour (0xff991b1b) : juce::Colour (0xff0e7490));
        row.btnModInlet->onClick = [this, primaryId, paramKey, hasMod] {
            if (hasMod)
            {
                nodeGraph.removeModulationInlet (primaryId, paramKey);
            }
            else
            {
                auto n = nodeGraph.getNodeById (primaryId);
                if (n) n->addModulationInlet (paramKey);
            }
            rebuildInspector();
            repaint();
        };
        addAndMakeVisible (*row.btnModInlet);

        row.btnTapValue = std::make_unique<juce::TextButton> ("[TAP]");
        row.btnTapValue->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f766e));
        row.btnTapValue->onClick = [this, primaryId, paramKey] {
            auto n = nodeGraph.getNodeById (primaryId);
            if (n)
            {
                std::string snippet = "tap('" + n->getLabel() + "." + paramKey + "')";
                juce::SystemClipboard::copyTextToClipboard (snippet);
                formulaEditor.insertTextAtCaret (" + " + snippet);
            }
        };
        addAndMakeVisible (*row.btnTapValue);

        propertyRows.push_back (std::move (row));
    }

    auto methods = node->getExposedMethods();
    for (const auto& mName : methods)
    {
        auto btn = std::make_unique<juce::TextButton> ("[EXEC] " + mName);
        std::string m = mName;
        btn->onClick = [this, primaryId, m] {
            auto n = nodeGraph.getNodeById (primaryId);
            if (n)
            {
                if (m == "Load Audio File..." || m == "Load Sample File...")
                {
                    auto fc = std::make_shared<juce::FileChooser> ("Select Audio Sample File...",
                                                                   juce::File::getSpecialLocation (juce::File::userHomeDirectory),
                                                                   "*.wav;*.aif;*.aiff;*.mp3;*.flac");
                    fc->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, primaryId, fc] (const juce::FileChooser& chooser) {
                            auto result = chooser.getResult();
                            if (result.existsAsFile())
                            {
                                auto n = nodeGraph.getNodeById (primaryId);
                                auto sampler = std::dynamic_pointer_cast<SamplerNode> (n);
                                if (sampler) sampler->loadAudioFile (result);
                                rebuildInspector();
                                repaint();
                            }
                        });
                }
                else
                {
                    n->invokeMethod (m);
                    rebuildInspector();
                    repaint();
                }
            }
        };
        btn->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff8b5cf6));
        btn->setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        addAndMakeVisible (*btn);
        methodButtons.push_back (std::move (btn));
    }

    // Populate INCOMING and OUTGOING connection sections
    connectionRows.clear();

    incomingSectionHeader = std::make_unique<juce::Label>();
    incomingSectionHeader->setText ("INCOMING CONNECTIONS (FROM):", juce::dontSendNotification);
    incomingSectionHeader->setFont (juce::FontOptions (12.0f, juce::Font::bold));
    incomingSectionHeader->setColour (juce::Label::textColourId, juce::Colour (0xff06b6d4));
    addAndMakeVisible (*incomingSectionHeader);

    outgoingSectionHeader = std::make_unique<juce::Label>();
    outgoingSectionHeader->setText ("OUTGOING CONNECTIONS (TO):", juce::dontSendNotification);
    outgoingSectionHeader->setFont (juce::FontOptions (12.0f, juce::Font::bold));
    outgoingSectionHeader->setColour (juce::Label::textColourId, juce::Colour (0xff8b5cf6));
    addAndMakeVisible (*outgoingSectionHeader);

    int incCount = 0;
    for (const auto& conn : nodeGraph.getConnections())
    {
        if (conn.destNodeId == primaryId)
        {
            incCount++;
            InspectorConnectionRow r;
            r.connectionId = conn.id;
            r.isIncoming = true;

            auto srcNode = nodeGraph.getNodeById (conn.sourceNodeId);
            std::string srcName = srcNode ? srcNode->getLabel() : "node";
            std::string inPortName = (conn.destInletIdx >= 0 && conn.destInletIdx < static_cast<int>(node->getInlets().size())) ? node->getInlets()[conn.destInletIdx].name : std::to_string(conn.destInletIdx);

            r.label = std::make_unique<juce::Label>();
            r.label->setText ("FROM [" + srcName + "] out" + std::to_string(conn.sourceOutletIdx) + " -> in" + std::to_string(conn.destInletIdx) + " (" + inPortName + ")", juce::dontSendNotification);
            r.label->setFont (juce::FontOptions (11.0f));
            addAndMakeVisible (*r.label);

            int cid = conn.id;
            r.btnRemoveWire = std::make_unique<juce::TextButton> ("[REMOVE]");
            r.btnRemoveWire->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff991b1b));
            r.btnRemoveWire->onClick = [this, cid] {
                nodeGraph.pushUndoState();
                nodeGraph.removeConnection (cid);
                rebuildInspector();
                repaint();
            };
            addAndMakeVisible (*r.btnRemoveWire);

            connectionRows.push_back (std::move (r));
        }
    }

    if (incCount == 0)
    {
        InspectorConnectionRow r;
        r.label = std::make_unique<juce::Label>();
        r.label->setText (" (No incoming connections)", juce::dontSendNotification);
        r.label->setFont (juce::FontOptions (10.5f, juce::Font::italic));
        r.label->setColour (juce::Label::textColourId, juce::Colours::grey);
        addAndMakeVisible (*r.label);
        r.isIncoming = true;
        connectionRows.push_back (std::move (r));
    }

    int outCount = 0;
    for (const auto& conn : nodeGraph.getConnections())
    {
        if (conn.sourceNodeId == primaryId)
        {
            outCount++;
            InspectorConnectionRow r;
            r.connectionId = conn.id;
            r.isIncoming = false;

            auto destNode = nodeGraph.getNodeById (conn.destNodeId);
            std::string destName = destNode ? destNode->getLabel() : "node";
            std::string outPortName = (conn.sourceOutletIdx >= 0 && conn.sourceOutletIdx < static_cast<int>(node->getOutlets().size())) ? node->getOutlets()[conn.sourceOutletIdx].name : std::to_string(conn.sourceOutletIdx);

            r.label = std::make_unique<juce::Label>();
            r.label->setText ("TO [" + destName + "] out" + std::to_string(conn.sourceOutletIdx) + " (" + outPortName + ") -> in" + std::to_string(conn.destInletIdx), juce::dontSendNotification);
            r.label->setFont (juce::FontOptions (11.0f));
            addAndMakeVisible (*r.label);

            int cid = conn.id;
            r.btnRemoveWire = std::make_unique<juce::TextButton> ("[REMOVE]");
            r.btnRemoveWire->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff991b1b));
            r.btnRemoveWire->onClick = [this, cid] {
                nodeGraph.pushUndoState();
                nodeGraph.removeConnection (cid);
                rebuildInspector();
                repaint();
            };
            addAndMakeVisible (*r.btnRemoveWire);

            connectionRows.push_back (std::move (r));
        }
    }

    if (outCount == 0)
    {
        InspectorConnectionRow r;
        r.label = std::make_unique<juce::Label>();
        r.label->setText (" (No outgoing connections)", juce::dontSendNotification);
        r.label->setFont (juce::FontOptions (10.5f, juce::Font::italic));
        r.label->setColour (juce::Label::textColourId, juce::Colours::grey);
        addAndMakeVisible (*r.label);
        r.isIncoming = false;
        connectionRows.push_back (std::move (r));
    }

    resized();
}

bool RelativisticCanvasComponent::keyPressed (const juce::KeyPress& key)
{
    if (isEditingDraftObject)
    {
        if (key.getKeyCode() == juce::KeyPress::escapeKey)
        {
            destroyDraftObjectEditor();
            repaint();
            return true;
        }
        if (key.getKeyCode() == juce::KeyPress::downKey)
        {
            if (!filteredAutocompleteItems.empty())
            {
                selectedAutocompleteIdx = (selectedAutocompleteIdx + 1) % static_cast<int>(filteredAutocompleteItems.size());
                if (draftObjectEditor && selectedAutocompleteIdx >= 0 && selectedAutocompleteIdx < static_cast<int>(filteredAutocompleteItems.size()))
                {
                    draftObjectEditor->setText (filteredAutocompleteItems[selectedAutocompleteIdx].typeName + " ");
                    draftObjectEditor->setCaretPosition (draftObjectEditor->getText().length());
                }
                repaint();
                return true;
            }
        }
        if (key.getKeyCode() == juce::KeyPress::upKey)
        {
            if (!filteredAutocompleteItems.empty())
            {
                selectedAutocompleteIdx = (selectedAutocompleteIdx - 1 + static_cast<int>(filteredAutocompleteItems.size())) % static_cast<int>(filteredAutocompleteItems.size());
                if (draftObjectEditor && selectedAutocompleteIdx >= 0 && selectedAutocompleteIdx < static_cast<int>(filteredAutocompleteItems.size()))
                {
                    draftObjectEditor->setText (filteredAutocompleteItems[selectedAutocompleteIdx].typeName + " ");
                    draftObjectEditor->setCaretPosition (draftObjectEditor->getText().length());
                }
                repaint();
                return true;
            }
        }
        if (key.getKeyCode() == juce::KeyPress::tabKey)
        {
            if (!filteredAutocompleteItems.empty() && selectedAutocompleteIdx >= 0 && selectedAutocompleteIdx < static_cast<int>(filteredAutocompleteItems.size()))
            {
                if (draftObjectEditor)
                {
                    draftObjectEditor->setText (filteredAutocompleteItems[selectedAutocompleteIdx].typeName + " ");
                    draftObjectEditor->setCaretPosition (draftObjectEditor->getText().length());
                }
                repaint();
                return true;
            }
        }
    }

    bool isCmdOrCtrl = key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown();

    // 1. Select All (Cmd-A)
    if (isCmdOrCtrl && (key.getKeyCode() == 'A' || key.getKeyCode() == 'a'))
    {
        selectedNodeIds.clear();
        for (const auto& n : nodeGraph.getNodes())
        {
            selectedNodeIds.insert (n->getId());
        }
        repaint();
        return true;
    }

    // 2. Duplicate (Cmd-D)
    if (isCmdOrCtrl && (key.getKeyCode() == 'D' || key.getKeyCode() == 'd'))
    {
        if (!selectedNodeIds.empty())
        {
            std::vector<int> sel (selectedNodeIds.begin(), selectedNodeIds.end());
            auto newIds = nodeGraph.duplicateNodes (sel);
            selectedNodeIds.clear();
            selectedNodeIds.insert (newIds.begin(), newIds.end());
            repaint();
            return true;
        }
    }

    // 3. Copy (Cmd-C)
    if (isCmdOrCtrl && (key.getKeyCode() == 'C' || key.getKeyCode() == 'c'))
    {
        if (!selectedNodeIds.empty())
        {
            std::vector<int> sel (selectedNodeIds.begin(), selectedNodeIds.end());
            clipboardTree = nodeGraph.copyNodes (sel);
            return true;
        }
    }

    // 4. Cut (Cmd-X)
    if (isCmdOrCtrl && (key.getKeyCode() == 'X' || key.getKeyCode() == 'x'))
    {
        if (!selectedNodeIds.empty())
        {
            std::vector<int> sel (selectedNodeIds.begin(), selectedNodeIds.end());
            clipboardTree = nodeGraph.copyNodes (sel);
            nodeGraph.cutNodes (sel);
            selectedNodeIds.clear();
            repaint();
            return true;
        }
    }

    // 5. Paste (Cmd-V)
    if (isCmdOrCtrl && (key.getKeyCode() == 'V' || key.getKeyCode() == 'v'))
    {
        if (clipboardTree.isValid())
        {
            auto newIds = nodeGraph.pasteNodes (clipboardTree);
            selectedNodeIds.clear();
            selectedNodeIds.insert (newIds.begin(), newIds.end());
            repaint();
            return true;
        }
    }

    // 6. Undo (Cmd-Z)
    if (isCmdOrCtrl && !key.getModifiers().isShiftDown() && (key.getKeyCode() == 'Z' || key.getKeyCode() == 'z'))
    {
        if (nodeGraph.undo())
        {
            selectedNodeIds.clear();
            selectedConnectionId = 0;
            rebuildInspector();
            repaint();
            return true;
        }
    }

    // 7. Redo (Cmd-Shift-Z or Cmd-Y)
    if ((isCmdOrCtrl && key.getModifiers().isShiftDown() && (key.getKeyCode() == 'Z' || key.getKeyCode() == 'z')) ||
        (isCmdOrCtrl && (key.getKeyCode() == 'Y' || key.getKeyCode() == 'y')))
    {
        if (nodeGraph.redo())
        {
            selectedNodeIds.clear();
            selectedConnectionId = 0;
            rebuildInspector();
            repaint();
            return true;
        }
    }
    // 8a. Zoom In (Cmd+= / Cmd++)
    if (isCmdOrCtrl && (key.getKeyCode() == '=' || key.getKeyCode() == '+'))
    {
        zoomIn();
        return true;
    }

    // 8b. Zoom Out (Cmd--)
    if (isCmdOrCtrl && (key.getKeyCode() == '-' || key.getKeyCode() == '_'))
    {
        zoomOut();
        return true;
    }

    // 8c. Reset Zoom 100% (Cmd-0)
    if (isCmdOrCtrl && key.getKeyCode() == '0')
    {
        resetZoom();
        return true;
    }

    // 9. Add Object Hotkey (Cmd+1 or N key)
    if ((isCmdOrCtrl && key.getKeyCode() == '1') ||
        (!isCmdOrCtrl && (key.getKeyCode() == 'N' || key.getKeyCode() == 'n')))
    {
        spawnInlineObjectEditor ({ getWidth() * 0.4f, getHeight() * 0.4f });
        return true;
    }

    // Save Project File (Cmd+Shift+S for Save As, Cmd+S for Save)
    if (isCmdOrCtrl && (key.getKeyCode() == 'S' || key.getKeyCode() == 's'))
    {
        if (key.getModifiers().isShiftDown())
            savePatchAs();
        else
            savePatch();
        return true;
    }

    // Open Project File (Cmd+O / Ctrl+O)
    if (isCmdOrCtrl && (key.getKeyCode() == 'O' || key.getKeyCode() == 'o'))
    {
        loadPatch();
        return true;
    }

    // 10. Delete / Backspace Key
    if (key.getKeyCode() == juce::KeyPress::deleteKey || key.getKeyCode() == juce::KeyPress::backspaceKey)
    {
        if (selectedConnectionId > 0)
        {
            nodeGraph.pushUndoState();
            nodeGraph.removeConnection (selectedConnectionId);
            selectedConnectionId = 0;
            repaint();
            return true;
        }
        if (!selectedNodeIds.empty())
        {
            std::vector<int> sel (selectedNodeIds.begin(), selectedNodeIds.end());
            nodeGraph.cutNodes (sel);
            selectedNodeIds.clear();
            repaint();
            return true;
        }
    }
    // 11. Arrow Keys (Move Selected Nodes or Pan Canvas Viewport)
    if (key.getKeyCode() == juce::KeyPress::upKey ||
        key.getKeyCode() == juce::KeyPress::downKey ||
        key.getKeyCode() == juce::KeyPress::leftKey ||
        key.getKeyCode() == juce::KeyPress::rightKey)
    {
        float step = key.getModifiers().isShiftDown() ? 1.0f : 10.0f;
        float dx = 0.0f, dy = 0.0f;
        if (key.getKeyCode() == juce::KeyPress::leftKey)  dx = -step;
        if (key.getKeyCode() == juce::KeyPress::rightKey) dx = step;
        if (key.getKeyCode() == juce::KeyPress::upKey)    dy = -step;
        if (key.getKeyCode() == juce::KeyPress::downKey)  dy = step;

        if (!selectedNodeIds.empty())
        {
            nodeGraph.pushUndoState();
            for (int id : selectedNodeIds)
            {
                auto n = nodeGraph.getNodeById (id);
                if (n) n->setPosition (n->getX() + dx, n->getY() + dy);
            }
            rebuildInspector();
            repaint();
            return true;
        }
        else
        {
            panX += dx * 2.0f;
            panY += dy * 2.0f;
            repaint();
            return true;
        }
    }

    return false;
}

RelativisticCanvasComponent::~RelativisticCanvasComponent()
{
    setLookAndFeel (nullptr);
    stopTimer();
}

void RelativisticCanvasComponent::timerCallback()
{
    double hzSec = nodeGraph.getCurrentCausalityHorizonSec();
    btnHorizonReadout.setButtonText ("HORIZON: +" + juce::String (hzSec, 3) + "s");
    repaint();
}float RelativisticCanvasComponent::getNodeWidth (const RelativisticNode& node) const
{
    if (node.getTypeName() == "time.scope" || node.getTypeName() == "time.display" || node.getTypeName() == "time.monitor")
        return 190.0f;
    if (node.getTypeName() == "time.xy" || node.getTypeName() == "xy" || node.getTypeName() == "xy~" || node.getTypeName() == "plot.xy")
        return 190.0f;
    if (node.getTypeName() == "spectrometer~" || node.getTypeName() == "spectrum~" || node.getTypeName() == "fft~")
        return 220.0f;
    if (node.getTypeName() == "table")
        return 180.0f;
    if (node.getTypeName() == "out~" || node.getTypeName() == "out")
        return 210.0f;
    if (node.getTypeName() == "seq" || node.getTypeName() == "step")
        return 240.0f;
    if (node.getTypeName() == "meter~" || node.getTypeName() == "vu~")
        return 160.0f;
    if (node.getTypeName() == "number~" || node.getTypeName() == "num~")
        return 130.0f;
    if (node.getTypeName() == "print" || node.getTypeName() == "monitor")
        return 220.0f;

    juce::Font labelFont = FontManager::getInstance().getOxaniumFont (14.0f, true);
    juce::Font portFont = FontManager::getInstance().getOxaniumFont (9.5f, false);

    float textW = static_cast<float>(labelFont.getStringWidth (node.getLabel())) + 38.0f;

    float inletsTotalW = 0.0f;
    for (const auto& in : node.getInlets())
    {
        float w = static_cast<float>(in.name.length()) * 7.5f + 16.0f;
        inletsTotalW += std::max (60.0f, w);
    }

    float outletsTotalW = 0.0f;
    for (const auto& out : node.getOutlets())
    {
        float w = static_cast<float>(out.name.length()) * 7.5f + 16.0f;
        outletsTotalW += std::max (60.0f, w);
    }

    float portsW = std::max (inletsTotalW, outletsTotalW) + 32.0f;

    return std::max ({ 160.0f, textW, portsW });
}

float RelativisticCanvasComponent::getNodeHeight (const RelativisticNode& node) const
{
    if (node.getTypeName() == "time.scope" || node.getTypeName() == "time.display" || node.getTypeName() == "time.monitor")
        return 110.0f;
    if (node.getTypeName() == "time.xy" || node.getTypeName() == "xy" || node.getTypeName() == "xy~" || node.getTypeName() == "plot.xy")
        return 110.0f;
    if (node.getTypeName() == "spectrometer~" || node.getTypeName() == "spectrum~" || node.getTypeName() == "fft~")
        return 110.0f;
    if (node.getTypeName() == "table")
        return 75.0f;
    if (node.getTypeName() == "out~" || node.getTypeName() == "out")
        return 100.0f;
    if (node.getTypeName() == "seq" || node.getTypeName() == "step")
        return 90.0f;
    if (node.getTypeName() == "meter~" || node.getTypeName() == "vu~")
        return 70.0f;
    if (node.getTypeName() == "number~" || node.getTypeName() == "num~")
        return 55.0f;
    if (node.getTypeName() == "print" || node.getTypeName() == "monitor")
        return 110.0f;
    return 52.0f;
}

juce::Point<float> RelativisticCanvasComponent::getInletPos (const RelativisticNode& node, int idx) const

{
    const float nodeW = getNodeWidth (node);
    const int count = static_cast<int>(node.getInlets().size());
    const float spacing = nodeW / static_cast<float>(count + 1);
    return { node.getX() + panX + spacing * (idx + 1), node.getY() + panY };
}

juce::Point<float> RelativisticCanvasComponent::getOutletPos (const RelativisticNode& node, int idx) const
{
    const float nodeW = getNodeWidth (node);
    const float nodeH = getNodeHeight (node);
    const int count = static_cast<int>(node.getOutlets().size());
    const float spacing = nodeW / static_cast<float>(count + 1);
    return { node.getX() + panX + spacing * (idx + 1), node.getY() + panY + nodeH };
}

void RelativisticCanvasComponent::setZoomLevel (float newZoom, juce::Point<float> anchorPos)
{
    float clampedZoom = std::clamp (newZoom, 0.4f, 2.5f);
    if (std::abs (clampedZoom - zoomLevel) < 0.001f) return;

    if (anchorPos.x == 0.0f && anchorPos.y == 0.0f)
    {
        anchorPos = { getWidth() * 0.5f, getHeight() * 0.5f };
    }

    float factor = clampedZoom / zoomLevel;
    panX = anchorPos.x - factor * (anchorPos.x - panX);
    panY = anchorPos.y - factor * (anchorPos.y - panY);

    zoomLevel = clampedZoom;
    repaint();
}

void RelativisticCanvasComponent::zoomIn()
{
    setZoomLevel (zoomLevel * 1.15f);
}

void RelativisticCanvasComponent::zoomOut()
{
    setZoomLevel (zoomLevel / 1.15f);
}

void RelativisticCanvasComponent::resetZoom()
{
    zoomLevel = 1.0f;
    repaint();
}

void RelativisticCanvasComponent::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isAltDown())
    {
        float zoomDelta = (wheel.deltaY > 0.0f) ? 1.1f : 0.9f;
        setZoomLevel (zoomLevel * zoomDelta, e.position);
    }
    else
    {
        panX += wheel.deltaX * 120.0f;
        panY += wheel.deltaY * 120.0f;
        repaint();
    }
}

void RelativisticCanvasComponent::mouseDown (const juce::MouseEvent& e)
{
    grabKeyboardFocus();
    juce::Point<float> mousePos = e.position;
    bool isShift = e.mods.isShiftDown();

    // Autocomplete Popup Item Mouse Click Selection
    if (isEditingDraftObject && draftObjectEditor)
    {
        float popupX = draftObjectEditor->getX();
        float popupY = draftObjectEditor->getY() + draftObjectEditor->getHeight() + 4.0f;
        float popupW = std::max (220.0f, static_cast<float>(draftObjectEditor->getWidth()));
        int maxShow = std::min (6, static_cast<int>(filteredAutocompleteItems.size()));
        float popupH = static_cast<float>(maxShow) * 22.0f + 6.0f;

        if (mousePos.x >= popupX && mousePos.x <= popupX + popupW &&
            mousePos.y >= popupY && mousePos.y <= popupY + popupH)
        {
            int clickedIdx = static_cast<int>((mousePos.y - popupY - 3.0f) / 22.0f);
            if (clickedIdx >= 0 && clickedIdx < maxShow)
            {
                selectedAutocompleteIdx = clickedIdx;
                draftObjectEditor->setText (filteredAutocompleteItems[clickedIdx].typeName + " ");
                commitDraftObject();
                return;
            }
        }
    }

    // 0. Check Table Node Waveform Graph Click / Drag
    for (const auto& node : nodeGraph.getNodes())
    {
        if (auto tableNode = std::dynamic_pointer_cast<TableNode> (node))
        {
            float graphX = tableNode->getX() + panX + 8.0f;
            float graphY = tableNode->getY() + panY + 22.0f;
            float graphW = getNodeWidth (*tableNode) - 16.0f;
            float graphH = getNodeHeight (*tableNode) - 26.0f;

            if (mousePos.x >= graphX && mousePos.x <= graphX + graphW &&
                mousePos.y >= graphY && mousePos.y <= graphY + graphH)
            {
                float normX = std::clamp ((mousePos.x - graphX) / graphW, 0.0f, 1.0f);
                float normY = 1.0f - 2.0f * std::clamp ((mousePos.y - graphY) / graphH, 0.0f, 1.0f);
                tableNode->writeSampleNormalized (normX, normY);
                selectedNodeIds.clear();
                selectedNodeIds.insert (tableNode->getId());
                rebuildInspector();
                repaint();
                return;
            }
        }
        else if (auto bangNode = std::dynamic_pointer_cast<BangNode> (node))
        {
            float nx = bangNode->getX() + panX;
            float ny = bangNode->getY() + panY;
            float nw = getNodeWidth (*bangNode);
            float nh = getNodeHeight (*bangNode);
            if (mousePos.x >= nx && mousePos.x <= nx + nw &&
                mousePos.y >= ny && mousePos.y <= ny + nh)
            {
                bangNode->triggerBang();
                repaint();
            }
        }
        else if (auto bangAudioNode = std::dynamic_pointer_cast<BangAudioNode> (node))
        {
            float nx = bangAudioNode->getX() + panX;
            float ny = bangAudioNode->getY() + panY;
            float nw = getNodeWidth (*bangAudioNode);
            float nh = getNodeHeight (*bangAudioNode);
            if (mousePos.x >= nx && mousePos.x <= nx + nw &&
                mousePos.y >= ny && mousePos.y <= ny + nh)
            {
                bangAudioNode->triggerBang();
                repaint();
            }
        }
    }

    // 1. Check Outlet Click (Cable Creation)
    static auto getDistanceToBezier = [] (juce::Point<float> p1, juce::Point<float> p2, juce::Point<float> mousePos) -> float
    {
        float dy = std::max (30.0f, std::abs (p2.y - p1.y) * 0.5f);
        juce::Point<float> cp1 = { p1.x, p1.y + dy };
        juce::Point<float> cp2 = { p2.x, p2.y - dy };

        float minDist = 99999.0f;
        const int numSamples = 24;
        for (int i = 0; i <= numSamples; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(numSamples);
            float u = 1.0f - t;
            float x = u*u*u*p1.x + 3.0f*u*u*t*cp1.x + 3.0f*u*t*t*cp2.x + t*t*t*p2.x;
            float y = u*u*u*p1.y + 3.0f*u*u*t*cp1.y + 3.0f*u*t*t*cp2.y + t*t*t*p2.y;

            float dist = mousePos.getDistanceFrom ({ x, y });
            if (dist < minDist) minDist = dist;
        }
        return minDist;
    };

    for (const auto& node : nodeGraph.getNodes())
    {
        for (size_t i = 0; i < node->getOutlets().size(); ++i)
        {
            auto p = getOutletPos (*node, static_cast<int>(i));
            if (p.getDistanceFrom (mousePos) < 12.0f)
            {
                isDraggingCable = true;
                cableSrcNodeId = node->getId();
                cableSrcOutletIdx = static_cast<int>(i);
                cableDragPos = mousePos;
                selectedConnectionId = 0;
                return;
            }
        }
    }

    // 2. Check Cable Click (Re-plug or Select)
    for (const auto& conn : nodeGraph.getConnections())
    {
        auto srcNode = nodeGraph.getNodeById (conn.sourceNodeId);
        auto destNode = nodeGraph.getNodeById (conn.destNodeId);
        if (srcNode && destNode)
        {
            auto p1 = getOutletPos (*srcNode, conn.sourceOutletIdx);
            auto p2 = getInletPos (*destNode, conn.destInletIdx);

            if (p2.getDistanceFrom (mousePos) < 14.0f)
            {
                cableSrcNodeId = conn.sourceNodeId;
                cableSrcOutletIdx = conn.sourceOutletIdx;
                isDraggingCable = true;
                cableDragPos = mousePos;

                nodeGraph.pushUndoState();
                nodeGraph.removeConnection (conn.id);
                selectedConnectionId = 0;
                repaint();
                return;
            }

            float distToCurve = getDistanceToBezier (p1, p2, mousePos);
            if (distToCurve < 14.0f)
            {
                selectedConnectionId = conn.id;
                selectedNodeIds.clear();
                rebuildInspector();
                repaint();
                return;
            }
        }
    }

    // 3. Check Node Box Click
    for (auto it = nodeGraph.getNodes().rbegin(); it != nodeGraph.getNodes().rend(); ++it)
    {
        const auto& node = *it;
        float nx = node->getX() + panX;
        float ny = node->getY() + panY;
        float nw = getNodeWidth (*node);
        float nh = getNodeHeight (*node);

        if (mousePos.x >= nx && mousePos.x <= nx + nw &&
            mousePos.y >= ny && mousePos.y <= ny + nh)
        {
            if (isShift)
            {
                if (selectedNodeIds.count (node->getId()) > 0)
                    selectedNodeIds.erase (node->getId());
                else
                    selectedNodeIds.insert (node->getId());
            }
            else
            {
                if (selectedNodeIds.count (node->getId()) == 0)
                {
                    selectedNodeIds.clear();
                    selectedNodeIds.insert (node->getId());
                }
            }

            selectedConnectionId = 0;
            draggingNodeId = node->getId();
            dragOffset = { mousePos.x - nx, mousePos.y - ny };

            rebuildInspector();
            repaint();
            return;
        }
    }

    // 4. Empty Canvas Click -> Panel Panning OR Rubberband Selection (ONLY IF SHIFT IS HELD)
    if (isShift)
    {
        selectedConnectionId = 0;
        isMarqueeDragging = true;
        marqueeRect = { mousePos.x, mousePos.y, 0.0f, 0.0f };
    }
    else
    {
        selectedNodeIds.clear();
        selectedConnectionId = 0;
        isCanvasPanning = true;
        panStartPos = e.position;
        initialPanOffset = { panX, panY };
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);
        rebuildInspector();
    }
    repaint();
}

void RelativisticCanvasComponent::mouseDoubleClick (const juce::MouseEvent& e)
{
    juce::Point<float> mousePos = e.position;

    for (const auto& node : nodeGraph.getNodes())
    {
        float nx = node->getX() + panX;
        float ny = node->getY() + panY;
        float nw = getNodeWidth (*node);
        float nh = getNodeHeight (*node);

        if (mousePos.x >= nx && mousePos.x <= nx + nw &&
            mousePos.y >= ny && mousePos.y <= ny + nh)
        {
            if (auto numNode = std::dynamic_pointer_cast<NumberNode> (node))
            {
                auto alert = std::make_unique<juce::AlertWindow> ("SET NUMBER VALUE", "Enter new floating-point value for [number]:", juce::AlertWindow::QuestionIcon);
                alert->addTextEditor ("numVal", juce::String (numNode->getParameter ("value", 0.0f)), "Value:");
                alert->addButton ("OK", 1);
                alert->addButton ("Cancel", 0);
                alert->enterModalState (true, juce::ModalCallbackFunction::create ([this, numNode, a = alert.get()] (int res) {
                    if (res == 1)
                    {
                        float val = a->getTextEditorContents ("numVal").getFloatValue();
                        numNode->setParameter ("value", val);
                        rebuildInspector();
                        repaint();
                    }
                }), true);
                return;
            }
            if (auto seqNode = std::dynamic_pointer_cast<StepSequencerNode> (node))
            {
                auto alert = std::make_unique<juce::AlertWindow> ("EDIT SEQUENCER PATTERN", "Enter space-separated MIDI notes or pitches for [seq]:", juce::AlertWindow::QuestionIcon);
                alert->addTextEditor ("patStr", juce::String (seqNode->getPatternString()), "Pattern Notes:");
                alert->addButton ("OK", 1);
                alert->addButton ("Cancel", 0);
                alert->enterModalState (true, juce::ModalCallbackFunction::create ([this, seqNode, a = alert.get()] (int res) {
                    if (res == 1)
                    {
                        std::string pat = a->getTextEditorContents ("patStr").toStdString();
                        seqNode->setPatternString (pat);
                        rebuildInspector();
                        repaint();
                    }
                }), true);
                return;
            }


            selectedNodeIds.clear();
            selectedNodeIds.insert (node->getId());
            inlineLabelEditor.setBounds (static_cast<int>(nx + 8), static_cast<int>(ny + 8), static_cast<int>(nw - 16), 28);
            inlineLabelEditor.setText (node->getLabel(), false);
            inlineLabelEditor.setVisible (true);
            inlineLabelEditor.grabKeyboardFocus();
            inlineLabelEditor.selectAll();
            return;
        }
    }

    // Double-click empty canvas -> Open Pure Data-Style Object Search Menu!
    showObjectSearchMenu (mousePos);
}

void RelativisticCanvasComponent::mouseDrag (const juce::MouseEvent& e)
{
    juce::Point<float> mousePos = e.position;

    // Check Table Node Mouse Drag Graph Drawing
    if (selectedNodeIds.size() == 1)
    {
        auto n = nodeGraph.getNodeById (*selectedNodeIds.begin());
        if (auto tableNode = std::dynamic_pointer_cast<TableNode> (n))
        {
            float graphX = tableNode->getX() + panX + 8.0f;
            float graphY = tableNode->getY() + panY + 22.0f;
            float graphW = getNodeWidth (*tableNode) - 16.0f;
            float graphH = getNodeHeight (*tableNode) - 26.0f;

            if (e.mouseDownPosition.x >= graphX && e.mouseDownPosition.x <= graphX + graphW &&
                e.mouseDownPosition.y >= graphY && e.mouseDownPosition.y <= graphY + graphH)
            {
                float normX = std::clamp ((mousePos.x - graphX) / graphW, 0.0f, 1.0f);
                float normY = 1.0f - 2.0f * std::clamp ((mousePos.y - graphY) / graphH, 0.0f, 1.0f);
                tableNode->writeSampleNormalized (normX, normY);
                repaint();
                return;
            }
        }
        else if (auto outNode = std::dynamic_pointer_cast<OutNode> (n))
        {
            float graphX = outNode->getX() + panX + 8.0f;
            float graphY = outNode->getY() + panY + 22.0f;
            float graphW = getNodeWidth (*outNode) - 46.0f;
            float graphH = getNodeHeight (*outNode) - 26.0f;

            if (e.mouseDownPosition.x >= graphX && e.mouseDownPosition.x <= graphX + graphW &&
                e.mouseDownPosition.y >= graphY && e.mouseDownPosition.y <= graphY + graphH)
            {
                outNode->invokeMethod ("Toggle Scope Mode");
                rebuildInspector();
                repaint();
                return;
            }
        }
        else if (auto numNode = std::dynamic_pointer_cast<NumberNode> (n))
        {
            float currVal = numNode->getParameter ("value", 0.0f);
            float step = e.mods.isShiftDown() ? 0.1f : 1.0f;
            float newVal = currVal - (static_cast<float>(e.getDistanceFromDragStartY()) * 0.1f * step);
            numNode->setParameter ("value", newVal);
            repaint();
            return;
        }
    }

    if (isCanvasPanning)
    {
        panX = initialPanOffset.x + (e.position.x - panStartPos.x);
        panY = initialPanOffset.y + (e.position.y - panStartPos.y);
        repaint();
    }
    else if (isDraggingCable && isEditMode)
    {
        cableDragPos = e.position;
        repaint();
    }
    else if (isMarqueeDragging && isEditMode)
    {
        float x1 = std::min (e.mouseDownPosition.x, e.position.x);
        float y1 = std::min (e.mouseDownPosition.y, e.position.y);
        float w = std::abs (e.position.x - e.mouseDownPosition.x);
        float h = std::abs (e.position.y - e.mouseDownPosition.y);
        marqueeRect = { x1, y1, w, h };

        for (const auto& node : nodeGraph.getNodes())
        {
            juce::Rectangle<float> nodeRect (node->getX() + panX, node->getY() + panY, getNodeWidth (*node), getNodeHeight (*node));
            if (marqueeRect.intersects (nodeRect))
            {
                selectedNodeIds.insert (node->getId());
            }
        }
        repaint();
    }
    else if (draggingNodeId > 0 && isEditMode)
    {
        auto anchorNode = nodeGraph.getNodeById (draggingNodeId);
        if (anchorNode)
        {
            float targetX = (e.position.x - dragOffset.x - panX);
            float targetY = (e.position.y - dragOffset.y - panY);

            if (snapToGrid)
            {
                targetX = std::round (targetX / gridSize) * gridSize;
                targetY = std::round (targetY / gridSize) * gridSize;
            }

            float dx = targetX - anchorNode->getX();
            float dy = targetY - anchorNode->getY();

            for (int id : selectedNodeIds)
            {
                auto n = nodeGraph.getNodeById (id);
                if (n) n->setPosition (n->getX() + dx, n->getY() + dy);
            }
            repaint();
        }
    }
}

void RelativisticCanvasComponent::mouseUp (const juce::MouseEvent& e)
{
    if (isCanvasPanning)
    {
        isCanvasPanning = false;
        setMouseCursor (juce::MouseCursor::NormalCursor);
    }
    if (isDraggingCable)
    {
        juce::Point<float> mousePos = e.position;

        for (const auto& node : nodeGraph.getNodes())
        {
            for (size_t i = 0; i < node->getInlets().size(); ++i)
            {
                auto p = getInletPos (*node, static_cast<int>(i));
                if (p.getDistanceFrom (mousePos) < 14.0f)
                {
                    nodeGraph.pushUndoState();
                    nodeGraph.addConnection (cableSrcNodeId, cableSrcOutletIdx, node->getId(), static_cast<int>(i));
                    break;
                }
            }
        }

        isDraggingCable = false;
        cableSrcNodeId = 0;
        repaint();
    }
    else if (isMarqueeDragging)
    {
        isMarqueeDragging = false;
        repaint();
    }

    draggingNodeId = 0;
}

void RelativisticCanvasComponent::mouseMove (const juce::MouseEvent& e)
{
    juce::Point<float> mousePos = e.position;
    HoveredPortInfo newHover;

    for (const auto& node : nodeGraph.getNodes())
    {
        for (size_t i = 0; i < node->getInlets().size(); ++i)
        {
            auto p = getInletPos (*node, static_cast<int>(i));
            if (p.getDistanceFrom (mousePos) < 12.0f)
            {
                newHover.nodeId = node->getId();
                newHover.portIdx = static_cast<int>(i);
                newHover.isInlet = true;
                newHover.pos = p;

                const auto& port = node->getInlets()[i];
                newHover.portName = port.name;

                if (port.audioData.getNumSamples() > 0 && port.audioData.getMagnitude (0, port.audioData.getNumSamples()) > 0.0001f)
                {
                    newHover.signalTypeName = "AUDIO RATE (CYAN)";
                    float mag = port.audioData.getMagnitude (0, port.audioData.getNumSamples());
                    newHover.routedValueText = "Audio Peak: " + juce::String (mag, 3).toStdString();
                }
                else if (port.type == NodePortType::Time)
                {
                    newHover.signalTypeName = "TIME DILATION (PURPLE)";
                    newHover.routedValueText = "Gamma: " + juce::String (port.timeGamma, 2).toStdString() + "x";
                }
                else
                {
                    newHover.signalTypeName = "CONTROL RATE (AMBER)";
                    newHover.routedValueText = "Val: " + juce::String (port.controlValue, 3).toStdString();
                }
                break;
            }
        }

        if (newHover.nodeId != 0) break;

        for (size_t i = 0; i < node->getOutlets().size(); ++i)
        {
            auto p = getOutletPos (*node, static_cast<int>(i));
            if (p.getDistanceFrom (mousePos) < 12.0f)
            {
                newHover.nodeId = node->getId();
                newHover.portIdx = static_cast<int>(i);
                newHover.isInlet = false;
                newHover.pos = p;

                const auto& port = node->getOutlets()[i];
                newHover.portName = port.name;

                if (port.type == NodePortType::Audio)
                {
                    newHover.signalTypeName = "AUDIO RATE (CYAN)";
                    float mag = port.audioData.getMagnitude (0, port.audioData.getNumSamples());
                    newHover.routedValueText = "Audio Peak: " + juce::String (mag, 3).toStdString();
                }
                else if (port.type == NodePortType::Time)
                {
                    newHover.signalTypeName = "TIME DILATION (PURPLE)";
                    newHover.routedValueText = "Gamma: " + juce::String (port.timeGamma, 2).toStdString() + "x";
                }
                else
                {
                    newHover.signalTypeName = "CONTROL RATE (AMBER)";
                    newHover.routedValueText = "Val: " + juce::String (port.controlValue, 3).toStdString();
                }
                break;
            }
        }
    }

    if (newHover.nodeId != hoveredPort.nodeId || newHover.portIdx != hoveredPort.portIdx || newHover.isInlet != hoveredPort.isInlet)
    {
        hoveredPort = newHover;
        repaint();
    }
}

void RelativisticCanvasComponent::drawCable (juce::Graphics& g, juce::Point<float> p1, juce::Point<float> p2, NodePortType type, bool isFeedbackLoop)
{
    juce::Colour cableColour = juce::Colour (0xff06b6d4); // Audio = Cyan
    if (type == NodePortType::Time)    cableColour = juce::Colour (0xffa855f7); // Time = Purple
    if (type == NodePortType::Control) cableColour = juce::Colour (0xfff59e0b); // Control = Amber
    if (isFeedbackLoop)                cableColour = juce::Colour (0xffef4444); // Feedback Warning = Neon Red

    if (cableStyle == CableStyle::Straight)
    {
        // Drop Shadow
        g.setColour (juce::Colour (0x66000000));
        g.drawLine (p1.x + 2.0f, p1.y + 3.0f, p2.x + 2.0f, p2.y + 3.0f, 4.0f);

        // Core Cable
        g.setColour (cableColour);
        g.drawLine (p1.x, p1.y, p2.x, p2.y, isFeedbackLoop ? 3.5f : 2.5f);
        return;
    }

    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;

    juce::Path path;
    path.startNewSubPath (p1);

    if (cableStyle == CableStyle::Organic)
    {
        // Natural Gravity Sag Control Points
        float sagFactor = std::max (50.0f, std::abs (dy) * 0.65f);
        float bowFactor = (dy < 0.0f) ? (dx >= 0.0f ? 90.0f : -90.0f) : (dx * 0.15f);

        juce::Point<float> c1 { p1.x + bowFactor, p1.y + sagFactor };
        juce::Point<float> c2 { p2.x - bowFactor, p2.y - sagFactor };

        path.cubicTo (c1, c2, p2);
    }
    else // SmoothS
    {
        float deltaY = std::abs (dy) * 0.5f + 30.0f;
        path.cubicTo (p1.x, p1.y + deltaY, p2.x, p2.y - deltaY, p2.x, p2.y);
    }

    // 1. Render Drop Shadow
    juce::Path shadowPath = path;
    shadowPath.applyTransform (juce::AffineTransform::translation (2.0f, 3.0f));
    g.setColour (juce::Colour (0x55000000));
    g.strokePath (shadowPath, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Render Anti-Aliased Core Cable
    g.setColour (cableColour);
    g.strokePath (path, juce::PathStrokeType (isFeedbackLoop ? 3.5f : 2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 3. Render Feedback Warning Badge on Cable Midpoint (only on return cable to prevent duplicate overlapping badges)
    if (isFeedbackLoop && (p1.x > p2.x || p1.y > p2.y))
    {
        juce::Point<float> mid = path.getPointAlongPath (path.getLength() * 0.5f);
        g.setColour (juce::Colour (0xff7f1d1d));
        g.fillRoundedRectangle (mid.x - 70.0f, mid.y - 10.0f, 140.0f, 20.0f, 4.0f);
        g.setColour (juce::Colour (0xffef4444));
        g.drawRoundedRectangle (mid.x - 70.0f, mid.y - 10.0f, 140.0f, 20.0f, 4.0f, 1.0f);
        g.setColour (juce::Colour (0xfff8fafc));
        g.setFont (FontManager::getInstance().getOxaniumFont (11.0f, true));
        g.drawText ("[FEEDBACK: 1-SMP DELAY]", mid.x - 70.0f, mid.y - 10.0f, 140.0f, 20.0f, juce::Justification::centred);
    }
}

void RelativisticCanvasComponent::drawNode (juce::Graphics& g, const std::shared_ptr<RelativisticNode>& node)
{
    const float x = node->getX() + panX;
    const float y = node->getY() + panY;
    const float w = getNodeWidth (*node);
    const float h = getNodeHeight (*node);

    bool isTimeObj = node->getTypeName().rfind ("time.", 0) == 0;
    bool isAudioObj = node->getTypeName().find ("~") != std::string::npos || node->getTypeName() == "dac~" || node->getTypeName() == "gain~" || node->getTypeName() == "out~";

    juce::Colour typeBadgeCol = isTimeObj ? juce::Colour (0xff8b5cf6) : (isAudioObj ? juce::Colour (0xff06b6d4) : juce::Colour (0xfff59e0b));

    // PlugData Slim Rounded Card Body & Clip Path
    juce::Path cardPath;
    cardPath.addRoundedRectangle (x, y, w, h, 6.0f);

    g.setColour (juce::Colour (0xff181825));
    g.fillPath (cardPath);

    // Left Type Indicator Strip (Clipped inside rounded corners)
    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (cardPath);
        g.setColour (typeBadgeCol);
        g.fillRect (x, y, 4.0f, h);
    }

    // Selection Halo & Border
    if (selectedNodeIds.count (node->getId()) > 0)
    {
        g.setColour (juce::Colour (0xff38bdf8)); // Glowing Cyan Outline
        g.drawRoundedRectangle (x - 1.5f, y - 1.5f, w + 3.0f, h + 3.0f, 7.0f, 2.0f);
    }
    else
    {
        g.setColour (juce::Colour (0xff2e2e42)); // Hairline Dark Slate Border
        g.drawRoundedRectangle (x, y, w, h, 6.0f, 1.0f);
    }

    // Render [bang] Control Trigger LED Button
    if (node->getTypeName() == "bang" || node->getTypeName() == "b")
    {
        float cx = x + w - 24.0f;
        float cy = y + h * 0.5f;
        g.setColour (juce::Colour (0xff0f172a));
        g.fillEllipse (cx - 10.0f, cy - 10.0f, 20.0f, 20.0f);
        g.setColour (juce::Colour (0xfff59e0b));
        g.drawEllipse (cx - 10.0f, cy - 10.0f, 20.0f, 20.0f, 1.5f);
        g.setColour (juce::Colour (0xfff59e0b).withAlpha (0.9f));
        g.fillEllipse (cx - 6.0f, cy - 6.0f, 12.0f, 12.0f);
    }

    // Render [bang~] Audio Impulse Spike LED Ring
    if (node->getTypeName() == "bang~" || node->getTypeName() == "b~")
    {
        float cx = x + w - 24.0f;
        float cy = y + h * 0.5f;
        g.setColour (juce::Colour (0xff0f172a));
        g.fillEllipse (cx - 10.0f, cy - 10.0f, 20.0f, 20.0f);
        g.setColour (juce::Colour (0xff06b6d4));
        g.drawEllipse (cx - 10.0f, cy - 10.0f, 20.0f, 20.0f, 1.5f);
        g.setColour (juce::Colour (0xff06b6d4).withAlpha (0.9f));
        g.fillEllipse (cx - 6.0f, cy - 6.0f, 12.0f, 12.0f);
    }

    // Special Canvas Visualization for [time.transport] Realtime Beat & Status Display + Beat Flash LED
    auto transportNode = std::dynamic_pointer_cast<TimeTransportNode> (node);
    if (transportNode)
    {
        double beats = transportNode->getCurrentBeatPosition();
        int bar = static_cast<int>(std::floor (beats / 4.0)) + 1;
        double beatInBar = std::fmod (beats, 4.0) + 1.0;
        bool playing = transportNode->getIsPlaying();
        bool flashing = transportNode->getIsBeatFlashing();

        float ledX = x + w - 20.0f;
        float ledY = y + h * 0.5f;
        g.setColour (juce::Colour (0xff070a12));
        g.fillEllipse (ledX - 8.0f, ledY - 8.0f, 16.0f, 16.0f);
        
        juce::Colour ledCol = flashing ? juce::Colour (0xfff59e0b) : (playing ? juce::Colour (0xff059669) : juce::Colour (0xff334155));
        g.setColour (ledCol);
        g.fillEllipse (ledX - 5.0f, ledY - 5.0f, 10.0f, 10.0f);
        if (flashing)
        {
            g.setColour (juce::Colours::white);
            g.drawEllipse (ledX - 8.0f, ledY - 8.0f, 16.0f, 16.0f, 2.0f);
        }

        g.setFont (FontManager::getInstance().getOxaniumFont (10.5f, true));
        g.setColour (playing ? juce::Colour (0xff38bdf8) : juce::Colour (0xff94a3b8));
        juce::String posStr = juce::String::formatted ("Bar %d : Beat %.1f", bar, beatInBar);
        g.drawText (posStr, x + 10.0f, y + 26.0f, w - 36.0f, 16.0f, juce::Justification::centredLeft);
    }

    // Special Canvas Visualization for [time.scope] Live Telemetry & Kinetic Relativistic Gauge
    auto timeScope = std::dynamic_pointer_cast<TimeScopeNode> (node);
    if (timeScope)
    {
        float gamma = timeScope->getMonitoredGamma();
        double tSec = timeScope->getMonitoredTimeSec();

        // 1. Text Telemetry Display
        g.setColour (juce::Colour (0xff06b6d4));
        g.setFont (FontManager::getInstance().getOxaniumFont (11.0f, true));
        juce::String gammaStr = juce::String::formatted (juce::CharPointer_UTF8 ("\xce\xb3: %+.2fx (%d%%)"), gamma, static_cast<int>(gamma * 100.0f));
        g.drawText (gammaStr, x + 10.0f, y + 20.0f, w - 16.0f, 15.0f, juce::Justification::centredLeft);

        g.setColour (juce::Colour (0xfff59e0b));
        juce::String timeStr = juce::String::formatted ("t_loc: %.3fs", tSec);
        g.drawText (timeStr, x + 10.0f, y + 35.0f, w - 16.0f, 15.0f, juce::Justification::centredLeft);

        // Live Signal History Waveform Trace Box
        float graphX = x + 8.0f;
        float graphY = y + 52.0f;
        float graphW = w - 16.0f;
        float graphH = 32.0f;

        g.setColour (juce::Colour (0xff070a12));
        g.fillRoundedRectangle (graphX, graphY, graphW, graphH, 2.0f);
        g.setColour (juce::Colour (0xff1e293b));
        g.drawRoundedRectangle (graphX, graphY, graphW, graphH, 2.0f, 1.0f);

        const auto& hist = timeScope->getSignalHistory();
        if (!hist.empty())
        {
            juce::Path p;
            float midY = graphY + graphH * 0.5f;
            for (size_t i = 0; i < hist.size(); ++i)
            {
                float px = graphX + (static_cast<float>(i) / static_cast<float>(hist.size() - 1)) * graphW;
                float py = midY - std::clamp (hist[i], -2.0f, 2.0f) * (graphH * 0.4f);
                if (i == 0) p.startNewSubPath (px, py);
                else p.lineTo (px, py);
            }
            g.setColour (juce::Colour (0xff06b6d4));
            g.strokePath (p, juce::PathStrokeType (1.2f));
        }

        // Kinetic Relativistic Speed Gauge Bar
        float gx = x + 8.0f;
        float gy = y + 88.0f;
        float gw = w - 16.0f;
        float gh = 12.0f;

        g.setColour (juce::Colour (0xff070a12));
        g.fillRoundedRectangle (gx, gy, gw, gh, 2.0f);
        g.setColour (juce::Colour (0xff1e293b));
        g.drawRoundedRectangle (gx, gy, gw, gh, 2.0f, 1.0f);

        float midX = gx + gw * 0.5f;
        float normGamma = std::clamp (gamma / 5.0f, -1.0f, 1.0f);
        float barLen = normGamma * (gw * 0.48f);

        if (gamma > 0.001f)
        {
            g.setColour (juce::Colour (0xff06b6d4));
            g.fillRect (midX, gy + 2.0f, barLen, gh - 4.0f);
        }
        else if (gamma < -0.001f)
        {
            g.setColour (juce::Colour (0xfff59e0b));
            g.fillRect (midX + barLen, gy + 2.0f, -barLen, gh - 4.0f);
        }
        else
        {
            g.setColour (juce::Colour (0xff8b5cf6));
            g.fillRect (midX - 3.0f, gy + 2.0f, 6.0f, gh - 4.0f);
        }
    }

    // Special Canvas Visualization for [table] Interactive Waveform / Step Sequencer Canvas

    auto tableNode = std::dynamic_pointer_cast<TableNode> (node);
    if (tableNode)
    {
        float graphX = x + 8.0f;
        float graphY = y + 22.0f;
        float graphW = w - 16.0f;
        float graphH = h - 26.0f;

        g.setColour (juce::Colour (0xff070a12));
        g.fillRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f);
        g.setColour (juce::Colour (0xff1e293b));
        g.drawRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f, 1.0f);

        const auto& data = tableNode->getTableData();
        int dataSize = static_cast<int>(data.size());
        if (dataSize > 0)
        {
            juce::Path wavePath;
            float midY = graphY + graphH * 0.5f;

            if (dataSize <= 16) // Step Bar Mode
            {
                float stepW = graphW / static_cast<float>(dataSize);
                for (int i = 0; i < dataSize; ++i)
                {
                    float val = data[i];
                    float normVal = (val > 1.5f) ? (val / 127.0f) : val;
                    float barH = (normVal * 0.5f) * graphH;
                    float bx = graphX + i * stepW;

                    g.setColour (juce::Colour (0xff06b6d4).withAlpha (0.85f));
                    if (normVal < 0.0f)
                        g.fillRect (bx + 1.0f, midY, stepW - 2.0f, -barH);
                    else
                        g.fillRect (bx + 1.0f, midY - barH, stepW - 2.0f, barH);
                }
            }
            else // Continuous Waveform Mode
            {
                int sampleStep = std::max (1, dataSize / 64);
                for (int i = 0; i < 64; ++i)
                {
                    int sampleIdx = std::min (dataSize - 1, i * sampleStep);
                    float val = std::clamp (data[sampleIdx], -1.0f, 1.0f);
                    float px = graphX + (static_cast<float>(i) / 63.0f) * graphW;
                    float py = midY - val * (graphH * 0.45f);

                    if (i == 0) wavePath.startNewSubPath (px, py);
                    else        wavePath.lineTo (px, py);
                }

                g.setColour (juce::Colour (0xff06b6d4)); // Cyber Cyan
                g.strokePath (wavePath, juce::PathStrokeType (1.5f));
            }
        }
    }

    // Special Canvas Visualization for [out~] Live VU RMS Meters & Oscilloscope Screen
    auto outNode = std::dynamic_pointer_cast<OutNode> (node);
    if (outNode)
    {
        float graphX = x + 8.0f;
        float graphY = y + 22.0f;
        float graphW = w - 46.0f;
        float graphH = h - 26.0f;

        // Dark Scope Screen
        g.setColour (juce::Colour (0xff050811));
        g.fillRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f);
        g.setColour (juce::Colour (0xff1e293b));
        g.drawRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f, 1.0f);

        // Center reticle crosshair
        g.setColour (juce::Colour (0xff1e293b));
        g.drawHorizontalLine (static_cast<int>(graphY + graphH * 0.5f), graphX, graphX + graphW);
        g.drawVerticalLine (static_cast<int>(graphX + graphW * 0.5f), graphY, graphY + graphH);

        const auto& scopeL = outNode->getScopeL();
        const auto& scopeR = outNode->getScopeR();
        int writeIdx = outNode->getScopeWriteIndex();

        float displayMode = outNode->getParameter ("displayMode", 0.0f);

        if (!scopeL.empty() && !scopeR.empty())
        {
            if (displayMode < 0.5f)
            {
                // Mode 0: Dual Trace Time Domain Scope (Left Cyan, Right Gold)
                juce::Path pathL, pathR;
                int total = static_cast<int>(scopeL.size());
                float midY = graphY + graphH * 0.5f;

                for (int i = 0; i < 64; ++i)
                {
                    int idx = (writeIdx + i * (total / 64)) % total;
                    float sampleL = std::clamp (scopeL[idx], -1.2f, 1.2f);
                    float sampleR = std::clamp (scopeR[idx], -1.2f, 1.2f);

                    float px = graphX + (static_cast<float>(i) / 63.0f) * graphW;
                    float pyL = midY - sampleL * (graphH * 0.42f);
                    float pyR = midY - sampleR * (graphH * 0.42f);

                    if (i == 0) { pathL.startNewSubPath (px, pyL); pathR.startNewSubPath (px, pyR); }
                    else        { pathL.lineTo (px, pyL);           pathR.lineTo (px, pyR); }
                }

                g.setColour (juce::Colour (0xff06b6d4)); // Cyber Cyan Left Channel
                g.strokePath (pathL, juce::PathStrokeType (1.5f));

                g.setColour (juce::Colour (0xfff59e0b)); // Relativistic Gold Right Channel
                g.strokePath (pathR, juce::PathStrokeType (1.2f));
            }
            else
            {
                // Mode 1: X-Y Lissajous Phase Plot Scope (Royal Violet)
                juce::Path pathXY;
                int total = static_cast<int>(scopeL.size());
                float centerX = graphX + graphW * 0.5f;
                float centerY = graphY + graphH * 0.5f;
                float scaleX = graphW * 0.42f;
                float scaleY = graphH * 0.42f;

                for (int i = 0; i < 64; ++i)
                {
                    int idx = (writeIdx + i * (total / 64)) % total;
                    float sampleL = std::clamp (scopeL[idx], -1.2f, 1.2f);
                    float sampleR = std::clamp (scopeR[idx], -1.2f, 1.2f);

                    float px = centerX + sampleL * scaleX;
                    float py = centerY - sampleR * scaleY;

                    if (i == 0) pathXY.startNewSubPath (px, py);
                    else        pathXY.lineTo (px, py);
                }

                g.setColour (juce::Colour (0xffa855f7)); // Royal Violet Lissajous Scope
                g.strokePath (pathXY, juce::PathStrokeType (1.5f));
            }
        }

        // Live VU RMS Meters on the right edge
        float meterX = x + w - 32.0f;
        float meterY = y + 22.0f;
        float meterW = 10.0f;
        float meterH = graphH;

        g.setColour (juce::Colour (0xff0b0f19));
        g.fillRoundedRectangle (meterX, meterY, meterW * 2.2f, meterH, 2.0f);

        float rmsL = std::clamp (outNode->getRmsL(), 0.0f, 1.2f);
        float rmsR = std::clamp (outNode->getRmsR(), 0.0f, 1.2f);

        auto drawBar = [&] (float bx, float level) {
            float barH = level * meterH;
            juce::Colour barColor = juce::Colour (0xff22c55e);
            if (level > 0.85f) barColor = juce::Colour (0xffeab308);
            if (level >= 1.0f) barColor = juce::Colour (0xffef4444);

            g.setColour (barColor);
            g.fillRect (bx, meterY + (meterH - barH), meterW - 1.0f, barH);
        };

        drawBar (meterX, rmsL);
        drawBar (meterX + meterW, rmsR);

        if (outNode->isRecordingActive())
        {
            g.setColour (juce::Colours::red.withAlpha (0.9f));
            g.fillEllipse (graphX + 6.0f, graphY + 6.0f, 8.0f, 8.0f);
            g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, true));
            g.drawText ("REC (/tmp)", graphX + 16.0f, graphY + 4.0f, 60.0f, 12.0f, juce::Justification::centredLeft);
        }
    }

    // Title Text in Sci-Fi Oxanium Font (No Truncation)
    g.setColour (juce::Colour (0xfff8fafc));
    g.setFont (FontManager::getInstance().getOxaniumFont (14.0f, true));
    float labelTextW = outNode ? w - 40.0f : w - 12.0f;
    g.drawText (node->getLabel(), x + 10, y, labelTextW, h, juce::Justification::centredLeft);

    // Inlets (Top Edge Dots with Smart Spaced Labels)
    juce::Font portFont = FontManager::getInstance().getOxaniumFont (9.5f, false);
    g.setFont (portFont);

    for (size_t i = 0; i < node->getInlets().size(); ++i)
    {
        auto p = getInletPos (*node, static_cast<int>(i));
        const auto& port = node->getInlets()[i];
        NodePortType type = port.type;

        bool hasAudioInput = (port.audioData.getNumSamples() > 0 && port.audioData.getMagnitude (0, port.audioData.getNumSamples()) > 0.0001f);

        juce::Colour portCol = (hasAudioInput || type == NodePortType::Audio) ? juce::Colour (0xff06b6d4) : (type == NodePortType::Time ? juce::Colour (0xff8b5cf6) : juce::Colour (0xfff59e0b));

        g.setColour (juce::Colour (0xff181825));
        g.fillEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f);
        g.setColour (portCol);
        g.fillEllipse (p.x - 2.5f, p.y - 2.5f, 5.0f, 5.0f);
        g.setColour (juce::Colours::white);
        g.drawEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f, 1.0f);

        // Smart Port Name Label without Truncation
        float lw = std::max (44.0f, static_cast<float>(portFont.getStringWidth (port.name)) + 6.0f);
        g.setColour (portCol.withAlpha (0.95f));
        g.drawText (port.name, p.x - lw * 0.5f, p.y + 3.0f, lw, 11.0f, juce::Justification::centred);
    }

    // Outlets (Bottom Edge Dots with Smart Spaced Labels)
    for (size_t i = 0; i < node->getOutlets().size(); ++i)
    {
        auto p = getOutletPos (*node, static_cast<int>(i));
        const auto& port = node->getOutlets()[i];
        NodePortType type = port.type;

        juce::Colour portCol = (type == NodePortType::Audio) ? juce::Colour (0xff06b6d4) : (type == NodePortType::Time ? juce::Colour (0xff8b5cf6) : juce::Colour (0xfff59e0b));

        g.setColour (juce::Colour (0xff181825));
        g.fillEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f);
        g.setColour (portCol);
        g.fillEllipse (p.x - 2.5f, p.y - 2.5f, 5.0f, 5.0f);
        g.setColour (juce::Colours::white);
        g.drawEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f, 1.0f);

        // Smart Port Name Label without Truncation
        float lw = std::max (44.0f, static_cast<float>(portFont.getStringWidth (port.name)) + 6.0f);
        g.setColour (portCol.withAlpha (0.95f));
        g.drawText (port.name, p.x - lw * 0.5f, p.y - 14.0f, lw, 11.0f, juce::Justification::centred);
    }
}

void RelativisticCanvasComponent::paint (juce::Graphics& g)
{
    // Dark Carbon Canvas Background
    g.fillAll (juce::Colour (0xff070a12));

    // Sci-Fi Micro-Grid Dot Matrix
    if (showGrid)
    {
        g.setColour (juce::Colour (0x1a94a3b8));
        float step = gridSize;
        int startX = static_cast<int>(std::fmod (panX, step));
        if (startX < 0) startX += static_cast<int>(step);
        int startY = static_cast<int>(std::fmod (panY, step)) + 55;
        if (startY < 55) startY += static_cast<int>(step);

        for (int gx = startX; gx < getWidth(); gx += static_cast<int>(step))
        {
            for (int gy = startY; gy < getHeight(); gy += static_cast<int>(step))
            {
                g.fillEllipse (static_cast<float>(gx), static_cast<float>(gy), 1.5f, 1.5f);
            }
        }
    }

    // Top Header Toolbar Panel
    g.setColour (juce::Colour (0xff0f172a));
    g.fillRect (0, 0, getWidth(), 45);
    g.setColour (juce::Colour (0xff1e293b));
    g.drawHorizontalLine (45, 0.0f, static_cast<float>(getWidth()));

    // Right Side Dual Inspector Panel Background
    const float inspectorW = 320.0f;
    const float inspectorX = getWidth() - inspectorW;
    g.setColour (juce::Colour (0xff0d1322));
    g.fillRect (inspectorX, 45.0f, inspectorW, getHeight() - 95.0f);
    g.setColour (juce::Colour (0xff1e293b));
    g.drawVerticalLine (static_cast<int>(inspectorX), 45.0f, static_cast<float>(getHeight() - 50));

    for (const auto& conn : nodeGraph.getConnections())
    {
        auto srcNode = nodeGraph.getNodeById (conn.sourceNodeId);
        auto destNode = nodeGraph.getNodeById (conn.destNodeId);

        if (srcNode && destNode)
        {
            auto p1 = getOutletPos (*srcNode, conn.sourceOutletIdx);
            auto p2 = getInletPos (*destNode, conn.destInletIdx);
            NodePortType type = srcNode->getOutlets()[conn.sourceOutletIdx].type;

            drawCable (g, p1, p2, type, conn.isFeedbackLoop);

            if (conn.id == selectedConnectionId)
            {
                g.setColour (juce::Colour (0xfff59e0b)); // Glowing Gold Cable Selection
                g.drawEllipse (p1.x - 7.0f, p1.y - 7.0f, 14.0f, 14.0f, 2.0f);
                g.drawEllipse (p2.x - 7.0f, p2.y - 7.0f, 14.0f, 14.0f, 2.0f);
            }
        }
    }

    if (isDraggingCable)
    {
        auto srcNode = nodeGraph.getNodeById (cableSrcNodeId);
        if (srcNode)
        {
            auto p1 = getOutletPos (*srcNode, cableSrcOutletIdx);
            NodePortType type = srcNode->getOutlets()[cableSrcOutletIdx].type;
            drawCable (g, p1, cableDragPos, type);
        }
    }

    for (const auto& node : nodeGraph.getNodes())
    {
        drawNode (g, node);
    }

    // Draw Rubberband Marquee Selection Box
    if (isMarqueeDragging)
    {
        g.setColour (juce::Colour (0x3306b6d4)); // Translucent Cyan Fill
        g.fillRect (marqueeRect);
        g.setColour (juce::Colour (0xff06b6d4)); // Cyan Stroke
        g.drawRect (marqueeRect, 1.5f);
    }

    // Render Floating Hover Tooltip Badge for Ports
    if (hoveredPort.nodeId > 0)
    {
        float tipX = std::min (hoveredPort.pos.x + 12.0f, static_cast<float>(getWidth() - 200));
        float tipY = std::max (10.0f, hoveredPort.pos.y - 45.0f);
        float tipW = 180.0f;
        float tipH = 40.0f;

        g.setColour (juce::Colour (0xee0b1322));
        g.fillRoundedRectangle (tipX, tipY, tipW, tipH, 5.0f);
        g.setColour (juce::Colour (0xff38bdf8));
        g.drawRoundedRectangle (tipX, tipY, tipW, tipH, 5.0f, 1.0f);

        g.setColour (juce::Colour (0xfff8fafc));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ((hoveredPort.isInlet ? "INLET: " : "OUTLET: ") + hoveredPort.portName, tipX + 8, tipY + 4, tipW - 16, 14, juce::Justification::left);

        g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
        g.setColour (juce::Colour (0xff38bdf8));
        g.drawText (hoveredPort.signalTypeName + " | " + hoveredPort.routedValueText, tipX + 8, tipY + 20, tipW - 16, 14, juce::Justification::left);
    }

    // Draw Cmd-1 Draft Object Box Autocomplete / Invalid Alert Overlay
    if (isEditingDraftObject)
    {
        float boxX = draftObjectCanvasPos.x + panX;
        float boxY = draftObjectCanvasPos.y + panY;
        float boxH = 34.0f;

        if (isDraftObjectInvalid)
        {
            // Invalid Object Warning Tag Below Box
            float alertY = boxY + boxH + 4.0f;
            float alertW = 220.0f;
            float alertH = 24.0f;

            g.setColour (juce::Colour (0xff450a0a)); // Dark Red pill
            g.fillRoundedRectangle (boxX, alertY, alertW, alertH, 4.0f);
            g.setColour (juce::Colour (0xffef4444)); // Neon Red outline
            g.drawRoundedRectangle (boxX, alertY, alertW, alertH, 4.0f, 1.0f);

            g.setColour (juce::Colour (0xfffca5a5)); // Bright Red text
            g.setFont (FontManager::getInstance().getOxaniumFont (11.5f, true));
            g.drawText ("! NO SUCH OBJECT AVAILABLE", boxX + 8, alertY, alertW - 16, alertH, juce::Justification::centredLeft);
        }
        else if (draftObjectEditor && draftObjectEditor->hasKeyboardFocus (true) && !filteredAutocompleteItems.empty())
        {
            // Floating Autocomplete Menu Below Box
            float autoX = boxX;
            float autoY = boxY + boxH + 4.0f;
            float autoW = 320.0f;
            int maxShow = std::min (6, static_cast<int>(filteredAutocompleteItems.size()));
            float autoH = maxShow * 26.0f + 8.0f;

            g.setColour (juce::Colour (0xff090d16)); // Deep Carbon Popup
            g.fillRoundedRectangle (autoX, autoY, autoW, autoH, 6.0f);
            g.setColour (juce::Colour (0xff1e293b));
            g.drawRoundedRectangle (autoX, autoY, autoW, autoH, 6.0f, 1.0f);

            for (int i = 0; i < maxShow; ++i)
            {
                float itemY = autoY + 4.0f + i * 26.0f;
                bool isSelected = (i == selectedAutocompleteIdx);

                if (isSelected)
                {
                    g.setColour (juce::Colour (0xff164e63)); // Cyber Cyan Selection Card
                    g.fillRoundedRectangle (autoX + 4.0f, itemY, autoW - 8.0f, 24.0f, 4.0f);
                    g.setColour (juce::Colour (0xff06b6d4));
                    g.drawRoundedRectangle (autoX + 4.0f, itemY, autoW - 8.0f, 24.0f, 4.0f, 1.0f);
                }

                const auto& item = filteredAutocompleteItems[i];

                // Type Name in Cyber Cyan / Royal White
                g.setColour (isSelected ? juce::Colour (0xff38bdf8) : juce::Colour (0xff06b6d4));
                g.setFont (FontManager::getInstance().getOxaniumFont (13.0f, true));
                g.drawText ("[" + item.typeName + "]", autoX + 10, itemY, 110, 24, juce::Justification::centredLeft);

                // Category Tag
                g.setColour (juce::Colour (0xfff59e0b)); // Relativistic Gold
                g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, true));
                g.drawText (item.category, autoX + 120, itemY, 40, 24, juce::Justification::centredLeft);

                // Description
                g.setColour (isSelected ? juce::Colour (0xfff8fafc) : juce::Colour (0xff94a3b8));
                g.setFont (FontManager::getInstance().getOxaniumFont (10.5f, false));
                g.drawText (item.description, autoX + 165, itemY, autoW - 170, 24, juce::Justification::centredLeft);
            }
        }
    }
}

void RelativisticCanvasComponent::resized()
{
    // Top Menu Bar Layout
    int menuX = 15;
    int menuY = 6;
    int menuW = 55;
    int menuH = 32;

    btnMenuFile.setBounds    (menuX,                         menuY, menuW, menuH);
    btnMenuEdit.setBounds    (menuX + menuW + 4,             menuY, menuW, menuH);
    btnMenuView.setBounds    (menuX + (menuW + 4) * 2,       menuY, menuW, menuH);
    btnMenuObjects.setBounds (menuX + (menuW + 4) * 3,       menuY, menuW + 15, menuH);
    btnMenuAudio.setBounds   (menuX + (menuW + 4) * 3 + menuW + 20, menuY, menuW + 5, menuH);
    btnMenuHelp.setBounds    (menuX + (menuW + 4) * 3 + (menuW + 20) * 2, menuY, menuW + 5, menuH);

    // Right-Aligned Top Header Control Buttons
    btnAudioPower.setBounds (getWidth() - 165, 6, 150, 32);
    btnSavePatch.setBounds (getWidth() - 250, 6, 80, 32);
    btnLoadPatch.setBounds (getWidth() - 335, 6, 80, 32);
    btnRemoveCable.setBounds (getWidth() - 410, 6, 70, 32);

    // Bottom Palette Toolbar (Streamlined Pure Data-Style)
    const float paletteY = getHeight() - 48.0f;
    const float paletteH = 36.0f;

    btnAddObject.setBounds (12, static_cast<int>(paletteY), 160, static_cast<int>(paletteH));
    btnToggleCord.setBounds (180, static_cast<int>(paletteY), 140, static_cast<int>(paletteH));
    btnClear.setBounds (328, static_cast<int>(paletteY), 110, static_cast<int>(paletteH));
    btnHorizonReadout.setBounds (446, static_cast<int>(paletteY), 145, static_cast<int>(paletteH));
    btnResetHorizon.setBounds (599, static_cast<int>(paletteY), 135, static_cast<int>(paletteH));

    // Right Side Dual Inspector Bounds
    const float inspectorW = 320.0f;
    const float inspectorX = getWidth() - inspectorW;
    const float inspectorTop = 85.0f;

    inspectorTitleLabel.setBounds (inspectorX + 15, 52, inspectorW - 30, 24);

    btnTabTopDown.setBounds (inspectorX + 15, inspectorTop, 140, 28);
    btnTabBottomUp.setBounds (inspectorX + 160, inspectorTop, 145, 28);

    if (!isBottomUpMode)
    {
        // TOP-DOWN Mode (Object-Specific Parameters, Expressions & Methods)
        float rowY = inspectorTop + 45.0f;
        for (auto& row : propertyRows)
        {
            if (row.label) row.label->setBounds (inspectorX + 15, rowY, inspectorW - 30, 18);
            if (row.slider) row.slider->setBounds (inspectorX + 15, rowY + 20, inspectorW - 195, 24);
            if (row.btnModInlet) row.btnModInlet->setBounds (inspectorX + inspectorW - 175, rowY + 20, 75, 24);
            if (row.btnTapValue) row.btnTapValue->setBounds (inspectorX + inspectorW - 95, rowY + 20, 80, 24);
            if (row.exprEditor) row.exprEditor->setBounds (inspectorX + 15, rowY + 46, inspectorW - 30, 20);

            if (row.label) row.label->setVisible (true);
            if (row.slider) row.slider->setVisible (true);
            if (row.btnModInlet) row.btnModInlet->setVisible (true);
            if (row.btnTapValue) row.btnTapValue->setVisible (true);
            if (row.exprEditor) row.exprEditor->setVisible (true);

            rowY += 72.0f;
        }

        for (auto& mBtn : methodButtons)
        {
            if (mBtn)
            {
                mBtn->setBounds (inspectorX + 15, rowY, inspectorW - 30, 26);
                mBtn->setVisible (true);
                rowY += 32.0f;
            }
        }

        // Layout Incoming Connections
        if (incomingSectionHeader)
        {
            incomingSectionHeader->setBounds (inspectorX + 15, rowY + 5, inspectorW - 30, 20);
            incomingSectionHeader->setVisible (true);
            rowY += 28.0f;
        }

        for (auto& cRow : connectionRows)
        {
            if (cRow.isIncoming)
            {
                if (cRow.label)
                {
                    cRow.label->setBounds (inspectorX + 15, rowY, cRow.btnRemoveWire ? (inspectorW - 105) : (inspectorW - 30), 22);
                    cRow.label->setVisible (true);
                }
                if (cRow.btnRemoveWire)
                {
                    cRow.btnRemoveWire->setBounds (inspectorX + inspectorW - 85, rowY, 70, 22);
                    cRow.btnRemoveWire->setVisible (true);
                }
                rowY += 25.0f;
            }
        }

        // Layout Outgoing Connections
        if (outgoingSectionHeader)
        {
            outgoingSectionHeader->setBounds (inspectorX + 15, rowY + 5, inspectorW - 30, 20);
            outgoingSectionHeader->setVisible (true);
            rowY += 28.0f;
        }

        for (auto& cRow : connectionRows)
        {
            if (!cRow.isIncoming)
            {
                if (cRow.label)
                {
                    cRow.label->setBounds (inspectorX + 15, rowY, cRow.btnRemoveWire ? (inspectorW - 105) : (inspectorW - 30), 22);
                    cRow.label->setVisible (true);
                }
                if (cRow.btnRemoveWire)
                {
                    cRow.btnRemoveWire->setBounds (inspectorX + inspectorW - 85, rowY, 70, 22);
                    cRow.btnRemoveWire->setVisible (true);
                }
                rowY += 25.0f;
            }
        }

        btnInsertTapDropdown.setVisible (false);
        formulaEditor.setVisible (false);
        btnApplyFormula.setVisible (false);
    }
    else
    {
        // BOTTOM-UP Mode (Code Math)
        for (auto& row : propertyRows)
        {
            if (row.label) row.label->setVisible (false);
            if (row.slider) row.slider->setVisible (false);
            if (row.btnModInlet) row.btnModInlet->setVisible (false);
            if (row.btnTapValue) row.btnTapValue->setVisible (false);
            if (row.exprEditor) row.exprEditor->setVisible (false);
        }
        for (auto& mBtn : methodButtons)
        {
            if (mBtn) mBtn->setVisible (false);
        }

        if (incomingSectionHeader) incomingSectionHeader->setVisible (false);
        if (outgoingSectionHeader) outgoingSectionHeader->setVisible (false);
        for (auto& cRow : connectionRows)
        {
            if (cRow.label) cRow.label->setVisible (false);
            if (cRow.btnRemoveWire) cRow.btnRemoveWire->setVisible (false);
        }

        btnInsertTapDropdown.setBounds (inspectorX + 15, inspectorTop + 45, inspectorW - 30, 26);
        formulaEditor.setBounds (inspectorX + 15, inspectorTop + 76, inspectorW - 30, 290);
        btnApplyFormula.setBounds (inspectorX + 15, inspectorTop + 375, inspectorW - 30, 32);

        btnInsertTapDropdown.setVisible (true);
        formulaEditor.setVisible (true);
        btnApplyFormula.setVisible (true);
    }
}

} // namespace time_dilation
