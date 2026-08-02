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

    addAndMakeVisible (btnMenuObjects);
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

    juce::PopupMenu subCords;
    subCords.addItem (10, "Organic Catenary Cables", true, cableStyle == CableStyle::Organic);
    subCords.addItem (11, "Smooth S-Curve Cables", true, cableStyle == CableStyle::SmoothS);
    subCords.addItem (12, "Straight Pure Data-Style", true, cableStyle == CableStyle::Straight);
    m.addSubMenu ("Patch Cord Style", subCords);

    m.addItem (1, "Recenter Canvas View", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuView),
        [this] (int result) {
            if (result == 1) { panX = 0.0f; panY = 0.0f; }
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
    m.addItem (1, "Search All Objects... (N / Double-Click)", true);

    juce::PopupMenu subMonitors;
    subMonitors.addItem (100, "[time.scope]\tRelativistic Time Monitor & Telemetry Visualizer", true);
    subMonitors.addItem (101, "[time.display]\tDigital Time & Dilation Gauge Display", true);
    subMonitors.addItem (102, "[time.monitor]\tCoordinate Time Stream Inspector", true);
    m.addSubMenu ("Time Data Monitors & Telemetry", subMonitors);

    juce::PopupMenu subControl;
    subControl.addItem (110, "[number]\tControl Number Box (Click & Drag Value)", true);
    subControl.addItem (111, "[bang]\tControl Trigger Pulse (1.0 Spike)", true);
    subControl.addItem (112, "[bang~]\tAudio Rate 1-Sample Impulse Spike", true);
    subControl.addItem (113, "[table]\tInteractive Wavetable & Sample Canvas", true);
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
    subAudio.addItem (24, "[delay~]\tFeedback Delay Line", true);
    subAudio.addItem (25, "[dac~]\tMaster Audio DAC", true);
    subAudio.addItem (26, "[gain~]\tAudio Signal Scaler", true);
    subAudio.addItem (27, "[out~]\tMaster Output & Live VU Meters", true);
    subAudio.addItem (28, "[env~]\tEnvelope Follower", true);
    m.addSubMenu ("Audio Processors", subAudio);

    juce::PopupMenu subMath;
    subMath.addItem (30, "[expr]\tControl Expression", true);
    subMath.addItem (31, "[expr~]\tAudio Expression", true);
    subMath.addItem (32, "[fexpr~]\tFilter Recurrent Expression", true);
    subMath.addItem (33, "[v]\tValue Storage Control Node", true);
    subMath.addItem (34, "[z~]\t1-Sample Feedback Delay", true);
    subMath.addItem (35, "[snapshot~]\tAudio Snapshot", true);
    subMath.addItem (36, "[+]\tSignal/Control Adder", true);
    subMath.addItem (37, "[*]\tSignal/Control Multiplier", true);
    subMath.addItem (38, "[mtof]\tMIDI Note -> Hz Frequency", true);
    subMath.addItem (39, "[ftom]\tHz Frequency -> MIDI Note", true);
    subMath.addItem (40, "[note]\tAlgorithmic Note Generator", true);
    m.addSubMenu ("Math & Control Nodes", subMath);

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

    m.addSeparator();
    m.addSectionHeader ("--- USER HELP & WORKSTATION MANUAL ---");
    m.addItem (1, "Quick Start Guide", true);
    m.addItem (2, "Relativistic Time Dilation Architecture Manual", true);
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
    m.addItem (30, "[time.scope]\tRelativistic Time & Telemetry Visualizer Monitor", true);

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
    repaint();
}

float RelativisticCanvasComponent::getNodeWidth (const RelativisticNode& node) const
{
    if (node.getTypeName() == "time.scope" || node.getTypeName() == "time.display" || node.getTypeName() == "time.monitor")
        return 170.0f;
    if (node.getTypeName() == "table")
        return 160.0f;

    juce::Font labelFont (juce::FontOptions (14.0f, juce::Font::bold));
    float textW = labelFont.getStringWidthF (node.getLabel()) + 38.0f;

    int numInlets = static_cast<int>(node.getInlets().size());
    int numOutlets = static_cast<int>(node.getOutlets().size());
    int maxPorts = std::max (numInlets, numOutlets);
    float portsW = static_cast<float>(maxPorts) * 44.0f + 24.0f;

    return std::max ({ 140.0f, textW, portsW });
}

float RelativisticCanvasComponent::getNodeHeight (const RelativisticNode& node) const
{
    if (node.getTypeName() == "time.scope" || node.getTypeName() == "time.display" || node.getTypeName() == "time.monitor")
        return 75.0f;
    if (node.getTypeName() == "table")
        return 70.0f;
    return 48.0f;
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

void RelativisticCanvasComponent::mouseDown (const juce::MouseEvent& e)
{
    grabKeyboardFocus();
    const float nodeW = 150.0f;
    const float nodeH = 44.0f;
    juce::Point<float> mousePos = e.position;
    bool isShift = e.mods.isShiftDown();

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
                selectedNodeIds.clear();
                selectedNodeIds.insert (node->getId());
            }

            selectedConnectionId = 0;
            draggingNodeId = node->getId();
            dragOffset = { mousePos.x - nx, mousePos.y - ny };

            rebuildInspector();
            repaint();
            return;
        }
    }

    // 4. Empty Canvas Click -> Canvas Viewport Panning OR Rubberband Selection
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
        else if (auto numNode = std::dynamic_pointer_cast<NumberNode> (n))
        {
            float currVal = numNode->getParameter ("value", 0.0f);
            float step = e.mods.isShiftDown() ? 0.1f : 1.0f;
            float newVal = currVal - (e.getMouseVectorOffsetY() * 0.1f * step);
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
            float dx = (e.position.x - dragOffset.x - panX) - anchorNode->getX();
            float dy = (e.position.y - dragOffset.y - panY) - anchorNode->getY();

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
        g.setColour (juce::Colour (0xff06b6d4)); // Cyan
        g.setFont (FontManager::getInstance().getOxaniumFont (11.0f, true));
        juce::String gammaStr = juce::String::formatted ("γ: %+.2fx (%d%%)", gamma, static_cast<int>(gamma * 100.0f));
        g.drawText (gammaStr, x + 10.0f, y + 20.0f, w - 16.0f, 15.0f, juce::Justification::centredLeft);

        g.setColour (juce::Colour (0xfff59e0b)); // Gold
        juce::String timeStr = juce::String::formatted ("t_loc: %.3fs", tSec);
        g.drawText (timeStr, x + 10.0f, y + 35.0f, w - 16.0f, 15.0f, juce::Justification::centredLeft);

        // 2. Kinetic Relativistic Speed Gauge Bar
        float gx = x + 8.0f;
        float gy = y + 52.0f;
        float gw = w - 16.0f;
        float gh = 15.0f;

        g.setColour (juce::Colour (0xff070a12));
        g.fillRoundedRectangle (gx, gy, gw, gh, 2.0f);
        g.setColour (juce::Colour (0xff1e293b));
        g.drawRoundedRectangle (gx, gy, gw, gh, 2.0f, 1.0f);

        float midX = gx + gw * 0.5f;
        float normGamma = std::clamp (gamma / 5.0f, -1.0f, 1.0f);
        float barLen = normGamma * (gw * 0.48f);

        if (gamma > 0.001f)
        {
            g.setColour (juce::Colour (0xff06b6d4)); // Forward: Cyber Cyan
            g.fillRect (midX, gy + 2.0f, barLen, gh - 4.0f);
        }
        else if (gamma < -0.001f)
        {
            g.setColour (juce::Colour (0xfff59e0b)); // Retrograde: Relativistic Gold
            g.fillRect (midX + barLen, gy + 2.0f, -barLen, gh - 4.0f);
        }
        else
        {
            g.setColour (juce::Colour (0xff8b5cf6)); // Stasis: Royal Violet
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
        float lw = std::max (44.0f, portFont.getStringWidthF (port.name) + 6.0f);
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
        float lw = std::max (44.0f, portFont.getStringWidthF (port.name) + 6.0f);
        g.setColour (portCol.withAlpha (0.95f));
        g.drawText (port.name, p.x - lw * 0.5f, p.y - 14.0f, lw, 11.0f, juce::Justification::centred);
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
