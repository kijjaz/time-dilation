#include "RelativisticCanvasComponent.h"
#include "RelativisticNodeObjects.h"
#include "RelativisticSequencers.h"
#include "RelativisticExpressionParser.h"
#include "RelativisticTimeline.h"
#include "TimelineEditorComponent.h"
#include "StepSequencerGridComponent.h"
#include "ProjectFileManager.h"
#include "FontManager.h"
#include "../VersionInfo.h"
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

    addAndMakeVisible (btnMenuPool);
    btnMenuPool.setButtonText ("Pool");
    btnMenuPool.onClick = [this] { showPoolDrawer(); };

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

    addAndMakeVisible (btnToggleDebugMode);
    btnToggleDebugMode.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
    btnToggleDebugMode.setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff59e0b));
    btnToggleDebugMode.onClick = [this] {
        isDebugMode = !isDebugMode;
        btnToggleDebugMode.setButtonText (isDebugMode ? "DEBUG: ON" : "DEBUG: OFF");
        btnToggleDebugMode.setColour (juce::TextButton::buttonColourId, isDebugMode ? juce::Colour (0xff6d28d9) : juce::Colour (0xff1e293b));
        btnToggleDebugMode.setColour (juce::TextButton::textColourOffId, isDebugMode ? juce::Colour (0xff38bdf8) : juce::Colour (0xfff59e0b));
        rebuildInspector();
        repaint();
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

    addChildComponent (exprFormulaLabel);
    exprFormulaLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    exprFormulaLabel.setColour (juce::Label::textColourId, juce::Colour (0xff06b6d4));

    addChildComponent (exprFormulaEditor);
    exprFormulaEditor.setFont (juce::FontOptions (12.0f));
    exprFormulaEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f172a));
    exprFormulaEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff06b6d4));
    exprFormulaEditor.onReturnKey = [this] {
        int primaryId = !selectedNodeIds.empty() ? *selectedNodeIds.begin() : 0;
        auto n = nodeGraph.getNodeById (primaryId);
        if (n)
        {
            std::string exprStr = exprFormulaEditor.getText().toStdString();
            std::string typeName = n->getTypeName();
            if (typeName == "expr")           n->setLabel ("expr " + exprStr);
            else if (typeName == "expr~")     n->setLabel ("expr~ " + exprStr);
            else if (typeName == "fexpr~")    n->setLabel ("fexpr~ " + exprStr);
            else                              n->setLabel (exprStr);
            n->setFormulaScript (exprStr);
            rebuildInspector();
            repaint();
        }
    };

    addChildComponent (btnApplyExprFormula);
    btnApplyExprFormula.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f766e));
    btnApplyExprFormula.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff38bdf8));
    btnApplyExprFormula.onClick = [this] {
        int primaryId = !selectedNodeIds.empty() ? *selectedNodeIds.begin() : 0;
        auto n = nodeGraph.getNodeById (primaryId);
        if (n)
        {
            std::string exprStr = exprFormulaEditor.getText().toStdString();
            std::string typeName = n->getTypeName();
            if (typeName == "expr")           n->setLabel ("expr " + exprStr);
            else if (typeName == "expr~")     n->setLabel ("expr~ " + exprStr);
            else if (typeName == "fexpr~")    n->setLabel ("fexpr~ " + exprStr);
            else                              n->setLabel (exprStr);
            n->setFormulaScript (exprStr);
            rebuildInspector();
            repaint();
        }
    };

    addChildComponent (helpModalOverlay);
    helpModalOverlay.setVisible (false);

    addChildComponent (inlineLabelEditor);
    inlineLabelEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff1e293b));
    inlineLabelEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xffeab308));
    inlineLabelEditor.onReturnKey = [this] {
        std::string newText = inlineLabelEditor.getText().toStdString();
        int targetId = editingNodeId > 0 ? editingNodeId : (!selectedNodeIds.empty() ? *selectedNodeIds.begin() : 0);
        auto n = nodeGraph.getNodeById (targetId);
        if (n)
        {
            n->setLabel (newText);
            std::stringstream ss (newText);
            std::string type;
            ss >> type;
            float val = 0.0f;
            if (ss >> val)
            {
                if (n->getTypeName() == "osc~") n->setParameter ("frequency", val);
                else if (n->getTypeName() == "gain~") n->setParameter ("gain", val);
                else if (n->getTypeName() == "metro") n->setParameter ("tempo", val);
                else if (n->getTypeName() == "number") n->setParameter ("value", val);
            }
            rebuildInspector();
        }
        editingNodeId = 0;
        inlineLabelEditor.setVisible (false);
        repaint();
    };
    inlineLabelEditor.onFocusLost = [this] {
        if (editingNodeId > 0)
        {
            auto n = nodeGraph.getNodeById (editingNodeId);
            if (n) n->setLabel (inlineLabelEditor.getText().toStdString());
        }
        editingNodeId = 0;
        inlineLabelEditor.setVisible (false);
        repaint();
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

    // Quick Canvas Navigation HUD Toolbar Controls
    addAndMakeVisible (btnNavZoomOut);
    btnNavZoomOut.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
    btnNavZoomOut.onClick = [this] { zoomOut(); };

    addAndMakeVisible (btnNavResetZoom);
    btnNavResetZoom.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
    btnNavResetZoom.onClick = [this] { resetZoom(); };

    addAndMakeVisible (btnNavZoomIn);
    btnNavZoomIn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
    btnNavZoomIn.onClick = [this] { zoomIn(); };

    addAndMakeVisible (btnNavFitView);
    btnNavFitView.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f766e));
    btnNavFitView.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff38bdf8));
    btnNavFitView.onClick = [this] { fitAllNodesInView(); };

    addAndMakeVisible (btnNavTidy);
    btnNavTidy.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6d28d9));
    btnNavTidy.setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff59e0b));
    btnNavTidy.onClick = [this] { autoTidyLayout(); };

    addAndMakeVisible (btnToggleConsole);
    btnToggleConsole.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f172a));
    btnToggleConsole.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff10b981));
    btnToggleConsole.onClick = [this] {
        isConsoleVisible = !isConsoleVisible;
        btnToggleConsole.setButtonText (isConsoleVisible ? "CONSOLE: ON" : "CONSOLE: OFF");
        btnToggleConsole.setColour (juce::TextButton::buttonColourId, isConsoleVisible ? juce::Colour (0xff065f46) : juce::Colour (0xff0f172a));
        consoleEditor.setVisible (isConsoleVisible);
        btnClearConsole.setVisible (isConsoleVisible);
        resized();
        repaint();
    };

    addChildComponent (consoleEditor);
    consoleEditor.setMultiLine (true);
    consoleEditor.setReadOnly (true);
    consoleEditor.setScrollbarsShown (true);
    consoleEditor.setCaretVisible (false);
    consoleEditor.setFont (FontManager::getInstance().getOxaniumFont (11.0f, false));
    consoleEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff070a12));
    consoleEditor.setColour (juce::TextEditor::textColourId, juce::Colour (0xff10b981));
    consoleEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff1e293b));

    addChildComponent (btnClearConsole);
    btnClearConsole.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
    btnClearConsole.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffef4444));
    btnClearConsole.onClick = [this] {
        nodeGraph.clearConsoleLogs();
        consoleEditor.setText ("--- Console Logs Cleared ---");
    };

    addAndMakeVisible (inspectorViewport);
    inspectorViewport.setScrollBarsShown (true, false);
    inspectorViewport.setViewedComponent (&inspectorContainer, false);
    inspectorViewport.getVerticalScrollBar().setColour (juce::ScrollBar::thumbColourId, juce::Colour (0xff38bdf8).withAlpha (0.6f));

    inspectorContainer.addAndMakeVisible (exprFormulaLabel);
    inspectorContainer.addAndMakeVisible (exprFormulaEditor);
    inspectorContainer.addAndMakeVisible (btnApplyExprFormula);

    nodeGraph.onGraphModified = [this] {
        markUnsavedChanges();
        repaint();
    };

    oscServer = std::make_unique<RelativisticOscServer> (nodeGraph);
    oscServer->onGraphModified = [this] { markUnsavedChanges(); repaint(); rebuildInspector(); };
    oscServer->onOscLogMessage = [this] (const std::string& msg, bool isWarn) {
        showNotificationBanner (msg, isWarn);
    };
    oscServer->onExportWavRequested = [this] (const std::string& path, float durSec) {
        exportAudioWavToFile (juce::File (path), durSec);
    };
    oscServer->onExportPngRequested = [this] (const std::string& path) {
        exportCanvasPngToFile (juce::File (path));
    };
    oscServer->onLoadPatchRequested = [this] (const std::string& path) {
        juce::File file (path);
        if (ProjectFileManager::getInstance().loadProjectBundle (file, nodeGraph))
        {
            currentProjectFile = file;
            clearUnsavedChanges();
            selectedNodeIds.clear();
            selectedConnectionId = 0;
            rebuildInspector();
            repaint();
        }
    };
    oscServer->onSavePatchRequested = [this] (const std::string& path) {
        juce::File file (path);
        ProjectFileManager::getInstance().saveProjectBundle (file, nodeGraph);
    };
    oscServer->startServer (9001);

    startTimerHz (30);
}

bool RelativisticCanvasComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        juce::File file (f);
        juce::String ext = file.getFileExtension().toLowerCase();
        if (ext == ".patch" || ext == ".json" || ext == ".tdaw" || ext == ".xml")
            return true;
    }
    return false;
}

void RelativisticCanvasComponent::filesDropped (const juce::StringArray& files, int /*x*/, int /*y*/)
{
    for (const auto& f : files)
    {
        juce::File file (f);
        juce::String ext = file.getFileExtension().toLowerCase();
        if (ext == ".patch" || ext == ".json" || ext == ".tdaw" || ext == ".xml")
        {
            if (ProjectFileManager::getInstance().loadProjectBundle (file, nodeGraph))
            {
                currentProjectFile = file;
                clearUnsavedChanges();
                selectedNodeIds.clear();
                selectedConnectionId = 0;
                rebuildInspector();
                repaint();
                showNotificationBanner ("Loaded Patch: " + file.getFileName().toStdString(), false);
                break;
            }
        }
    }
}

void RelativisticCanvasComponent::updateConsoleDrawer()
{
    auto logs = nodeGraph.getConsoleLogs();
    juce::String text;
    for (const auto& log : logs)
    {
        juce::String timeStr = juce::String::formatted ("[t=%.3fs] ", log.timestampSec);
        text << timeStr << log.sourceLabel << ": " << log.message << "\n";
    }
    if (text.isEmpty()) text = "--- Console Ready. [print] nodes & OSC commands will stream logs here ---";

    if (consoleEditor.getText() != text)
    {
        consoleEditor.setText (text, false);
        consoleEditor.moveCaretToEnd();
    }
}

void RelativisticCanvasComponent::showHelpDialog (const juce::String& topic, const juce::String& content)
{
    helpModalOverlay.showDialog (topic, content);
}

void RelativisticCanvasComponent::requestExit (std::function<void()> onProceedExit)
{
    if (!hasUnsavedChanges)
    {
        if (onProceedExit) onProceedExit();
        return;
    }

    activeAlertWindow = std::make_unique<juce::AlertWindow> (
        "Unsaved Changes",
        "You have unsaved changes in your patch. Would you like to save before closing?",
        juce::AlertWindow::QuestionIcon);

    activeAlertWindow->addButton ("Save", 1);
    activeAlertWindow->addButton ("Don't Save", 2);
    activeAlertWindow->addButton ("Cancel", 0);

    activeAlertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this, onProceedExit] (int result) {
        activeAlertWindow.reset();
        if (result == 1) // Save
        {
            savePatchWithCallback ([onProceedExit] (bool saved) {
                if (saved && onProceedExit) onProceedExit();
            });
        }
        else if (result == 2) // Don't Save
        {
            clearUnsavedChanges();
            if (onProceedExit) onProceedExit();
        }
    }), true);
}

void RelativisticCanvasComponent::savePatchWithCallback (std::function<void(bool)> onComplete)
{
    if (currentProjectFile.existsAsFile() || currentProjectFile.isDirectory())
    {
        bool success = ProjectFileManager::getInstance().saveProjectBundle (currentProjectFile, nodeGraph);
        if (success)
        {
            clearUnsavedChanges();
            if (onComplete) onComplete (true);
        }
        else
        {
            savePatchAsWithCallback (onComplete);
        }
    }
    else
    {
        savePatchAsWithCallback (onComplete);
    }
}

void RelativisticCanvasComponent::savePatchAsWithCallback (std::function<void(bool)> onComplete)
{
    auto fc = std::make_shared<juce::FileChooser> ("Save Time Dilation Relativistic Patch As...",
                                                   juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                                                   "*.patch;*.json;*.tdaw;*.xml");
    fc->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectDirectories | juce::FileBrowserComponent::warnAboutOverwriting,
        [this, fc, onComplete] (const juce::FileChooser& chooser) {
            auto result = chooser.getResult();
            if (result != juce::File())
            {
                juce::File targetFile = result;
                if (!targetFile.isDirectory() && targetFile.getFileExtension().isEmpty())
                {
                    targetFile = targetFile.withFileExtension ("patch");
                }

                bool success = ProjectFileManager::getInstance().saveProjectBundle (targetFile, nodeGraph);
                if (success)
                {
                    currentProjectFile = targetFile;
                    clearUnsavedChanges();
                    if (onComplete) onComplete (true);
                }
                else
                {
                    if (onComplete) onComplete (false);
                }
            }
            else
            {
                if (onComplete) onComplete (false);
            }
        });
}

void RelativisticCanvasComponent::savePatchAs()
{
    savePatchAsWithCallback (nullptr);
}

void RelativisticCanvasComponent::savePatch()
{
    savePatchWithCallback (nullptr);
}

void RelativisticCanvasComponent::loadPresetPatch (const juce::String& presetFileName)
{
    juce::File presetFile = juce::File::getCurrentWorkingDirectory().getChildFile ("Presets").getChildFile (presetFileName);
    if (!presetFile.existsAsFile())
    {
        presetFile = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                        .getParentDirectory().getChildFile ("Presets").getChildFile (presetFileName);
    }

    if (presetFile.existsAsFile())
    {
        if (ProjectFileManager::getInstance().loadProjectBundle (presetFile, nodeGraph))
        {
            currentProjectFile = presetFile;
            clearUnsavedChanges();
            selectedNodeIds.clear();
            selectedConnectionId = 0;
            rebuildInspector();
            repaint();
            showNotificationBanner ("Loaded Preset: " + presetFile.getFileNameWithoutExtension().toStdString(), false);
        }
    }
}

void RelativisticCanvasComponent::loadPatch()
{
    auto fc = std::make_shared<juce::FileChooser> ("Open Time Dilation Relativistic Patch (.patch / .json / .xml)...",
                                                   juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                                                   "*.patch;*.json;*.tdaw;*.xml");
    fc->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectDirectories,
        [this, fc] (const juce::FileChooser& chooser) {
            auto result = chooser.getResult();
            if (result != juce::File())
            {
                bool success = ProjectFileManager::getInstance().loadProjectBundle (result, nodeGraph);
                if (success)
                {
                    currentProjectFile = result;
                    clearUnsavedChanges();
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
    juce::String timeStr = juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S");
    juce::File defaultFile = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                                 .getChildFile ("time_dilation_audio_" + timeStr + ".wav");

    auto fc = std::make_shared<juce::FileChooser> ("Export Rendered Audio WAV File As...",
                                                   defaultFile,
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

void RelativisticCanvasComponent::exportAudioWavToFile (const juce::File& resultFile, float durationSec)
{
    if (resultFile == juce::File()) return;

    juce::String timestampStr = juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S");
    juce::File targetFile = resultFile;

    if (targetFile.isDirectory())
    {
        targetFile = targetFile.getChildFile ("time_dilation_audio_" + timestampStr + ".wav");
    }
    else if (targetFile.getFileNameWithoutExtension().contains ("{timestamp}"))
    {
        juce::String newName = targetFile.getFileNameWithoutExtension().replace ("{timestamp}", timestampStr) + "." + (targetFile.getFileExtension().isEmpty() ? "wav" : targetFile.getFileExtension());
        targetFile = targetFile.getParentDirectory().getChildFile (newName);
    }
    else if (targetFile.getFileExtension().isEmpty())
    {
        targetFile = targetFile.withFileExtension ("wav");
    }

    targetFile.deleteFile();

    bool prevPower = nodeGraph.isAudioEngineEnabled();
    nodeGraph.setAudioEngineEnabled (true);

    double renderRate = 44100.0;
    int renderSamples = static_cast<int>(renderRate * std::max (0.5f, durationSec));
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

    nodeGraph.setAudioEngineEnabled (prevPower);

    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer (wavFormat.createWriterFor (
        targetFile.createOutputStream().release(),
        renderRate,
        2,
        16,
        {},
        0));

    if (writer != nullptr)
    {
        writer->writeFromAudioSampleBuffer (renderBuffer, 0, renderSamples);
    }
}

void RelativisticCanvasComponent::exportCanvasPngToFile (const juce::File& resultFile)
{
    if (resultFile == juce::File()) return;

    juce::String timestampStr = juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S");
    juce::File targetFile = resultFile;

    if (targetFile.isDirectory())
    {
        targetFile = targetFile.getChildFile ("time_dilation_screenshot_" + timestampStr + ".png");
    }
    else if (targetFile.getFileNameWithoutExtension().contains ("{timestamp}"))
    {
        juce::String newName = targetFile.getFileNameWithoutExtension().replace ("{timestamp}", timestampStr) + "." + (targetFile.getFileExtension().isEmpty() ? "png" : targetFile.getFileExtension());
        targetFile = targetFile.getParentDirectory().getChildFile (newName);
    }
    else if (targetFile.getFileExtension().isEmpty())
    {
        targetFile = targetFile.withFileExtension ("png");
    }

    targetFile.deleteFile();

    juce::Image snapshot = createComponentSnapshot (getLocalBounds());
    juce::FileOutputStream stream (targetFile);
    if (stream.openedOk())
    {
        juce::PNGImageFormat pngFormat;
        pngFormat.writeImageToStream (snapshot, stream);
    }
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
                code << "// Time Dilation DAW (v0.0.1) — Exported C++ Relativistic Audio Graph\n";
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
    m.addSeparator();
    m.addItem (99, "Quit Workstation (Cmd+Q)", true);

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
            else if (result == 99)
            {
                requestExit ([] {
                    if (auto* app = juce::JUCEApplication::getInstance())
                        app->systemRequestedQuit();
                });
            }
        });
}

void RelativisticCanvasComponent::showMenuEdit()
{
    juce::PopupMenu m;
    m.addSectionHeader ("--- WORKSTATION MODE ---");
    m.addItem (10, "Mode: Edit Mode (Cmd+E)", true, isEditMode);
    m.addItem (11, "Mode: Perform / Play Mode (Cmd+E)", true, !isEditMode);
    m.addSeparator();
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
            if (result == 10 || result == 11)
            {
                btnToggleMode.triggerClick();
            }
            else if (result == 1) nodeGraph.undo();
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
    m.addItem (109, "Fit All Nodes in View", true);
    m.addSeparator();
    m.addItem (500, "Debug Overlay Mode (Inspect All Nodes & Telemetry)", true, isDebugMode);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuView),
        [this] (int result) {
            if (result == 500) { btnToggleDebugMode.triggerClick(); }
            else if (result == 1) { panX = 0.0f; panY = 0.0f; resetZoom(); }
            else if (result == 109) { fitAllNodesInView(); }
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

void RelativisticCanvasComponent::panCanvas (float dx, float dy)
{
    panX += dx;
    panY += dy;
    repaint();
}

void RelativisticCanvasComponent::fitAllNodesInView()
{
    const auto& nodes = nodeGraph.getNodes();
    if (nodes.empty())
    {
        panX = 0.0f;
        panY = 0.0f;
        zoomLevel = 1.0f;
        repaint();
        return;
    }

    float minX = 99999.0f, minY = 99999.0f;
    float maxX = -99999.0f, maxY = -99999.0f;

    for (const auto& n : nodes)
    {
        float x = n->getX();
        float y = n->getY();
        float w = getNodeWidth (*n);
        float h = getNodeHeight (*n);

        minX = std::min (minX, x);
        minY = std::min (minY, y);
        maxX = std::max (maxX, x + w);
        maxY = std::max (maxY, y + h);
    }

    float canvasW = std::max (100.0f, getWidth() - 340.0f);
    float canvasH = std::max (100.0f, getHeight() - 100.0f);

    float bboxW = std::max (10.0f, maxX - minX + 60.0f);
    float bboxH = std::max (10.0f, maxY - minY + 60.0f);

    float fitZoom = std::min (canvasW / bboxW, canvasH / bboxH);
    fitZoom = std::clamp (fitZoom, 0.4f, 1.5f);

    zoomLevel = fitZoom;
    panX = (canvasW - (maxX + minX) * fitZoom) * 0.5f;
    panY = 55.0f + (canvasH - (maxY + minY) * fitZoom) * 0.5f;

    repaint();
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

void RelativisticCanvasComponent::showPoolDrawer()
{
    if (!poolDrawer)
    {
        poolDrawer = std::make_unique<AssetPoolDrawerComponent>();
        addAndMakeVisible (poolDrawer.get());
        poolDrawer->onCloseRequested = [this] {
            poolDrawer->setVisible (false);
        };
    }
    poolDrawer->refreshPool();
    poolDrawer->setVisible (true);
    poolDrawer->toFront (true);
    resized();
}

void RelativisticCanvasComponent::showMenuHelp()
{
    juce::PopupMenu m;
    
    // 1. CONTROL & TRIGGER PATCHES
    m.addSectionHeader ("1. CONTROL & TRIGGER PATCHES");
    m.addItem (20, "Basic Counter & Pulse Trigger ([bang] -> [counter] -> [number])", true);
    m.addItem (21, "Step Sequencer & Pitch Dataflow ([time.metro~] -> [seq] -> [mtof])", true);

    // 2. MATH & EXPRESSION PATCHES
    m.addSeparator();
    m.addSectionHeader ("2. MATH & SIGNAL EXPRESSION PATCHES");
    m.addItem (22, "Mathematical Expression Evaluator ([number] -> [expr] -> [number])", true);
    m.addItem (23, "Wireless Parameter Tapping ([osc1] & [expr tap('osc1.frequency')])", true);

    // 3. AUDIO & SYNTHESIS PATCHES
    m.addSeparator();
    m.addSectionHeader ("3. TRADITIONAL AUDIO & DSP SYNTHESIS");
    m.addItem (24, "Simple Waveform Oscillator ([osc~] -> [gain~] -> [out~])", true);
    m.addItem (16, "Interactive Wavetable Drawing ([table] -> [tabosc4~] -> [out~])", true);
    m.addItem (19, "Modular Subtractive Synth ([tidal] -> [mtof] -> [osc~] -> [filter~] -> [adsr~] -> [gain~] -> [out~])", true);
    m.addItem (18, "Polyphonic Future Bass Drums ([drumseq] -> [fbdrum~] -> [out~])", true);

    // 4. RELATIVISTIC TIME DILATION & COMBINED ENGINE PATCHES
    m.addSeparator();
    m.addSectionHeader ("4. RELATIVISTIC TIME DILATION & COMBINED ENGINE");
    m.addItem (10, "[time.warp~] Continuous Warp Speed Oscillator", true);
    m.addItem (11, "[time.retro~] Retrograde Reverse Playback (-1.0x)", true);
    m.addItem (12, "[time.stasis~] Event Horizon Stasis Freeze", true);
    m.addItem (13, "[time.singularity~] Black Hole Gravitational Redshift", true);
    m.addItem (14, "[time.quantize~] Relativistic Metric Grid Stutter", true);
    m.addItem (15, "[time.transport] Multi-Clock Transport Sync", true);
    m.addItem (17, "[time.math~] Lorentz Time Boost Composition", true);

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

    auto openExampleInNewWindow = [] (int patchId) {
        juce::MessageManager::callAsync ([patchId] {
            if (auto* appInstance = juce::JUCEApplication::getInstance())
                appInstance->anotherInstanceStarted ("--example=" + juce::String (patchId));
        });
    };

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btnMenuHelp),
        [this, openExampleInNewWindow] (int result) {
            if (result == 20)      openExampleInNewWindow (20);
            else if (result == 21) openExampleInNewWindow (21);
            else if (result == 22) openExampleInNewWindow (22);
            else if (result == 23) openExampleInNewWindow (23);
            else if (result == 24) openExampleInNewWindow (24);
            else if (result == 16) openExampleInNewWindow (16);
            else if (result == 19) openExampleInNewWindow (19);
            else if (result == 18) openExampleInNewWindow (18);
            else if (result == 10) openExampleInNewWindow (10);
            else if (result == 11) openExampleInNewWindow (11);
            else if (result == 12) openExampleInNewWindow (12);
            else if (result == 13) openExampleInNewWindow (13);
            else if (result == 14) openExampleInNewWindow (14);
            else if (result == 15) openExampleInNewWindow (15);
            else if (result == 17) openExampleInNewWindow (17);
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
                    "Time Dilation DAW (" + juce::String (APP_VERSION_STRING) + ")\n"
                    "Producer: Kijjaz\n\n"
                    "A state-of-the-art Relativistic Modular Workstation unifying top-down visual patching with bottom-up authentic C++ / DSP math expression coding.");
            }
        });
}

void RelativisticCanvasComponent::initObjectCatalog()
{
    allRegisteredObjects = {
        // Relativistic Time Engines
        { "time.warp~", "Dilated Coordinate Time Generator (LFO Dilation)", "TIME", "time, warp, dilation, speed, rate, lfo, gamma, relativity, clock" },
        { "time.retro~", "Retrograde Time Reverser (-1.0x Time Flow)", "TIME", "time, retro, reverse, retrograde, backwards, inverted, flow" },
        { "time.stasis~", "Gravitational Time Stasis Freeze Engine", "TIME", "time, stasis, freeze, stop, zero, gravity, static, pause" },
        { "time.singularity~", "Event Horizon Gravitational Redshift Warp", "TIME", "time, singularity, black hole, redshift, distortion, gravity, horizon" },
        { "time.quantize~", "Metric Grid Time Quantizer (Micro-Step Stutter)", "TIME", "time, quantize, grid, step, stutter, rhythm, metric, subdivision" },
        { "time.transport", "Relativistic Master Transport Hub (BPM Clock)", "TIME", "time, transport, bpm, tempo, clock, master, play, bar, beat" },
        { "time.metro~", "Dilated Metronome Pulse Spiker", "TIME", "time, metro, metronome, pulse, tick, trigger, clock" },
        { "time.scope", "Relativistic Time & Telemetry Visualizer Monitor", "TIME", "time, scope, monitor, plot, display, graph, CRT, visualizer, telemetry" },
        { "time.xy", "2D Time Signal XY Vector Oscilloscope Plot", "TIME", "time, xy, lissajous, 2D, scope, plot, vector, visualizer" },
        { "time.future~", "Future Lookahead Causality Offset Engine", "TIME", "time, future, lookahead, causality, prediction, pre-delay, advance" },

        // Audio & DSP Generators & Processors
        { "spectrometer~", "Live Audio Spectrum Visualizer (Logo Gradient)", "DSP", "audio, spectrometer, spectrum, FFT, visualizer, frequency, display, logo" },
        { "spectrum~", "Audio Frequency Spectrometer Visualizer", "DSP", "audio, spectrum, frequency, FFT, display, analyzer" },
        { "fft~", "Fast Fourier Transform Audio Analyzer", "DSP", "audio, fft, fourier, transform, spectrum, frequency, analysis" },
        { "osc~", "Sine/Saw/Square Varispeed Oscillator", "DSP", "audio, osc, oscillator, sine, saw, square, triangle, synth, sound, generator, pitch, freq, tone" },
        { "phasor~", "Linear Ramp Audio Phase Generator", "DSP", "audio, phasor, ramp, phase, sawtooth, generator, LFO, sync" },
        { "sampler~", "Varispeed Audio Sampler & Loop Player", "DSP", "audio, sampler, sample, wav, player, playback, loop, varispeed, audiofile, sound" },
        { "filter~", "State-Variable Filter (LP/HP/BP/Notch)", "DSP", "audio, filter, cutoff, lowpass, highpass, bandpass, notch, resonance, EQ" },
        { "svfilter~", "Vadim Zavalishin TPT State-Variable Filter", "DSP", "audio, filter, svf, tpt, cutoff, resonance, ladder, analog" },
        { "delay~", "Feedback Delay Line (Hermite Varispeed)", "DSP", "audio, delay, echo, feedback, time, hermite, varispeed, slapback" },
        { "drive~", "Non-Linear Harmonic Tube Overdrive Distortion", "DSP", "audio, drive, distortion, overdrive, tube, saturation, harmonics, gain" },
        { "reverb~", "Stereo Algorithmic Reverb Unit", "DSP", "audio, reverb, room, hall, space, echo, decay, wet" },
        { "crush~", "Quantum Bitcrusher & Sample Reducer", "DSP", "audio, crush, bitcrusher, sample rate, digital, lo-fi, distortion, quantum" },
        { "adsr~", "Attack-Decay-Sustain-Release Envelope Generator", "DSP", "audio, adsr, envelope, attack, decay, sustain, release, ADSR, contour" },
        { "env~", "Audio Envelope Follower (Peak Detector)", "DSP", "audio, env, follower, peak, detector, dynamics, volume, RMS" },
        { "gain~", "Relativistic Audio Signal Scaler & Time Warper", "DSP", "audio, gain, volume, scale, level, amp, multiplier, path" },
        { "out~", "Master Output Fader & Live Oscilloscope CRT", "DSP", "audio, out, output, master, dac, speaker, volume, meter, main" },
        { "dac~", "Audio Master Output Hardware DAC", "DSP", "audio, dac, output, hardware, speaker, soundcard" },
        { "fbdrum~", "Polyphonic Future Bass Drum Synthesizer", "DSP", "audio, drum, kick, snare, percussion, synth, bass, fbdrum" },
        { "tabosc4~", "4-Point Hermite Interpolated Wavetable Oscillator", "DSP", "audio, tabosc4, wavetable, table, oscillator, lookup, 4point, hermite" },

        // Sequencers & Generative Engines
        { "seq", "Multi-Step Pattern Sequencer", "SEQ", "seq, sequencer, step, pattern, notes, pitch, melody" },
        { "drumseq", "Multi-Track 16-Step Future Bass Drum Sequencer", "SEQ", "drumseq, drum, sequence, rhythm, beat, 16step, pattern" },
        { "euclid", "Euclidean Rhythm Generator", "SEQ", "euclid, euclidean, rhythm, generator, pattern, polyrhythm, algorithm" },
        { "markov", "Stochastic Markov Chain Melodic Generator", "SEQ", "markov, stochastic, probabilistic, random, generator, melody, chain" },
        { "tidal", "Tidal Live-Coding Mini-Notation Sequencer", "SEQ", "tidal, livecoding, mini-notation, pattern, rhythm, beat, code, algo" },
        { "timeline", "Multi-Track Timeline Clip Sequencer", "SEQ", "timeline, multitrack, DAW, clips, arrangement, sequence, track" },

        // Control Interactors & Triggers
        { "number", "Control Number Box (Hot/Cold Inlets)", "CTRL", "number, num, box, value, slider, control, input, float, int" },
        { "num", "Control Number Box (Hot/Cold Inlets)", "CTRL", "num, number, box, value, slider, control, input" },
        { "f", "Float Control Box (Hot/Cold Inlets)", "CTRL", "f, float, number, num, decimal, value, input" },
        { "float", "Float Control Box (Hot/Cold Inlets)", "CTRL", "float, f, number, num, decimal, value, input" },
        { "i", "Integer Control Box (Rounds to Int, Hot/Cold)", "CTRL", "i, int, integer, round, whole, number, count" },
        { "int", "Integer Control Box (Rounds to Int, Hot/Cold)", "CTRL", "int, i, integer, round, whole, number, count" },
        { "integer", "Integer Control Box (Rounds to Int, Hot/Cold)", "CTRL", "integer, i, int, round, whole, number, count" },
        { "msg", "Message Box (Store & Send Control Value, $1 Parameter Substitution)", "CTRL", "msg, message, symbol, text, command, send, trigger, parameter, sub" },
        { "message", "Message Box (Store & Send Control Value, $1 Parameter Substitution)", "CTRL", "message, msg, symbol, text, command, send, trigger, parameter, sub" },
        { "bang", "Control Trigger Pulse Spiker", "CTRL", "bang, trigger, pulse, button, click, fire, spike" },
        { "bang~", "Audio-Rate Impulse Spike Spiker", "CTRL", "bang, audio trigger, pulse, spike, impulse, click" },
        { "counter", "Smart Value Counter (Low, High, Step, Carry)", "CTRL", "counter, count, step, index, accumulator, math" },
        { "metro", "Standard Control Metronome Pulse Generator (Pd metro)", "CTRL", "metro, metronome, pulse, tick, trigger, clock, timer, period, bpm" },
        { "note", "MIDI Note Pitch Generator", "CTRL", "note, midi, pitch, key, frequency, mtof" },
        { "tap", "Control Signal Wireless Tap Listener", "CTRL", "tap, wireless, receiver, listener, parameter, query, link" },
        { "tap~", "Audio Signal Wireless Tap Listener", "CTRL", "tap, wireless, audio, receiver, listener, signal, link" },

        // Expressions & Math
        { "expr", "Control Expression ($v1, tap('id'))", "MATH", "path, math, expr, expression, formula, script, code, evaluation, calculate, add, multiply" },
        { "expr~", "Audio Expression ($v1, tap('id'))", "MATH", "path, math, expr, audio expression, formula, dsp, code, script, calculate" },
        { "fexpr~", "Filter Recurrent Expression ($y1[-1])", "MATH", "path, math, fexpr, filter expression, recurrent, y1, code, script, history" },
        { "v", "Value Storage Control Node", "MATH", "path, math, v, value, variable, store, memory, hold" },
        { "z~", "1-Sample Feedback Delay Unit", "MATH", "path, math, z, delay, 1sample, feedback, memory, unit" },
        { "snapshot~", "Audio-to-Control Sample Snapshot", "MATH", "path, math, snapshot, sample, hold, audio2control, capture" },
        { "+", "Signal & Control Adder", "MATH", "path, math, +, add, plus, sum, arithmetic, calculate, scalar, combining" },
        { "-", "Signal & Control Subtractor", "MATH", "path, math, -, sub, minus, subtract, arithmetic, calculate, scalar" },
        { "*", "Signal & Control Multiplier", "MATH", "path, math, *, multiply, mult, scale, product, gain, arithmetic, scalar" },
        { "/", "Signal & Control Divider", "MATH", "path, math, /, div, divide, division, fraction, ratio, arithmetic, scalar" },
        { "%", "Signal & Control Modulo", "MATH", "path, math, %, mod, modulo, remainder, fmod, wrap, arithmetic, scalar" },
        { "mod", "Signal & Control Modulo", "MATH", "path, math, mod, %, modulo, remainder, fmod, wrap, arithmetic, scalar" },
        { "mtof", "MIDI Pitch to Frequency Hz Converter", "MATH", "path, math, mtof, midi, pitch, frequency, hz, convert" },
        { "ftom", "Frequency Hz to MIDI Pitch Converter", "MATH", "path, math, ftom, frequency, hz, midi, pitch, convert" },

        // Tables & Data Memory
        { "table", "Wavetable / Step Value Memory Canvas", "DATA", "data, table, array, wavetable, memory, buffer, graph, draw, sample" },
        { "tabwrite~", "Write Audio Buffer to Table Memory", "DATA", "data, tabwrite, write, record, store, table, memory, buffer" },
        { "tabread~", "Read Audio Buffer from Table Memory", "DATA", "data, tabread, read, playback, table, memory, buffer" }
    };
    filteredAutocompleteItems = allRegisteredObjects;
}

void RelativisticCanvasComponent::updateDraftObjectBounds()
{
    if (isEditingDraftObject && draftObjectEditor)
    {
        float boxX = draftObjectCanvasPos.x + panX;
        float boxY = draftObjectCanvasPos.y + panY;

        juce::String currentText = draftObjectEditor->getText();
        float textW = getTextWidth (draftObjectEditor->getFont(), currentText) + 36.0f;
        float boxW = std::clamp (textW, 160.0f, 440.0f);
        float boxH = 34.0f;

        draftObjectEditor->setBounds (static_cast<int>(boxX), static_cast<int>(boxY), static_cast<int>(boxW), static_cast<int>(boxH));
    }
}

void RelativisticCanvasComponent::spawnInlineObjectEditor (juce::Point<float> spawnCanvasPos)
{
    if (isEditingDraftObject)
        destroyDraftObjectEditor();

    draftObjectCanvasPos = spawnCanvasPos;
    isEditingDraftObject = true;
    isDraftObjectInvalid = false;
    draftWarningText = "";

    draftObjectEditor = std::make_unique<juce::TextEditor>();
    draftObjectEditor->setFont (FontManager::getInstance().getOxaniumFont (13.0f, true));
    draftObjectEditor->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f172a));
    draftObjectEditor->setColour (juce::TextEditor::textColourId, juce::Colour (0xff38bdf8));
    draftObjectEditor->setColour (juce::TextEditor::outlineColourId, juce::Colour (0xfff59e0b));
    draftObjectEditor->setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour (0xfff59e0b));
    draftObjectEditor->setIndents (6, 4);

    draftObjectEditor->onTextChange = [this] {
        updateAutocompleteFilter (draftObjectEditor->getText());
        updateDraftObjectBounds();
        repaint();
    };

    draftObjectEditor->onReturnKey = [this] {
        commitDraftObject();
    };

    draftObjectEditor->onEscapeKey = [this] {
        destroyDraftObjectEditor();
        repaint();
    };

    addAndMakeVisible (*draftObjectEditor);
    draftObjectEditor->grabKeyboardFocus();

    updateAutocompleteFilter ("");
    updateDraftObjectBounds();
    resized();
    repaint();
}

void RelativisticCanvasComponent::destroyDraftObjectEditor()
{
    isEditingDraftObject = false;
    isDraftObjectInvalid = false;
    draftWarningText = "";
    if (draftObjectEditor)
    {
        removeChildComponent (draftObjectEditor.get());
        draftObjectEditor.reset();
    }
}

void RelativisticCanvasComponent::updateAutocompleteFilter (const juce::String& text)
{
    filteredAutocompleteItems.clear();
    juce::String trimmed = text.trim();
    int spaceIdx = trimmed.indexOfChar (' ');
    juce::String firstWord = (spaceIdx >= 0 ? trimmed.substring (0, spaceIdx) : trimmed).toLowerCase();

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
            juce::String kw = juce::String (item.keywords).toLowerCase();
            if (tName.startsWith (firstWord) || tName.contains (firstWord) || desc.contains (firstWord) || kw.contains (firstWord))
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

    if (!RelativisticNodeGraph::isValidObjectType (typeToken))
    {
        if (selectedAutocompleteIdx >= 0 && selectedAutocompleteIdx < static_cast<int>(filteredAutocompleteItems.size()))
        {
            typeToken = filteredAutocompleteItems[selectedAutocompleteIdx].typeName;
        }
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
        if (fullText.length() > juce::String (typeToken).length())
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
    m.addItem (1, "[time.warp]\tDilated Coordinate Time Generator", true);
    m.addItem (2, "[time.retro]\tRetrograde Time Reverser", true);
    m.addItem (3, "[time.quantize]\tMetric Grid Time Quantizer", true);
    m.addItem (4, "[time.metro]\tDilated Metronome Pulse Generator", true);
    m.addItem (44, "[time.+]\tTime Clock Adder ([time.+ 0.5])", true);
    m.addItem (45, "[time.-]\tTime Clock Subtractor ([time.- 0.2])", true);
    m.addItem (46, "[time.*]\tTime Clock Multiplier / Scaler ([time.* 2.0])", true);
    m.addItem (47, "[time./]\tTime Clock Divider ([time./ 1.5])", true);
    m.addItem (48, "[time.expr]\tCustom Time Math Expression (g * v1 + v2)", true);
    m.addItem (24, "[time.stasis]\tGravitational Time Stasis Freeze Engine", true);
    m.addItem (25, "[time.singularity]\tEvent Horizon Gravitational Redshift Warp", true);
    m.addItem (40, "[time.future]\tFuture Lookahead Causality Offset Engine", true);
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
    m.addItem (49, "[toggle]\t0/1 Switch (Click or Bang to Toggle)", true);
    m.addItem (50, "[slider]\tControl Value Slider Bar", true);
    m.addItem (51, "[radio]\tMulti-Step Radio Selector Strip", true);
    m.addItem (52, "[spigot]\tControl Message Gate (Pass/Block)", true);
    m.addItem (53, "[select]\tValue Selector ([sel 0 60 127])", true);
    m.addItem (54, "[metro]\tMetronome Tick Pulse ([metro 500])", true);
    m.addItem (66, "[delay]\tControl Bang Delay ([delay 250])", true);
    m.addItem (67, "[pipe]\tDilated Control Value FIFO Queue ([pipe 250])", true);
    m.addItem (55, "[send]\tWireless Bus Broadcaster ([s bus1])", true);
    m.addItem (56, "[receive]\tWireless Bus Receiver ([r bus1])", true);

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
    m.addSectionHeader ("--- MATH & BOOLEAN LOGIC COMPARATORS ---");
    m.addItem (11, "[expr]\tControl Expression ($v1, tap('node.prop'))", true);
    m.addItem (12, "[expr~]\tAudio Expression ($v1, tap('node.prop'))", true);
    m.addItem (13, "[fexpr~]\tFilter Recurrent Expression ($y1[-1])", true);
    m.addItem (19, "[v]\tValue Storage Control Node", true);
    m.addItem (20, "[z~]\t1-Sample Feedback Delay Node", true);
    m.addItem (21, "[snapshot~]\tAudio-to-Control Snapshot Node", true);
    m.addItem (22, "[+]\tSignal & Control Adder", true);
    m.addItem (23, "[*]\tSignal & Control Multiplier", true);
    m.addItem (57, "[==]\tEquality Comparator (a == b)", true);
    m.addItem (58, "[!=]\tInequality Comparator (a != b)", true);
    m.addItem (59, "[>]\tGreater Than Comparator (a > b)", true);
    m.addItem (60, "[<]\tLess Than Comparator (a < b)", true);
    m.addItem (61, "[>=]\tGreater Or Equal (a >= b)", true);
    m.addItem (62, "[<=]\tLess Or Equal (a <= b)", true);
    m.addItem (63, "[&&]\tLogical AND Gate", true);
    m.addItem (64, "[||]\tLogical OR Gate", true);
    m.addItem (65, "[!]\tLogical NOT Inverter", true);

    m.addSeparator();
    m.addSectionHeader ("--- TABLES & ARRAY DATA NODES ---");
    m.addItem (26, "[table]\tInteractive Table / Array Buffer", true);
    m.addItem (27, "[tabread~]\tTable Sample / Pitch Reader", true);
    m.addItem (28, "[tabwrite~]\tLive Audio/Data Table Recorder", true);
    m.addItem (29, "[tabosc4~]\t4-Pt Wavetable Oscillator", true);

    m.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (juce::Rectangle<int> (static_cast<int>(spawnPos.x), static_cast<int>(spawnPos.y), 1, 1)),
        [this, spawnPos] (int result) {
            std::string typeName;
            if (result == 1) typeName = "time.warp";
            else if (result == 2) typeName = "time.retro";
            else if (result == 3) typeName = "time.quantize";
            else if (result == 4) typeName = "time.metro";
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
            else if (result == 24) typeName = "time.stasis";
            else if (result == 25) typeName = "time.singularity";
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
            else if (result == 40) typeName = "time.future";
            else if (result == 41) typeName = "time.transport";
            else if (result == 42) typeName = "fbdrum~";
            else if (result == 43) typeName = "drumseq";
            else if (result == 44) typeName = "time.+";
            else if (result == 45) typeName = "time.-";
            else if (result == 46) typeName = "time.*";
            else if (result == 47) typeName = "time./";
            else if (result == 48) typeName = "time.expr";
            else if (result == 49) typeName = "toggle";
            else if (result == 50) typeName = "slider";
            else if (result == 51) typeName = "radio";
            else if (result == 52) typeName = "spigot";
            else if (result == 53) typeName = "select";
            else if (result == 54) typeName = "metro";
            else if (result == 55) typeName = "send";
            else if (result == 56) typeName = "receive";
            else if (result == 57) typeName = "==";
            else if (result == 58) typeName = "!=";
            else if (result == 59) typeName = ">";
            else if (result == 60) typeName = "<";
            else if (result == 61) typeName = ">=";
            else if (result == 62) typeName = "<=";
            else if (result == 63) typeName = "&&";
            else if (result == 64) typeName = "||";
            else if (result == 65) typeName = "!";
            else if (result == 66) typeName = "delay";
            else if (result == 67) typeName = "pipe";

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
    // 8a. Zoom In (Cmd+= / Cmd++ / Cmd+NumpadPlus)
    if (isCmdOrCtrl && (key.getKeyCode() == '=' || key.getKeyCode() == '+' || key.getKeyCode() == juce::KeyPress::numberPadAdd || key.getTextCharacter() == '+' || key.getTextCharacter() == '='))
    {
        zoomIn();
        return true;
    }

    // 8b. Zoom Out (Cmd-- / Cmd+_ / Cmd+NumpadMinus)
    if (isCmdOrCtrl && (key.getKeyCode() == '-' || key.getKeyCode() == '_' || key.getKeyCode() == juce::KeyPress::numberPadSubtract || key.getTextCharacter() == '-' || key.getTextCharacter() == '_'))
    {
        zoomOut();
        return true;
    }

    // 8c. Reset Zoom 100% (Cmd-0)
    if (isCmdOrCtrl && (key.getKeyCode() == '0' || key.getTextCharacter() == '0'))
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

    // Quit Workstation Application (Cmd+Q / Ctrl+Q)
    if (isCmdOrCtrl && (key.getKeyCode() == 'Q' || key.getKeyCode() == 'q'))
    {
        requestExit ([] {
            if (auto* app = juce::JUCEApplication::getInstance())
                app->systemRequestedQuit();
        });
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
    if (oscServer)
    {
        oscServer->stopServer();
        oscServer = nullptr;
    }
    inspectorViewport.setViewedComponent (nullptr, false);
}

void RelativisticCanvasComponent::timerCallback()
{
    double hzSec = nodeGraph.getCurrentCausalityHorizonSec();
    btnHorizonReadout.setButtonText ("HORIZON: +" + juce::String (hzSec, 3) + "s");

    if (isConsoleVisible)
    {
        updateConsoleDrawer();
    }

    repaint();
}float RelativisticCanvasComponent::getNodeWidth (const RelativisticNode& node) const
{
    if (node.getTypeName() == "time.scope" || node.getTypeName() == "time.display" || node.getTypeName() == "time.monitor")
        return 220.0f;
    if (node.getTypeName() == "time.xy" || node.getTypeName() == "xy" || node.getTypeName() == "xy~" || node.getTypeName() == "plot.xy")
        return 220.0f;
    if (node.getTypeName() == "spectrometer~" || node.getTypeName() == "spectrum~" || node.getTypeName() == "fft~")
        return 240.0f;
    if (node.getTypeName() == "table")
        return 200.0f;
    if (node.getTypeName() == "out~" || node.getTypeName() == "out")
        return 240.0f;
    if (node.getTypeName() == "seq" || node.getTypeName() == "step")
        return 260.0f;
    if (node.getTypeName() == "meter~" || node.getTypeName() == "vu~")
        return 180.0f;
    if (node.getTypeName() == "number~" || node.getTypeName() == "num~")
        return 140.0f;
    if (node.getTypeName() == "print" || node.getTypeName() == "monitor")
        return 240.0f;

    juce::Font labelFont = FontManager::getInstance().getOxaniumFont (14.0f, true);

    float textW = getTextWidth (labelFont, node.getLabel()) + 44.0f;

    float inletsTotalW = 0.0f;
    for (const auto& in : node.getInlets())
    {
        float w = static_cast<float>(in.name.length()) * 7.5f + 20.0f;
        inletsTotalW += std::max (54.0f, w);
    }

    float outletsTotalW = 0.0f;
    for (const auto& out : node.getOutlets())
    {
        float w = static_cast<float>(out.name.length()) * 7.5f + 20.0f;
        outletsTotalW += std::max (54.0f, w);
    }

    float portsW = std::max (inletsTotalW, outletsTotalW) + 40.0f;

    return std::max ({ 170.0f, textW, portsW });
}

float RelativisticCanvasComponent::getNodeHeight (const RelativisticNode& node) const
{
    float baseH = 68.0f;
    if (node.getTypeName() == "time.transport")
        baseH = 76.0f;
    else if (node.getTypeName() == "time.scope" || node.getTypeName() == "time.display" || node.getTypeName() == "time.monitor")
        baseH = 120.0f;
    else if (node.getTypeName() == "time.xy" || node.getTypeName() == "xy" || node.getTypeName() == "xy~" || node.getTypeName() == "plot.xy")
        baseH = 120.0f;
    else if (node.getTypeName() == "spectrometer~" || node.getTypeName() == "spectrum~" || node.getTypeName() == "fft~")
        baseH = 120.0f;
    else if (node.getTypeName() == "table")
        baseH = 85.0f;
    else if (node.getTypeName() == "out~" || node.getTypeName() == "out")
        baseH = 110.0f;
    else if (node.getTypeName() == "seq" || node.getTypeName() == "step")
        baseH = 100.0f;
    else if (node.getTypeName() == "meter~" || node.getTypeName() == "vu~")
        baseH = 78.0f;
    else if (node.getTypeName() == "number~" || node.getTypeName() == "num~")
        baseH = 60.0f;
    else if (node.getTypeName() == "print" || node.getTypeName() == "monitor")
        baseH = 120.0f;
    else if (node.getInlets().size() > 3 || node.getOutlets().size() > 3)
        baseH = 74.0f;

    if (node.isShowDelaylineEnabled()) baseH += 128.0f;
    if (node.isShowPipeEnabled()) baseH += 128.0f;

    return baseH;
}

void RelativisticCanvasComponent::drawDelaylineDots (juce::Graphics& g, const RelativisticNode& node, float x, float y, float w, float h) const
{
    g.setColour (juce::Colour (0xff070a12));
    g.fillRoundedRectangle (x, y, w, h, 3.0f);
    g.setColour (juce::Colour (0xff1e293b));
    g.drawRoundedRectangle (x, y, w, h, 3.0f, 1.0f);

    g.setColour (juce::Colour (0xff06b6d4));
    g.setFont (FontManager::getInstance().getOxaniumFont (9.5f, true));
    g.drawText ("AUDIO DELAYLINE (128px SAMPLE DOTS)", x + 6.0f, y + 2.0f, w - 12.0f, 12.0f, juce::Justification::centredLeft);

    float midY = y + h * 0.5f + 4.0f;
    g.setColour (juce::Colour (0xff334155).withAlpha (0.4f));
    g.drawDashedLine (juce::Line<float> (x + 4.0f, midY, x + w - 4.0f, midY), nullptr, 0, 1.0f);

    float graphW = w - 12.0f;
    float graphX = x + 6.0f;
    int numDots = std::min (160, static_cast<int>(graphW));
    if (numDots <= 0) return;

    std::vector<float> dots;
    int writePos = 0;
    int bufSamples = 0;
    node.getAudioDelayLine().getSampleSnapshot (dots, numDots, writePos, bufSamples);

    if (bufSamples > 0 && !dots.empty())
    {
        g.setColour (juce::Colour (0xff06b6d4));
        for (int i = 0; i < numDots; ++i)
        {
            float sampleVal = dots[i];
            float px = graphX + (static_cast<float>(i) / static_cast<float>(numDots - 1)) * graphW;
            float py = midY - std::clamp (sampleVal, -1.5f, 1.5f) * ((h - 20.0f) * 0.45f);

            g.fillEllipse (px - 1.0f, py - 1.0f, 2.5f, 2.5f);
        }

        float writeX = graphX + (static_cast<float>(writePos) / static_cast<float>(bufSamples)) * graphW;
        g.setColour (juce::Colour (0xfff59e0b));
        g.drawLine (writeX, y + 14.0f, writeX, y + h - 4.0f, 1.2f);
    }
}

void RelativisticCanvasComponent::drawControlPipeDots (juce::Graphics& g, const RelativisticNode& node, float x, float y, float w, float h) const
{
    g.setColour (juce::Colour (0xff070a12));
    g.fillRoundedRectangle (x, y, w, h, 3.0f);
    g.setColour (juce::Colour (0xff1e293b));
    g.drawRoundedRectangle (x, y, w, h, 3.0f, 1.0f);

    g.setColour (juce::Colour (0xff8b5cf6));
    g.setFont (FontManager::getInstance().getOxaniumFont (9.5f, true));
    g.drawText ("CONTROL MESSAGE PIPE (128px EVENT DOTS)", x + 6.0f, y + 2.0f, w - 12.0f, 12.0f, juce::Justification::centredLeft);

    float axisY = y + h * 0.7f;
    g.setColour (juce::Colour (0xff334155).withAlpha (0.4f));
    g.drawLine (x + 4.0f, axisY, x + w - 4.0f, axisY, 1.0f);

    auto msgs = node.getControlMessagePipe().getSnapshotMessages();

    float graphW = w - 12.0f;
    float graphX = x + 6.0f;
    double currentTau = node.getLocalCoordinateTime();

    if (!msgs.empty())
    {
        for (const auto& msg : msgs)
        {
            double diffTau = msg.targetTau - currentTau;
            float normX = std::clamp (static_cast<float>(diffTau + 2.0) / 4.0f, 0.0f, 1.0f);
            float px = graphX + normX * graphW;
            float py = axisY - std::clamp (msg.value / 127.0f, 0.0f, 1.0f) * (h * 0.5f) - 4.0f;

            if (msg.isBang)
            {
                g.setColour (juce::Colour (0xfff59e0b));
                g.drawEllipse (px - 3.0f, py - 3.0f, 6.0f, 6.0f, 1.5f);
            }
            else
            {
                g.setColour (juce::Colour (0xff8b5cf6));
                g.fillEllipse (px - 2.0f, py - 2.0f, 4.0f, 4.0f);
            }
        }
    }

    float currentX = graphX + 0.5f * graphW;
    g.setColour (juce::Colour (0xff06b6d4));
    g.drawLine (currentX, y + 14.0f, currentX, y + h - 4.0f, 1.2f);
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

void RelativisticCanvasComponent::alignSelectedLeft()
{
    if (selectedNodeIds.size() < 2) return;
    float minX = 99999.0f;
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            minX = std::min (minX, n->getX());
    }
    nodeGraph.pushUndoState();
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            n->setPosition (minX, n->getY());
    }
    repaint();
}

void RelativisticCanvasComponent::alignSelectedRight()
{
    if (selectedNodeIds.size() < 2) return;
    float maxRight = -99999.0f;
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            maxRight = std::max (maxRight, n->getX() + getNodeWidth (*n));
    }
    nodeGraph.pushUndoState();
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            n->setPosition (maxRight - getNodeWidth (*n), n->getY());
    }
    repaint();
}

void RelativisticCanvasComponent::alignSelectedTop()
{
    if (selectedNodeIds.size() < 2) return;
    float minY = 99999.0f;
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            minY = std::min (minY, n->getY());
    }
    nodeGraph.pushUndoState();
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            n->setPosition (n->getX(), minY);
    }
    repaint();
}

void RelativisticCanvasComponent::alignSelectedBottom()
{
    if (selectedNodeIds.size() < 2) return;
    float maxBottom = -99999.0f;
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            maxBottom = std::max (maxBottom, n->getY() + getNodeHeight (*n));
    }
    nodeGraph.pushUndoState();
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            n->setPosition (n->getX(), maxBottom - getNodeHeight (*n));
    }
    repaint();
}

void RelativisticCanvasComponent::distributeSelectedHorizontally()
{
    if (selectedNodeIds.size() < 3) return;
    std::vector<std::shared_ptr<RelativisticNode>> nodes;
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            nodes.push_back (n);
    }
    std::sort (nodes.begin(), nodes.end(), [] (const auto& a, const auto& b) { return a->getX() < b->getX(); });

    float minX = nodes.front()->getX();
    float maxX = nodes.back()->getX();
    float step = (maxX - minX) / static_cast<float>(nodes.size() - 1);

    nodeGraph.pushUndoState();
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        nodes[i]->setPosition (minX + static_cast<float>(i) * step, nodes[i]->getY());
    }
    repaint();
}

void RelativisticCanvasComponent::distributeSelectedVertically()
{
    if (selectedNodeIds.size() < 3) return;
    std::vector<std::shared_ptr<RelativisticNode>> nodes;
    for (int id : selectedNodeIds)
    {
        if (auto n = nodeGraph.getNodeById (id))
            nodes.push_back (n);
    }
    std::sort (nodes.begin(), nodes.end(), [] (const auto& a, const auto& b) { return a->getY() < b->getY(); });

    float minY = nodes.front()->getY();
    float maxY = nodes.back()->getY();
    float step = (maxY - minY) / static_cast<float>(nodes.size() - 1);

    nodeGraph.pushUndoState();
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        nodes[i]->setPosition (nodes[i]->getX(), minY + static_cast<float>(i) * step);
    }
    repaint();
}

void RelativisticCanvasComponent::autoTidyLayout()
{
    const auto& nodes = nodeGraph.getNodes();
    if (nodes.empty()) return;

    nodeGraph.pushUndoState();

    std::map<int, int> inDegree;
    std::map<int, std::vector<int>> adj;
    std::map<int, int> nodeLevel;

    for (const auto& n : nodes)
    {
        inDegree[n->getId()] = 0;
        nodeLevel[n->getId()] = 0;
    }

    for (const auto& conn : nodeGraph.getConnections())
    {
        adj[conn.sourceNodeId].push_back (conn.destNodeId);
        inDegree[conn.destNodeId]++;
    }

    std::queue<int> q;
    for (const auto& pair : inDegree)
    {
        if (pair.second == 0)
        {
            q.push (pair.first);
            nodeLevel[pair.first] = 0;
        }
    }

    while (!q.empty())
    {
        int curr = q.front();
        q.pop();

        int currLvl = nodeLevel[curr];
        for (int neighbor : adj[curr])
        {
            nodeLevel[neighbor] = std::max (nodeLevel[neighbor], currLvl + 1);
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0)
                q.push (neighbor);
        }
    }

    std::map<int, std::vector<std::shared_ptr<RelativisticNode>>> levelNodes;
    for (const auto& n : nodes)
    {
        levelNodes[nodeLevel[n->getId()]].push_back (n);
    }

    float startX = 80.0f;
    float startY = 80.0f;
    float levelSpacingY = 160.0f;
    float itemSpacingX = 260.0f;

    for (auto& pair : levelNodes)
    {
        int lvl = pair.first;
        auto& lvlList = pair.second;

        float currY = startY + static_cast<float>(lvl) * levelSpacingY;
        for (size_t i = 0; i < lvlList.size(); ++i)
        {
            float currX = startX + static_cast<float>(i) * itemSpacingX;
            lvlList[i]->setPosition (currX, currY);
        }
    }

    fitAllNodesInView();
    repaint();
}

void RelativisticCanvasComponent::showNodeContextMenu (const std::shared_ptr<RelativisticNode>& targetNode, juce::Point<float> mousePos)
{
    juce::PopupMenu m;
    std::string header = targetNode ? ("NODE: " + targetNode->getLabel()) : "CANVAS CONTEXT MENU";
    m.addSectionHeader (header);

    if (targetNode)
    {
        m.addItem (1, "Duplicate (Cmd+D)", true);
        m.addItem (2, "Cut (Cmd+X)", true);
        m.addItem (3, "Copy (Cmd+C)", true);
        m.addItem (4, "Delete Node", true);
        m.addSeparator();

        juce::PopupMenu subAlign;
        subAlign.addItem (10, "Align Left", selectedNodeIds.size() >= 2);
        subAlign.addItem (11, "Align Right", selectedNodeIds.size() >= 2);
        subAlign.addItem (12, "Align Top", selectedNodeIds.size() >= 2);
        subAlign.addItem (13, "Align Bottom", selectedNodeIds.size() >= 2);
        subAlign.addSeparator();
        subAlign.addItem (14, "Distribute Horizontally", selectedNodeIds.size() >= 3);
        subAlign.addItem (15, "Distribute Vertically", selectedNodeIds.size() >= 3);
        m.addSubMenu ("Align Selected", subAlign);

        m.addItem (20, "Auto-Tidy Patch Layout (Cmd+Shift+R)", true);
        m.addSeparator();

        bool dlOn = targetNode->isShowDelaylineEnabled();
        m.addItem (30, dlOn ? "Hide Audio Delayline (128px)" : "Show Audio Delayline (128px)", true);

        bool pipeOn = targetNode->isShowPipeEnabled();
        m.addItem (31, pipeOn ? "Hide Control Message Pipe (128px)" : "Show Control Message Pipe (128px)", true);
    }
    else
    {
        m.addItem (100, "Add New Object... (N)", true);
        m.addItem (101, "Paste (Cmd+V)", clipboardTree.isValid());
        m.addItem (20, "Auto-Tidy Patch Layout (Cmd+Shift+R)", true);
        m.addItem (102, "Fit All Nodes in View", true);
        m.addItem (103, "Reset Zoom (100%)", true);
    }

    m.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (juce::Rectangle<int> (mousePos.x, mousePos.y, 1, 1)),
        [this, targetNode] (int res) {
            if (res == 1) btnDuplicate.triggerClick();
            else if (res == 2) {
                if (!selectedNodeIds.empty()) {
                    std::vector<int> sel (selectedNodeIds.begin(), selectedNodeIds.end());
                    clipboardTree = nodeGraph.copyNodes (sel);
                    nodeGraph.cutNodes (sel);
                    selectedNodeIds.clear();
                    repaint();
                }
            }
            else if (res == 3) btnCopy.triggerClick();
            else if (res == 4) {
                if (!selectedNodeIds.empty()) {
                    std::vector<int> sel (selectedNodeIds.begin(), selectedNodeIds.end());
                    nodeGraph.cutNodes (sel);
                    selectedNodeIds.clear();
                    repaint();
                }
            }
            else if (res == 10) alignSelectedLeft();
            else if (res == 11) alignSelectedRight();
            else if (res == 12) alignSelectedTop();
            else if (res == 13) alignSelectedBottom();
            else if (res == 14) distributeSelectedHorizontally();
            else if (res == 15) distributeSelectedVertically();
            else if (res == 20) autoTidyLayout();
            else if (res == 30) {
                if (targetNode) targetNode->setShowDelaylineEnabled (!targetNode->isShowDelaylineEnabled());
                juce::MessageManager::callAsync ([this] { rebuildInspector(); repaint(); });
            }
            else if (res == 31) {
                if (targetNode) targetNode->setShowPipeEnabled (!targetNode->isShowPipeEnabled());
                juce::MessageManager::callAsync ([this] { rebuildInspector(); repaint(); });
            }
            else if (res == 100) spawnInlineObjectEditor ({ getWidth() * 0.4f, getHeight() * 0.4f });
            else if (res == 101) btnPaste.triggerClick();
            else if (res == 102) fitAllNodesInView();
            else if (res == 103) resetZoom();
        });
}

juce::Rectangle<float> RelativisticCanvasComponent::getMinimapBounds() const
{
    float miniW = 180.0f;
    float miniH = 110.0f;
    float miniX = 15.0f;
    float miniY = static_cast<float>(getHeight()) - miniH - 55.0f;
    return { miniX, miniY, miniW, miniH };
}

void RelativisticCanvasComponent::drawMinimap (juce::Graphics& g) const
{
    if (!showMinimap) return;

    auto bounds = getMinimapBounds();
    g.setColour (juce::Colour (0xee0b1322));
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (juce::Colour (0xff1e293b));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    g.setColour (juce::Colour (0xff06b6d4));
    g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, true));
    g.drawText ("MINIMAP RADAR", bounds.getX() + 6.0f, bounds.getY() + 3.0f, bounds.getWidth() - 12.0f, 12.0f, juce::Justification::centredLeft);

    const auto& nodes = nodeGraph.getNodes();
    if (nodes.empty()) return;

    float minX = 99999.0f, minY = 99999.0f, maxX = -99999.0f, maxY = -99999.0f;
    for (const auto& n : nodes)
    {
        minX = std::min (minX, n->getX());
        minY = std::min (minY, n->getY());
        maxX = std::max (maxX, n->getX() + getNodeWidth (*n));
        maxY = std::max (maxY, n->getY() + getNodeHeight (*n));
    }

    float worldW = std::max (600.0f, maxX - minX + 200.0f);
    float worldH = std::max (400.0f, maxY - minY + 200.0f);
    float innerX = bounds.getX() + 6.0f;
    float innerY = bounds.getY() + 18.0f;
    float innerW = bounds.getWidth() - 12.0f;
    float innerH = bounds.getHeight() - 24.0f;

    float scaleX = innerW / worldW;
    float scaleY = innerH / worldH;

    for (const auto& n : nodes)
    {
        float mx = innerX + (n->getX() - minX + 100.0f) * scaleX;
        float my = innerY + (n->getY() - minY + 100.0f) * scaleY;
        float mw = std::max (3.0f, getNodeWidth (*n) * scaleX);
        float mh = std::max (2.0f, getNodeHeight (*n) * scaleY);

        bool isTimeObj = n->getTypeName().rfind ("time.", 0) == 0;
        bool isAudioObj = n->getTypeName().find ("~") != std::string::npos;
        juce::Colour nodeCol = isTimeObj ? juce::Colour (0xff8b5cf6) : (isAudioObj ? juce::Colour (0xff06b6d4) : juce::Colour (0xfff59e0b));

        g.setColour (nodeCol.withAlpha (0.85f));
        g.fillRect (mx, my, mw, mh);
    }

    float canvasW = std::max (100.0f, static_cast<float>(getWidth()) - 340.0f);
    float canvasH = std::max (100.0f, static_cast<float>(getHeight()) - 100.0f);

    float viewWorldX = (-panX) / zoomLevel;
    float viewWorldY = (-panY + 45.0f) / zoomLevel;
    float viewWorldW = canvasW / zoomLevel;
    float viewWorldH = canvasH / zoomLevel;

    float vx = innerX + (viewWorldX - minX + 100.0f) * scaleX;
    float vy = innerY + (viewWorldY - minY + 100.0f) * scaleY;
    float vw = std::clamp (viewWorldW * scaleX, 10.0f, innerW);
    float vh = std::clamp (viewWorldH * scaleY, 8.0f, innerH);

    g.setColour (juce::Colour (0xff38bdf8));
    g.drawRect (vx, vy, vw, vh, 1.2f);
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

void RelativisticCanvasComponent::paint (juce::Graphics& g)
{
    // Dark Carbon Canvas Background
    g.fillAll (juce::Colour (0xff070a12));

    const float inspectorW = 320.0f;
    const float inspectorX = std::max (0.0f, static_cast<float>(getWidth()) - inspectorW);
    const float canvasH = std::max (0.0f, static_cast<float>(getHeight()) - 95.0f);

    // 1. Render Canvas Elements inside STRICT Clipped Bounds
    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (juce::Rectangle<int> (0, 45, static_cast<int>(inspectorX), static_cast<int>(canvasH)));

        // Sci-Fi Micro-Grid Dot Matrix
        if (showGrid)
        {
            g.setColour (juce::Colour (0x1a94a3b8));
            float step = gridSize;
            int startX = static_cast<int>(std::fmod (panX, step));
            if (startX < 0) startX += static_cast<int>(step);
            int startY = static_cast<int>(std::fmod (panY, step)) + 55;
            if (startY < 55) startY += static_cast<int>(step);

            for (int gx = startX; gx < static_cast<int>(inspectorX); gx += static_cast<int>(step))
            {
                for (int gy = startY; gy < static_cast<int>(canvasH + 45.0f); gy += static_cast<int>(step))
                {
                    g.fillEllipse (static_cast<float>(gx), static_cast<float>(gy), 1.5f, 1.5f);
                }
            }
        }

        // Draw Connections
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

        // Draw Dragging Cable
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

        // Draw Canvas Nodes
        for (const auto& node : nodeGraph.getNodes())
        {
            drawNode (g, node);
            if (isDebugMode) drawNodeDebugOverlay (g, *node);
        }

        if (isDebugMode) drawDebugOverlay (g);

        // Draw Rubberband Marquee Selection Box
        if (isMarqueeDragging)
        {
            g.setColour (juce::Colour (0x3306b6d4)); // Translucent Cyan Fill
            g.fillRect (marqueeRect);
            g.setColour (juce::Colour (0xff06b6d4)); // Cyan Stroke
            g.drawRect (marqueeRect, 1.5f);
        }

        // Draw Slate Sci-Fi Port Hover Inspection Tooltip Badge
        if (hoveredPort.nodeId != 0 && !isDraggingCable)
        {
            juce::String tipTitle = (hoveredPort.isInlet ? "INLET: " : "OUTLET: ") + juce::String (hoveredPort.portName);
            juce::String typeText = juce::String (hoveredPort.signalTypeName);
            juce::String valText  = juce::String (hoveredPort.routedValueText);

            float badgeW = 210.0f;
            float badgeH = 52.0f;
            float badgeX = std::min (hoveredPort.pos.x + 14.0f, inspectorX - badgeW - 10.0f);
            float badgeY = std::max (50.0f, hoveredPort.pos.y - 26.0f);

            juce::Rectangle<float> badgeRect (badgeX, badgeY, badgeW, badgeH);

            g.setColour (juce::Colour (0xf40b0f19));
            g.fillRoundedRectangle (badgeRect, 6.0f);
            g.setColour (juce::Colour (0xff38bdf8));
            g.drawRoundedRectangle (badgeRect, 6.0f, 1.5f);

            g.setColour (juce::Colour (0xfff59e0b));
            g.setFont (FontManager::getInstance().getOxaniumFont (11.0f, true));
            g.drawText (tipTitle, badgeRect.getX() + 8.0f, badgeRect.getY() + 4.0f, badgeW - 16.0f, 16.0f, juce::Justification::left);

            g.setColour (juce::Colour (0xff94a3b8));
            g.setFont (FontManager::getInstance().getOxaniumFont (9.5f, false));
            g.drawText (typeText, badgeRect.getX() + 8.0f, badgeRect.getY() + 19.0f, badgeW - 16.0f, 14.0f, juce::Justification::left);

            g.setColour (juce::Colour (0xfff8fafc));
            g.setFont (FontManager::getInstance().getOxaniumFont (10.5f, true));
            g.drawText (valText, badgeRect.getX() + 8.0f, badgeRect.getY() + 32.0f, badgeW - 16.0f, 16.0f, juce::Justification::left);
        }
    }

    // 2. Top Header Toolbar Panel (On Top of Canvas)
    g.setColour (juce::Colour (0xff0f172a));
    g.fillRect (0, 0, getWidth(), 45);
    g.setColour (juce::Colour (0xff1e293b));
    g.drawHorizontalLine (45, 0.0f, static_cast<float>(getWidth()));

    // 3. Right Side Dual Inspector Panel Background (On Top of Canvas)
    g.setColour (juce::Colour (0xff0d1322));
    g.fillRect (inspectorX, 45.0f, inspectorW, canvasH);
    g.setColour (juce::Colour (0xff1e293b));
    g.drawVerticalLine (static_cast<int>(inspectorX), 45.0f, static_cast<float>(getHeight() - 50));

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
    if (isEditingDraftObject && draftObjectEditor)
    {
        float boxX = static_cast<float>(draftObjectEditor->getX());
        float boxY = static_cast<float>(draftObjectEditor->getY());
        float boxW = static_cast<float>(draftObjectEditor->getWidth());
        float boxH = static_cast<float>(draftObjectEditor->getHeight());

        // Outer glowing halo around active draft box
        juce::Colour borderCol = isDraftObjectInvalid ? juce::Colour (0xffef4444) : juce::Colour (0xfff59e0b);
        g.setColour (borderCol.withAlpha (0.25f));
        g.fillRoundedRectangle (boxX - 4.0f, boxY - 4.0f, boxW + 8.0f, boxH + 8.0f, 7.0f);
        g.setColour (borderCol);
        g.drawRoundedRectangle (boxX - 1.0f, boxY - 1.0f, boxW + 2.0f, boxH + 2.0f, 5.0f, 1.5f);

        // Preview Inlet & Outlet rings
        g.setColour (juce::Colour (0xff8b5cf6)); // Violet Time Ring
        g.fillEllipse (boxX + 12.0f, boxY - 4.0f, 7.0f, 7.0f);
        g.setColour (juce::Colour (0xfff59e0b)); // Gold Control Ring
        g.fillEllipse (boxX + 32.0f, boxY - 4.0f, 7.0f, 7.0f);
        g.setColour (juce::Colour (0xff06b6d4)); // Cyan Outlet Ring
        g.fillEllipse (boxX + 22.0f, boxY + boxH - 3.0f, 7.0f, 7.0f);

        if (isDraftObjectInvalid)
        {
            // Invalid Object Warning Tag Below Box
            float alertY = boxY + boxH + 6.0f;
            float alertW = 220.0f;
            float alertH = 24.0f;

            g.setColour (juce::Colour (0xee7f1d1d));
            g.fillRoundedRectangle (boxX, alertY, alertW, alertH, 4.0f);
            g.setColour (juce::Colour (0xffef4444));
            g.drawRoundedRectangle (boxX, alertY, alertW, alertH, 4.0f, 1.0f);

            g.setColour (juce::Colour (0xffffffff));
            g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
            g.drawText ("UNKNOWN OBJECT TYPE", boxX + 6, alertY, alertW - 12, alertH, juce::Justification::centredLeft);
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
    btnMenuEdit.setBounds    (menuX + (menuW + 4),           menuY, menuW, menuH);
    btnMenuView.setBounds    (menuX + (menuW + 4) * 2,       menuY, menuW, menuH);
    btnMenuObjects.setBounds (menuX + (menuW + 4) * 3,       menuY, menuW + 15, menuH);
    btnMenuAudio.setBounds   (menuX + (menuW + 4) * 3 + menuW + 20, menuY, menuW + 5, menuH);
    btnMenuPool.setBounds    (menuX + (menuW + 4) * 3 + (menuW + 20) * 2 - 10, menuY, menuW + 5, menuH);
    btnMenuHelp.setBounds    (menuX + (menuW + 4) * 3 + (menuW + 20) * 3 - 20, menuY, menuW + 5, menuH);

    // Right-Aligned Top Header Control Buttons
    btnAudioPower.setBounds (getWidth() - 165, 6, 150, 32);
    btnSavePatch.setBounds (getWidth() - 255, 6, 85, 32);
    btnLoadPatch.setBounds (getWidth() - 345, 6, 85, 32);
    btnRemoveCable.setBounds (getWidth() - 425, 6, 75, 32);
    btnToggleDebugMode.setBounds (getWidth() - 515, 6, 85, 32);
    btnToggleConsole.setBounds (getWidth() - 625, 6, 105, 32);

    if (isConsoleVisible)
    {
        const int consoleW = std::max (200, getWidth() - 340);
        const int consoleH = 140;
        const int consoleY = getHeight() - 48 - consoleH - 6;
        consoleEditor.setBounds (12, consoleY, consoleW, consoleH);
        btnClearConsole.setBounds (12 + consoleW - 75, consoleY + 6, 65, 24);
    }

    // Bottom Palette Toolbar (Streamlined Pure Data-Style)
    const float paletteY = getHeight() - 48.0f;
    const float paletteH = 36.0f;

    btnAddObject.setBounds (12, static_cast<int>(paletteY), 160, static_cast<int>(paletteH));
    btnToggleCord.setBounds (180, static_cast<int>(paletteY), 140, static_cast<int>(paletteH));
    btnClear.setBounds (328, static_cast<int>(paletteY), 110, static_cast<int>(paletteH));
    btnHorizonReadout.setBounds (446, static_cast<int>(paletteY), 145, static_cast<int>(paletteH));
    btnResetHorizon.setBounds (599, static_cast<int>(paletteY), 135, static_cast<int>(paletteH));

    updateDraftObjectBounds();

    // Right Side Dual Inspector Bounds
    const float inspectorW = 320.0f;
    const float inspectorX = getWidth() - inspectorW;
    const float inspectorTop = 85.0f;

    inspectorTitleLabel.setBounds (inspectorX + 15, 52, inspectorW - 30, 24);

    btnTabTopDown.setBounds (inspectorX + 15, inspectorTop, 140, 28);
    btnTabBottomUp.setBounds (inspectorX + 160, inspectorTop, 145, 28);

    if (!isBottomUpMode)
    {
        // TOP-DOWN Mode (Scrollable Inspector Controls inside inspectorViewport)
        float rowY = 0.0f;
        const float compW = inspectorW - 25.0f;

        int primaryId = !selectedNodeIds.empty() ? *selectedNodeIds.begin() : 0;
        auto node = nodeGraph.getNodeById (primaryId);
        std::string typeName = node ? node->getTypeName() : "";
        bool isExprType = (typeName == "expr" || typeName == "expr~" || typeName == "fexpr~");

        if (isExprType)
        {
            exprFormulaLabel.setBounds (5, static_cast<int>(rowY), static_cast<int>(compW), 18);
            exprFormulaEditor.setBounds (5, static_cast<int>(rowY + 20), static_cast<int>(compW), 26);
            btnApplyExprFormula.setBounds (5, static_cast<int>(rowY + 50), static_cast<int>(compW), 26);

            exprFormulaLabel.setVisible (true);
            exprFormulaEditor.setVisible (true);
            btnApplyExprFormula.setVisible (true);

            rowY += 84.0f;
        }
        else
        {
            exprFormulaLabel.setVisible (false);
            exprFormulaEditor.setVisible (false);
            btnApplyExprFormula.setVisible (false);
        }

        for (auto& row : propertyRows)
        {
            if (row.label) row.label->setBounds (5, static_cast<int>(rowY), static_cast<int>(compW), 18);
            if (row.slider) row.slider->setBounds (5, static_cast<int>(rowY + 20), static_cast<int>(compW - 160), 24);
            if (row.btnToggle) row.btnToggle->setBounds (5, static_cast<int>(rowY + 20), static_cast<int>(compW - 160), 24);
            if (row.symbolEditor) row.symbolEditor->setBounds (5, static_cast<int>(rowY + 20), static_cast<int>(compW - 160), 24);
            if (row.btnModInlet) row.btnModInlet->setBounds (static_cast<int>(compW - 150), static_cast<int>(rowY + 20), 75, 24);
            if (row.btnTapValue) row.btnTapValue->setBounds (static_cast<int>(compW - 70), static_cast<int>(rowY + 20), 70, 24);
            if (row.exprEditor) row.exprEditor->setBounds (5, static_cast<int>(rowY + 46), static_cast<int>(compW), 20);

            if (row.label) row.label->setVisible (true);
            if (row.slider) row.slider->setVisible (true);
            if (row.btnToggle) row.btnToggle->setVisible (true);
            if (row.symbolEditor) row.symbolEditor->setVisible (true);
            if (row.btnModInlet) row.btnModInlet->setVisible (true);
            if (row.btnTapValue) row.btnTapValue->setVisible (true);
            if (row.exprEditor) row.exprEditor->setVisible (true);

            rowY += 72.0f;
        }

        for (auto& mBtn : methodButtons)
        {
            if (mBtn)
            {
                mBtn->setBounds (5, static_cast<int>(rowY), static_cast<int>(compW), 26);
                mBtn->setVisible (true);
                rowY += 32.0f;
            }
        }

        // Layout Incoming Connections
        if (incomingSectionHeader)
        {
            incomingSectionHeader->setBounds (5, static_cast<int>(rowY + 5), static_cast<int>(compW), 20);
            incomingSectionHeader->setVisible (true);
            rowY += 28.0f;
        }

        for (auto& cRow : connectionRows)
        {
            if (cRow.isIncoming)
            {
                if (cRow.label)
                {
                    cRow.label->setBounds (5, static_cast<int>(rowY), static_cast<int>(cRow.btnRemoveWire ? (compW - 75) : compW), 22);
                    cRow.label->setVisible (true);
                }
                if (cRow.btnRemoveWire)
                {
                    cRow.btnRemoveWire->setBounds (static_cast<int>(compW - 70), static_cast<int>(rowY), 70, 22);
                    cRow.btnRemoveWire->setVisible (true);
                }
                rowY += 25.0f;
            }
        }

        // Layout Outgoing Connections
        if (outgoingSectionHeader)
        {
            outgoingSectionHeader->setBounds (5, static_cast<int>(rowY + 5), static_cast<int>(compW), 20);
            outgoingSectionHeader->setVisible (true);
            rowY += 28.0f;
        }

        for (auto& cRow : connectionRows)
        {
            if (!cRow.isIncoming)
            {
                if (cRow.label)
                {
                    cRow.label->setBounds (5, static_cast<int>(rowY), static_cast<int>(cRow.btnRemoveWire ? (compW - 75) : compW), 22);
                    cRow.label->setVisible (true);
                }
                if (cRow.btnRemoveWire)
                {
                    cRow.btnRemoveWire->setBounds (static_cast<int>(compW - 70), static_cast<int>(rowY), 70, 22);
                    cRow.btnRemoveWire->setVisible (true);
                }
                rowY += 25.0f;
            }
        }

        int totalH = static_cast<int>(rowY + 20.0f);
        inspectorContainer.setBounds (0, 0, static_cast<int>(compW), totalH);
        inspectorViewport.setBounds (static_cast<int>(inspectorX + 10), static_cast<int>(inspectorTop + 35), static_cast<int>(inspectorW - 15), static_cast<int>(getHeight() - (inspectorTop + 35) - 52));
        inspectorViewport.setVisible (true);

        btnInsertTapDropdown.setVisible (false);
        formulaEditor.setVisible (false);
        btnApplyFormula.setVisible (false);
    }
    else
    {
        inspectorViewport.setVisible (false);
        // BOTTOM-UP Mode (Code Math)
        for (auto& row : propertyRows)
        {
            if (row.label) row.label->setVisible (false);
            if (row.slider) row.slider->setVisible (false);
            if (row.btnToggle) row.btnToggle->setVisible (false);
            if (row.symbolEditor) row.symbolEditor->setVisible (false);
            if (row.btnModInlet) row.btnModInlet->setVisible (false);
            if (row.btnTapValue) row.btnTapValue->setVisible (false);
            if (row.exprEditor) row.exprEditor->setVisible (false);
        }
        for (auto& mBtn : methodButtons)
        {
            if (mBtn) mBtn->setVisible (false);
        }

        exprFormulaLabel.setVisible (false);
        exprFormulaEditor.setVisible (false);
        btnApplyExprFormula.setVisible (false);

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

    if (poolDrawer)
    {
        poolDrawer->setBounds (getLocalBounds().reduced (36));
    }

    helpModalOverlay.setBounds (getLocalBounds());
}

void RelativisticCanvasComponent::showNotificationBanner (const std::string& text, bool isWarning)
{
    notificationText = text;
    isNotificationWarning = isWarning;
    notificationExpiryTimeMs = juce::Time::getMillisecondCounter() + 4000;
    repaint();
}

void RelativisticCanvasComponent::drawNotificationBanner (juce::Graphics& g) const
{
    if (notificationText.empty()) return;
    uint32_t now = juce::Time::getMillisecondCounter();
    if (now > notificationExpiryTimeMs) return;

    float bannerW = std::min (640.0f, static_cast<float>(getWidth()) - 40.0f);
    float bannerH = 38.0f;
    float bannerX = (static_cast<float>(getWidth()) - bannerW) * 0.5f;
    float bannerY = 55.0f;

    juce::Colour bgCol = isNotificationWarning ? juce::Colour (0xee7f1d1d) : juce::Colour (0xee065f46);
    juce::Colour borderCol = isNotificationWarning ? juce::Colour (0xffef4444) : juce::Colour (0xff10b981);
    juce::Colour textCol = juce::Colour (0xffffffff);

    g.setColour (bgCol);
    g.fillRoundedRectangle (bannerX, bannerY, bannerW, bannerH, 8.0f);
    g.setColour (borderCol);
    g.drawRoundedRectangle (bannerX, bannerY, bannerW, bannerH, 8.0f, 1.5f);

    juce::Font f = FontManager::getInstance().getOxaniumFont (12.5f, true);
    g.setFont (f);
    g.setColour (textCol);
    g.drawText ((isNotificationWarning ? "[!] " : "[OK] ") + notificationText,
                bannerX + 16.0f, bannerY, bannerW - 32.0f, bannerH, juce::Justification::centredLeft);
}

void RelativisticCanvasComponent::drawDebugOverlay (juce::Graphics& g) const
{
    float hudX = 15.0f;
    float hudY = 55.0f;
    float hudW = 270.0f;

    juce::StringArray lines;
    lines.add ("=== SYSTEM DEBUG TELEMETRY HUD ===");
    lines.add ("Audio Engine: " + juce::String (nodeGraph.isAudioEngineEnabled() ? "ON (ACTIVE)" : "OFF (SAFE)"));
    lines.add ("Active Nodes: " + juce::String (nodeGraph.getNodes().size()) + "  |  Conns: " + juce::String (nodeGraph.getConnections().size()));
    lines.add ("Causality Horizon: +" + juce::String (nodeGraph.getCurrentCausalityHorizonSec(), 4) + "s");
    lines.add ("------------------------------------");
    lines.add ("GLOBAL VARIABLES:");
    for (const auto& kv : nodeGraph.getGlobalVariables())
    {
        lines.add ("  > $" + juce::String (kv.first) + " = " + juce::String (kv.second, 4));
    }
    if (nodeGraph.getGlobalVariables().empty())
    {
        lines.add ("  > $bpm = 120.0");
        lines.add ("  > $t = 0.000s");
        lines.add ("  > $gamma = 1.000x");
    }

    float hudH = lines.size() * 15.0f + 10.0f;

    g.setColour (juce::Colour (0xee070a12));
    g.fillRoundedRectangle (hudX, hudY, hudW, hudH, 6.0f);
    g.setColour (juce::Colour (0xfff59e0b)); // Relativistic Gold Border
    g.drawRoundedRectangle (hudX, hudY, hudW, hudH, 6.0f, 1.2f);

    g.setFont (FontManager::getInstance().getOxaniumFont (11.0f, false));
    float textY = hudY + 5.0f;
    for (int i = 0; i < lines.size(); ++i)
    {
        if (i == 0) g.setColour (juce::Colour (0xfff59e0b));
        else if (i == 1) g.setColour (nodeGraph.isAudioEngineEnabled() ? juce::Colour (0xff22c55e) : juce::Colour (0xffef4444));
        else g.setColour (juce::Colour (0xffcbd5e1));

        g.drawText (lines[i], hudX + 8.0f, textY, hudW - 16.0f, 15.0f, juce::Justification::centredLeft);
        textY += 15.0f;
    }
}

void RelativisticCanvasComponent::drawNodeDebugOverlay (juce::Graphics& g, const RelativisticNode& node) const
{
    float w = getNodeWidth (node);
    float h = getNodeHeight (node);
    float debugY = node.getY() + h + 4.0f;

    juce::Font debugFont = FontManager::getInstance().getOxaniumFont (10.0f, false);
    g.setFont (debugFont);

    juce::StringArray lines;
    lines.add ("DEBUG NODE #" + juce::String (node.getId()) + " [" + node.getTypeName() + "]");
    lines.add ("  > tau (local): " + juce::String (const_cast<RelativisticNode&>(node).updateCoordinateTime (0), 3) + "s");
    lines.add ("  > gamma (eff): " + juce::String (node.getEffectiveGamma(), 3) + "x");

    // Inlets Telemetry
    for (size_t i = 0; i < node.getInlets().size(); ++i)
    {
        const auto& in = node.getInlets()[i];
        juce::String valStr;
        if (in.type == NodePortType::Time) valStr = juce::String (in.timeGamma, 2) + "x (time)";
        else if (in.type == NodePortType::Audio) valStr = juce::String (in.audioData.getMagnitude (0, std::max (1, in.audioData.getNumSamples())), 3) + " (audio peak)";
        else valStr = juce::String (in.controlValue, 3) + " (ctrl)";
        lines.add ("  > in" + juce::String (i) + " (" + juce::String (in.name) + "): " + valStr);
    }

    // Parameters Telemetry
    for (const auto& kv : node.getParameters())
    {
        lines.add ("  > param [" + juce::String (kv.first) + "]: " + juce::String (kv.second, 3));
    }

    float boxW = std::max (w, 195.0f);
    float boxH = lines.size() * 14.0f + 8.0f;

    g.setColour (juce::Colour (0xee090d16)); // Deep Slate Card Fill
    g.fillRoundedRectangle (node.getX(), debugY, boxW, boxH, 4.0f);
    g.setColour (juce::Colour (0xff06b6d4)); // Cyber Cyan Stroke
    g.drawRoundedRectangle (node.getX(), debugY, boxW, boxH, 4.0f, 1.0f);

    float textY = debugY + 4.0f;
    for (int i = 0; i < lines.size(); ++i)
    {
        g.setColour (i == 0 ? juce::Colour (0xff38bdf8) : juce::Colour (0xffcbd5e1));
        g.drawText (lines[i], node.getX() + 6.0f, textY, boxW - 12.0f, 14.0f, juce::Justification::centredLeft);
        textY += 14.0f;
    }
}

} // namespace time_dilation
