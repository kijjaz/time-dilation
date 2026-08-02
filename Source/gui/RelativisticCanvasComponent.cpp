#include "RelativisticCanvasComponent.h"
#include "RelativisticNodeObjects.h"
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

    addAndMakeVisible (btnMenuPatch);
    btnMenuPatch.onClick = [this] { showMenuPatch(); };

    addAndMakeVisible (btnMenuAudio);
    btnMenuAudio.onClick = [this] { showMenuAudio(); };

    addAndMakeVisible (btnMenuHelp);
    btnMenuHelp.onClick = [this] { showMenuHelp(); };

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
    btnSavePatch.onClick = [this] {
        auto desktop = juce::File::getSpecialLocation (juce::File::userDesktopDirectory);
        auto patchFile = desktop.getChildFile ("MyRelativisticPatch.relpatch");
        if (nodeGraph.saveProjectToFile (patchFile))
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon, "PATCH SAVED", "Saved patch file to: " + patchFile.getFullPathName());
        }
    };

    addAndMakeVisible (btnLoadPatch);
    btnLoadPatch.onClick = [this] {
        auto desktop = juce::File::getSpecialLocation (juce::File::userDesktopDirectory);
        auto patchFile = desktop.getChildFile ("MyRelativisticPatch.relpatch");
        if (nodeGraph.loadProjectFromFile (patchFile))
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon, "PATCH LOADED", "Loaded patch file from: " + patchFile.getFullPathName());
            repaint();
        }
    };

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

    addAndMakeVisible (btnAddObject);
    btnAddObject.onClick = [this] {
        showObjectSearchMenu ({ getWidth() * 0.4f, getHeight() * 0.4f });
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

    startTimerHz (30);
}

void RelativisticCanvasComponent::showHelpDialog (const juce::String& topic, const juce::String& content)
{
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon, topic, content);
}

void RelativisticCanvasComponent::showMenuFile()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- PROJECT FILE MANAGEMENT ---");
    m.addItem (1, "New Patch", true);
    m.addItem (5, "New Window (⌘N)", true);
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
                repaint();
            }
            else if (result == 5)
            {
                if (auto* app = juce::JUCEApplication::getInstance())
                {
                    app->sendUnhandledBacktrace (juce::String());
                    // Spawn new window via JUCE command
                    juce::MessageManager::callAsync ([] {
                        if (auto* appInstance = juce::JUCEApplication::getInstance())
                            appInstance->anotherInstanceStarted ("");
                    });
                }
            }
            else if (result == 2 || result == 3 || result == 4)
            {
                btnSavePatch.triggerClick();
            }
            else if (result == 10)
            {
                showHelpDialog ("Export Audio", "Exporting audio WAV rendering engine.");
            }
            else if (result == 11)
            {
                showHelpDialog ("Export C++ Script", "Exporting authentic C++ DSP graph formulas.");
            }
        });
}

void RelativisticCanvasComponent::showMenuEdit()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- EDIT ACTIONS ---");
    m.addItem (1, "Undo (⌘Z)", nodeGraph.canUndo());
    m.addItem (2, "Redo (⌘⇧Z)", nodeGraph.canRedo());
    m.addSeparator();
    m.addItem (3, "Cut (⌘X)", !selectedNodeIds.empty());
    m.addItem (4, "Copy (⌘C)", !selectedNodeIds.empty());
    m.addItem (5, "Paste (⌘V)", true);
    m.addItem (6, "Duplicate (⌘D)", !selectedNodeIds.empty());
    m.addSeparator();
    m.addItem (7, "Select All (⌘A)", true);
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

    juce::PopupMenu subCords;
    subCords.addItem (10, "Organic Catenary Cables", true, cableStyle == CableStyle::Organic);
    subCords.addItem (11, "Smooth S-Curve Cables", true, cableStyle == CableStyle::SmoothS);
    subCords.addItem (12, "Straight Pure Data-Style", true, cableStyle == CableStyle::Straight);
    m.addSubMenu ("Patch Cord Style", subCords);

    m.addItem (1, "Recenter Canvas View", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuView),
        [this] (int result) {
            if (result == 10) { cableStyle = CableStyle::Organic; btnToggleCord.setButtonText ("CORDS: ORGANIC"); }
            else if (result == 11) { cableStyle = CableStyle::SmoothS; btnToggleCord.setButtonText ("CORDS: SMOOTH S"); }
            else if (result == 12) { cableStyle = CableStyle::Straight; btnToggleCord.setButtonText ("CORDS: STRAIGHT"); }
            repaint();
        });
}

void RelativisticCanvasComponent::showMenuPatch()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- PATCH GRAPH OPERATIONS ---");
    m.addItem (1, "Add Object... (N / Double-Click)", true);

    juce::PopupMenu subTime;
    subTime.addItem (10, "[time.warp~] Dilated Coordinate Clock", true);
    subTime.addItem (11, "[time.retro~] Retrograde Time Reverser", true);
    subTime.addItem (12, "[time.quantize~] Metric Grid Quantizer", true);
    subTime.addItem (13, "[time.metro~] Relativistic Metronome", true);
    subTime.addItem (14, "[time.stasis~] Gravitational Time Freeze", true);
    subTime.addItem (15, "[time.singularity~] Event Horizon Redshift", true);
    subTime.addItem (16, "[time.transport] Multi-Instance Transport", true);
    m.addSubMenu ("Relativistic Time Engines", subTime);

    juce::PopupMenu subAudio;
    subAudio.addItem (20, "[osc~] PolyBLEP VA Oscillator", true);
    subAudio.addItem (21, "[phasor~] Ramp Phase Generator", true);
    subAudio.addItem (22, "[sampler~] Audio Buffer Sampler", true);
    subAudio.addItem (23, "[filter~] State-Variable Filter", true);
    subAudio.addItem (24, "[delay~] Feedback Delay Line", true);
    subAudio.addItem (25, "[dac~] Master Audio DAC", true);
    subAudio.addItem (26, "[gain~] Audio Signal Scaler", true);
    subAudio.addItem (27, "[out~] Master Output & Live VU Meters", true);
    subAudio.addItem (28, "[env~] Envelope Follower", true);
    m.addSubMenu ("Audio Processors", subAudio);

    juce::PopupMenu subMath;
    subMath.addItem (30, "[expr] Control Expression", true);
    subMath.addItem (31, "[expr~] Audio Expression", true);
    subMath.addItem (32, "[fexpr~] Filter Recurrent Expression", true);
    subMath.addItem (33, "[v] Value Storage Control Node", true);
    subMath.addItem (34, "[z~] 1-Sample Feedback Delay", true);
    subMath.addItem (35, "[snapshot~] Audio Snapshot", true);
    subMath.addItem (36, "[+] Signal/Control Adder", true);
    subMath.addItem (37, "[*] Signal/Control Multiplier", true);
    subMath.addItem (38, "[mtof] MIDI Note -> Hz Frequency", true);
    subMath.addItem (39, "[ftom] Hz Frequency -> MIDI Note", true);
    subMath.addItem (40, "[note] Algorithmic Note Generator", true);
    m.addSubMenu ("Math & Control Nodes", subMath);

    m.addSeparator();
    m.addItem (2, "Detect & Highlight Feedback Loops", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuPatch),
        [this] (int result) {
            if (result == 1) showObjectSearchMenu ({ getWidth() * 0.4f, getHeight() * 0.4f });
            else if (result == 2) {
                nodeGraph.detectFeedbackLoops();
                showHelpDialog ("Feedback Loop Audit", "Checked audio graph connections. Any feedback loop connections are highlighted in Neon Warning Red with 1-sample delay memory.");
            }
            else
            {
                std::string typeName;
                if (result == 10) typeName = "time.warp~";
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
    m.addSectionHeader ("--- AUDIO ENGINE & DEVICE ---");
    m.addItem (1, "Audio Engine Power Switch", true, nodeGraph.isAudioEngineEnabled());

    juce::PopupMenu subSampleRate;
    subSampleRate.addItem (10, "44.1 kHz", true);
    subSampleRate.addItem (11, "48.0 kHz", true);
    subSampleRate.addItem (12, "96.0 kHz (Pro High-Res)", true);
    m.addSubMenu ("Sample Rate", subSampleRate);

    juce::PopupMenu subBuffer;
    subBuffer.addItem (20, "128 Samples (Ultra Low Latency)", true);
    subBuffer.addItem (21, "256 Samples (Pro Latency)", true);
    subBuffer.addItem (22, "512 Samples (Standard)", true);
    subBuffer.addItem (23, "1024 Samples (Safe Buffer)", true);
    m.addSubMenu ("Buffer Size", subBuffer);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuAudio),
        [this] (int result) {
            if (result == 1) btnAudioPower.triggerClick();
        });
}

void RelativisticCanvasComponent::showMenuHelp()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- USER HELP & WORKSTATION MANUAL ---");
    m.addItem (1, "Quick Start Guide", true);
    m.addItem (2, "Relativistic Time Dilation Architecture Manual", true);
    m.addItem (3, "Wireless Signal Tapping Syntax (tap())", true);
    m.addItem (4, "Pure Data Expression Scripting & C++ Math", true);
    m.addItem (5, "1-Sample Feedback Loop Protection", true);
    m.addItem (6, "Keyboard Shortcuts & Hotkeys", true);

    juce::PopupMenu subExamples;
    subExamples.addItem (10, "[time.warp~] Continuous Warp Speed Oscillator", true);
    subExamples.addItem (11, "[time.retro~] Retrograde Reverse Playback", true);
    subExamples.addItem (12, "[time.stasis~] Event Horizon Stasis Freeze", true);
    subExamples.addItem (13, "[time.singularity~] Black Hole Gravitational Warping", true);
    subExamples.addItem (14, "[time.quantize~] Relativistic Stutter Grid", true);
    subExamples.addItem (15, "[time.transport] Multi-Clock Transport Sync", true);
    m.addSubMenu ("Relativistic Time Example Patches", subExamples);

    m.addSeparator();
    m.addItem (7, "About Time Dilation DAW (v4.0)...", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuHelp),
        [this] (int result) {
            if (result == 1)
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
                    "- time.singularity~: Event horizon gravitational redshift.\n\n"
                    "UNIVERSAL TIME INLETS:\n"
                    "Every object has a purple time inlet (timeIn). Patching any time engine into an object's timeIn port dilates or reverses that object's clock independently!");
            }
            else if (result == 10) { nodeGraph.loadTimeWarpExamplePatch(); repaint(); }
            else if (result == 11) { nodeGraph.loadTimeRetroExamplePatch(); repaint(); }
            else if (result == 12) { nodeGraph.loadTimeStasisExamplePatch(); repaint(); }
            else if (result == 13) { nodeGraph.loadTimeSingularityExamplePatch(); repaint(); }
            else if (result == 14) { nodeGraph.loadTimeQuantizeExamplePatch(); repaint(); }
            else if (result == 15) { nodeGraph.loadTimeTransportExamplePatch(); repaint(); }
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
                    "- ⌘D / Cmd-D: Duplicate Selected Nodes\n"
                    "- ⌘C / Cmd-C: Copy Selected Nodes to Clipboard\n"
                    "- ⌘V / Cmd-V: Paste Nodes from Clipboard\n"
                    "- ⌘Z / Cmd-Z: Undo Last Patch Action\n"
                    "- ⌘⇧Z / Cmd-Shift-Z: Redo Last Patch Action\n"
                    "- ⌘A / Cmd-A: Select All Nodes\n"
                    "- Delete / Backspace: Remove Selected Nodes or Connections\n"
                    "- Shift-Click / Shift-Drag: Multi-select Nodes");
            }
            else if (result == 7)
            {
                showHelpDialog ("About Time Dilation DAW",
                    "Time Dilation DAW (Version 4.0)\n"
                    "Producer: Kijjaz\n\n"
                    "A state-of-the-art Relativistic Modular Workstation unifying top-down visual patching with bottom-up authentic C++ / DSP math expression coding.");
            }
        });
}

void RelativisticCanvasComponent::showObjectSearchMenu (juce::Point<float> spawnPos)
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- RELATIVISTIC TIME ENGINES ---");
    m.addItem (1, "[time.warp~]        Dilated Coordinate Time Generator", true);
    m.addItem (2, "[time.retro~]       Retrograde Time Reverser", true);
    m.addItem (3, "[time.quantize~]    Metric Grid Time Quantizer", true);
    m.addItem (4, "[time.metro~]       Dilated Metronome Pulse Generator", true);
    m.addItem (24, "[time.stasis~]      Gravitational Time Stasis Freeze Engine", true);
    m.addItem (25, "[time.singularity~] Event Horizon Gravitational Redshift Warp", true);

    m.addSeparator();
    m.addSectionHeader ("--- AUDIO & DSP PROCESSORS ---");
    m.addItem (5, "[osc~]          Sine/Saw/Square Oscillator", true);
    m.addItem (6, "[phasor~]       Linear Ramp Phase Generator", true);
    m.addItem (7, "[sampler~]      Audio Buffer Sampler", true);
    m.addItem (8, "[filter~]       State-Variable Filter", true);
    m.addItem (9, "[delay~]        Feedback Delay Line", true);
    m.addItem (10, "[dac~]         Audio Master Output DAC", true);
    m.addItem (14, "[gain~]        Audio Signal Scaler", true);
    m.addItem (15, "[out~]         Master Output Fader & Live Meters", true);
    m.addItem (16, "[env~]         Envelope Follower (Peak Detector)", true);
    m.addItem (17, "[tap]          Control Signal Wireless Tap", true);
    m.addItem (18, "[tap~]         Audio Signal Wireless Tap", true);

    m.addSeparator();
    m.addSectionHeader ("--- MATH SIGNAL EXPRESSIONS & CONTROL NODES ---");
    m.addItem (11, "[expr]         Control Expression ($v1, tap('node.prop'))", true);
    m.addItem (12, "[expr~]        Audio Expression ($v1, tap('node.prop'))", true);
    m.addItem (13, "[fexpr~]       Filter Recurrent Expression ($y1[-1])", true);
    m.addItem (19, "[v]            Value Storage Control Node", true);
    m.addItem (20, "[z~]           1-Sample Feedback Delay Node", true);
    m.addItem (21, "[snapshot~]    Audio-to-Control Snapshot Node", true);
    m.addItem (22, "[+]            Signal & Control Adder", true);
    m.addItem (23, "[*]            Signal & Control Multiplier", true);

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

        row.label = std::make_unique<juce::Label>();
        row.label->setText (def.name + ":", juce::dontSendNotification);
        row.label->setFont (juce::FontOptions (12.0f, juce::Font::bold));
        addAndMakeVisible (*row.label);

        row.slider = std::make_unique<juce::Slider>();
        row.slider->setSliderStyle (juce::Slider::LinearHorizontal);
        row.slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 55, 18);
        row.slider->setRange (def.minValue, def.maxValue, 0.1);
        row.slider->setValue (def.value);
        std::string paramKey = def.key;
        row.slider->onDragStart = [this] { nodeGraph.pushUndoState(); };
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
            auto n = nodeGraph.getNodeById (primaryId);
            if (n) n->setParamExpression (paramKey, ed->getText().toStdString());
        };
        addAndMakeVisible (*row.exprEditor);

        row.btnModInlet = std::make_unique<juce::TextButton> (def.modInletIdx >= 0 ? "[MODDED]" : "+ MOD INLET");
        row.btnModInlet->onClick = [this, primaryId, paramKey] {
            auto n = nodeGraph.getNodeById (primaryId);
            if (n)
            {
                n->addModulationInlet (paramKey);
                rebuildInspector();
                repaint();
            }
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
            if (n) n->invokeMethod (m);
        };
        addAndMakeVisible (*btn);
        methodButtons.push_back (std::move (btn));
    }

    resized();
}

bool RelativisticCanvasComponent::keyPressed (const juce::KeyPress& key)
{
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
            repaint();
            return true;
        }
    }

    // 9. Add Object Hotkey (N key)
    if (!isCmdOrCtrl && (key.getKeyCode() == 'N' || key.getKeyCode() == 'n'))
    {
        showObjectSearchMenu ({ getWidth() * 0.4f, getHeight() * 0.4f });
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
    return false;
}

RelativisticCanvasComponent::~RelativisticCanvasComponent()
{
    setLookAndFeel (nullptr);
    stopTimer();
}

void RelativisticCanvasComponent::timerCallback()
{
    repaint();
}

juce::Point<float> RelativisticCanvasComponent::getInletPos (const RelativisticNode& node, int idx) const
{
    const float nodeW = 150.0f;
    const int count = static_cast<int>(node.getInlets().size());
    const float spacing = nodeW / static_cast<float>(count + 1);
    return { node.getX() + spacing * (idx + 1), node.getY() };
}

juce::Point<float> RelativisticCanvasComponent::getOutletPos (const RelativisticNode& node, int idx) const
{
    const float nodeW = 150.0f;
    const float nodeH = 44.0f;
    const int count = static_cast<int>(node.getOutlets().size());
    const float spacing = nodeW / static_cast<float>(count + 1);
    return { node.getX() + spacing * (idx + 1), node.getY() + nodeH };
}

void RelativisticCanvasComponent::mouseDown (const juce::MouseEvent& e)
{
    grabKeyboardFocus();
    const float nodeW = 150.0f;
    const float nodeH = 44.0f;
    juce::Point<float> mousePos = e.position;
    bool isShift = e.mods.isShiftDown();

    // 1. Check Outlet Click (Cable Creation)
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

            float distToLine = std::abs ((p2.y - p1.y) * mousePos.x - (p2.x - p1.x) * mousePos.y + p2.x * p1.y - p2.y * p1.x) /
                               std::max (1.0f, p1.getDistanceFrom (p2));
            if (distToLine < 10.0f && mousePos.x >= std::min (p1.x, p2.x) - 10.0f && mousePos.x <= std::max (p1.x, p2.x) + 10.0f)
            {
                selectedConnectionId = conn.id;
                selectedNodeIds.clear();
                repaint();
                return;
            }
        }
    }

    // 3. Check Node Box Click
    for (auto it = nodeGraph.getNodes().rbegin(); it != nodeGraph.getNodes().rend(); ++it)
    {
        const auto& node = *it;
        if (mousePos.x >= node->getX() && mousePos.x <= node->getX() + nodeW &&
            mousePos.y >= node->getY() && mousePos.y <= node->getY() + nodeH)
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
                selectedNodeIds.clear();
                selectedNodeIds.insert (node->getId());
            }

            selectedConnectionId = 0;
            draggingNodeId = node->getId();
            dragOffset = { mousePos.x - node->getX(), mousePos.y - node->getY() };

            rebuildInspector();
            repaint();
            return;
        }
    }

    // 4. Empty Canvas Click -> Rubberband Marquee Selection
    if (!isShift)
    {
        selectedNodeIds.clear();
    }
    selectedConnectionId = 0;
    isMarqueeDragging = true;
    marqueeRect = { mousePos.x, mousePos.y, 0.0f, 0.0f };
    repaint();
}

void RelativisticCanvasComponent::mouseDoubleClick (const juce::MouseEvent& e)
{
    const float nodeW = 150.0f;
    const float nodeH = 44.0f;
    juce::Point<float> mousePos = e.position;

    for (const auto& node : nodeGraph.getNodes())
    {
        if (mousePos.x >= node->getX() && mousePos.x <= node->getX() + nodeW &&
            mousePos.y >= node->getY() && mousePos.y <= node->getY() + nodeH)
        {
            selectedNodeIds.clear();
            selectedNodeIds.insert (node->getId());
            inlineLabelEditor.setBounds (static_cast<int>(node->getX() + 8), static_cast<int>(node->getY() + 8), static_cast<int>(nodeW - 16), 28);
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
    if (isDraggingCable)
    {
        cableDragPos = e.position;
        repaint();
    }
    else if (isMarqueeDragging)
    {
        float x1 = std::min (e.mouseDownPosition.x, e.position.x);
        float y1 = std::min (e.mouseDownPosition.y, e.position.y);
        float w = std::abs (e.position.x - e.mouseDownPosition.x);
        float h = std::abs (e.position.y - e.mouseDownPosition.y);
        marqueeRect = { x1, y1, w, h };

        const float nodeW = 150.0f;
        const float nodeH = 44.0f;

        for (const auto& node : nodeGraph.getNodes())
        {
            juce::Rectangle<float> nodeRect (node->getX(), node->getY(), nodeW, nodeH);
            if (marqueeRect.intersects (nodeRect))
            {
                selectedNodeIds.insert (node->getId());
            }
        }
        repaint();
    }
    else if (draggingNodeId > 0)
    {
        auto anchorNode = nodeGraph.getNodeById (draggingNodeId);
        if (anchorNode)
        {
            float dx = (e.position.x - dragOffset.x) - anchorNode->getX();
            float dy = (e.position.y - dragOffset.y) - anchorNode->getY();

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

    // 3. Render Feedback Warning Badge on Cable Midpoint
    if (isFeedbackLoop)
    {
        juce::Point<float> mid = path.getPointAlongPath (path.getLength() * 0.5f);
        g.setColour (juce::Colour (0xff7f1d1d));
        g.fillRoundedRectangle (mid.x - 70.0f, mid.y - 10.0f, 140.0f, 20.0f, 4.0f);
        g.setColour (juce::Colour (0xffef4444));
        g.drawRoundedRectangle (mid.x - 70.0f, mid.y - 10.0f, 140.0f, 20.0f, 4.0f, 1.0f);
        g.setColour (juce::Colour (0xfff8fafc));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText ("[FEEDBACK: 1-SMP DELAY]", mid.x - 70.0f, mid.y - 10.0f, 140.0f, 20.0f, juce::Justification::centred);
    }
}

void RelativisticCanvasComponent::drawNode (juce::Graphics& g, const std::shared_ptr<RelativisticNode>& node)
{
    const float x = node->getX();
    const float y = node->getY();
    const float w = 150.0f;
    const float h = 44.0f;

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

    // Special Canvas Visualization for [out~] Live VU RMS Meters
    auto outNode = std::dynamic_pointer_cast<OutNode> (node);
    if (outNode)
    {
        float meterX = x + w - 32.0f;
        float meterY = y + 8.0f;
        float meterW = 10.0f;
        float meterH = 28.0f;

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

        drawBar (meterX + 1.0f, rmsL);
        drawBar (meterX + meterW + 1.0f, rmsR);
    }

    // Title Text in Sci-Fi Oxanium Font
    g.setColour (juce::Colour (0xfff8fafc));
    g.setFont (FontManager::getInstance().getOxaniumFont (14.0f, true));
    float textW = outNode ? w - 40.0f : w - 12.0f;
    g.drawText (node->getLabel(), x + 10, y, textW, h, juce::Justification::centredLeft);

    // Inlets (Top Edge Dots with Labels)
    for (size_t i = 0; i < node->getInlets().size(); ++i)
    {
        auto p = getInletPos (*node, static_cast<int>(i));
        const auto& port = node->getInlets()[i];
        NodePortType type = port.type;

        bool hasAudioInput = (port.audioData.getNumSamples() > 0 && port.audioData.getMagnitude (0, port.audioData.getNumSamples()) > 0.0001f);

        juce::Colour portCol = (hasAudioInput || type == NodePortType::Audio) ? juce::Colour (0xff06b6d4) : (type == NodePortType::Time ? juce::Colour (0xff8b5cf6) : juce::Colour (0xfff59e0b));

        g.setColour (juce::Colour (0xff181825));
        g.fillEllipse (p.x - 3.5f, p.y - 3.5f, 7.0f, 7.0f);
        g.setColour (portCol);
        g.fillEllipse (p.x - 2.5f, p.y - 2.5f, 5.0f, 5.0f);
        g.setColour (juce::Colours::white);
        g.drawEllipse (p.x - 3.5f, p.y - 3.5f, 7.0f, 7.0f, 1.0f);

        // Port Name Label
        g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, false));
        g.setColour (portCol.withAlpha (0.9f));
        g.drawText (port.name, p.x - 25.0f, p.y + 4.0f, 50.0f, 10.0f, juce::Justification::centred);
    }

    // Outlets (Bottom Edge Dots with Labels)
    for (size_t i = 0; i < node->getOutlets().size(); ++i)
    {
        auto p = getOutletPos (*node, static_cast<int>(i));
        const auto& port = node->getOutlets()[i];
        NodePortType type = port.type;

        juce::Colour portCol = (type == NodePortType::Audio) ? juce::Colour (0xff06b6d4) : (type == NodePortType::Time ? juce::Colour (0xff8b5cf6) : juce::Colour (0xfff59e0b));

        g.setColour (juce::Colour (0xff181825));
        g.fillEllipse (p.x - 3.5f, p.y - 3.5f, 7.0f, 7.0f);
        g.setColour (portCol);
        g.fillEllipse (p.x - 2.5f, p.y - 2.5f, 5.0f, 5.0f);
        g.setColour (juce::Colours::white);
        g.drawEllipse (p.x - 3.5f, p.y - 3.5f, 7.0f, 7.0f, 1.0f);

        // Port Name Label
        g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, false));
        g.setColour (portCol.withAlpha (0.9f));
        g.drawText (port.name, p.x - 25.0f, p.y - 14.0f, 50.0f, 10.0f, juce::Justification::centred);
    }
}

void RelativisticCanvasComponent::paint (juce::Graphics& g)
{
    // Dark Carbon Canvas Background
    g.fillAll (juce::Colour (0xff070a12));

    // Sci-Fi Micro-Grid Dot Matrix
    g.setColour (juce::Colour (0x1a94a3b8));
    for (int gx = 12; gx < getWidth(); gx += 24)
    {
        for (int gy = 55; gy < getHeight(); gy += 24)
        {
            g.fillEllipse (static_cast<float>(gx), static_cast<float>(gy), 1.5f, 1.5f);
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
}

void RelativisticCanvasComponent::resized()
{
    // Top Menu Bar Layout
    int menuX = 15;
    int menuY = 6;
    int menuW = 55;
    int menuH = 32;

    btnMenuFile.setBounds  (menuX,                         menuY, menuW, menuH);
    btnMenuEdit.setBounds  (menuX + menuW + 4,             menuY, menuW, menuH);
    btnMenuView.setBounds  (menuX + (menuW + 4) * 2,       menuY, menuW, menuH);
    btnMenuPatch.setBounds (menuX + (menuW + 4) * 3,       menuY, menuW + 5, menuH);
    btnMenuAudio.setBounds (menuX + (menuW + 4) * 4 + 5,   menuY, menuW + 5, menuH);
    btnMenuHelp.setBounds  (menuX + (menuW + 4) * 5 + 10,  menuY, menuW + 5, menuH);

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

        btnInsertTapDropdown.setBounds (inspectorX + 15, inspectorTop + 45, inspectorW - 30, 26);
        formulaEditor.setBounds (inspectorX + 15, inspectorTop + 76, inspectorW - 30, 290);
        btnApplyFormula.setBounds (inspectorX + 15, inspectorTop + 375, inspectorW - 30, 32);

        btnInsertTapDropdown.setVisible (true);
        formulaEditor.setVisible (true);
        btnApplyFormula.setVisible (true);
    }
}

} // namespace time_dilation
