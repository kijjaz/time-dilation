#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/RelativisticNodeGraph.h"
#include "../dsp/RelativisticOscServer.h"
#include "AssetPoolDrawerComponent.h"
#include "FontManager.h"

namespace time_dilation
{

enum class CableStyle
{
    Organic, // Natural Physics Gravity Sag
    SmoothS, // Modular Synth S-Curve
    Straight // Pd Classic Straight Line
};

class RelativisticLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RelativisticLookAndFeel();
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override;
};

class RelativisticCanvasComponent : public juce::Component,
                                     public juce::Timer
{
public:
    explicit RelativisticCanvasComponent (RelativisticNodeGraph& graph);
    ~RelativisticCanvasComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    bool keyPressed (const juce::KeyPress& key) override;

    float getNodeWidth (const RelativisticNode& node) const;
    float getNodeHeight (const RelativisticNode& node) const;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    void zoomIn();
    void zoomOut();
    void resetZoom();
    void setZoomLevel (float newZoom, juce::Point<float> anchorPos = { 0, 0 });
    float getZoomLevel() const { return zoomLevel; }
    RelativisticNodeGraph& getNodeGraph() { return nodeGraph; }
    void requestExit (std::function<void()> onProceedExit);
    bool isDirty() const { return hasUnsavedChanges; }


private:
    RelativisticNodeGraph& nodeGraph;
    RelativisticLookAndFeel customLookAndFeel;

    CableStyle cableStyle = CableStyle::Organic;
    bool showGrid = true;
    bool snapToGrid = false;
    float gridSize = 24.0f;
    float zoomLevel = 1.0f;
    std::set<int> selectedNodeIds;
    int selectedConnectionId = 0;
    int draggingNodeId = 0;
    juce::Point<float> dragOffset;

    // Hovered Port Info for Value Tooltips
    struct HoveredPortInfo
    {
        int nodeId = 0;
        int portIdx = 0;
        bool isInlet = true;
        juce::Point<float> pos;
        std::string portName;
        std::string signalTypeName;
        std::string routedValueText;
    };
    HoveredPortInfo hoveredPort;

    // Canvas Viewport Panning State
    float panX = 0.0f;
    float panY = 0.0f;
    bool isCanvasPanning = false;
    juce::Point<float> panStartPos;
    juce::Point<float> initialPanOffset;

    // Rubberband Marquee Selection State
    bool isMarqueeDragging = false;
    juce::Rectangle<float> marqueeRect;

    // Cable Patch Dragging State
    bool isDraggingCable = false;
    int cableSrcNodeId = 0;
    int cableSrcOutletIdx = 0;
    juce::Point<float> cableDragPos;

    // Internal Clipboard Tree
    juce::ValueTree clipboardTree;

    // Edit / Play Mode State
    bool isEditMode = true;
    int valueDragNodeId = 0;
    float valueDragStartVal = 0.0f;
    float valueDragStartMouseY = 0.0f;

    // Top Header Menu Bar Buttons
    juce::TextButton btnMenuFile    { "File" };
    juce::TextButton btnMenuEdit    { "Edit" };
    juce::TextButton btnMenuView    { "View" };
    juce::TextButton btnMenuObjects { "Objects" };
    juce::TextButton btnMenuAudio   { "Audio" };
    juce::TextButton btnMenuPool    { "Pool" };
    juce::TextButton btnMenuHelp    { "Help" };

    void showMenuFile();
    void showMenuEdit();
    void showMenuView();
    void showMenuObjects();
    void showMenuAudio();
    void showPoolDrawer();
    void showMenuHelp();
    void showHelpDialog (const juce::String& topic, const juce::String& content);

    // Top Header Toolbar Buttons
    juce::TextButton btnAudioPower { "AUDIO: OFF (SAFE)" };
    juce::TextButton btnToggleMode { "MODE: EDIT (Cmd+E)" };
    juce::TextButton btnUndo       { "UNDO" };
    juce::TextButton btnRedo       { "REDO" };
    juce::TextButton btnDuplicate  { "DUP (Cmd+D)" };
    juce::TextButton btnCopy       { "COPY (Cmd+C)" };
    juce::TextButton btnPaste      { "PASTE (Cmd+V)" };
    juce::TextButton btnSavePatch  { "SAVE" };
    juce::TextButton btnLoadPatch  { "OPEN" };
    juce::TextButton btnRemoveCable { "DEL" };

    // Cleaned-Up Palette Toolbar & Search Menu
    juce::TextButton btnAddObject   { "+ ADD OBJECT (N)" };
    juce::TextButton btnToggleCord  { "CORDS: ORGANIC" };
    juce::TextButton btnClear       { "CLEAR PATCH" };
    juce::TextButton btnHorizonReadout { "HORIZON: +0.000s" };
    juce::TextButton btnResetHorizon  { "RESET HORIZON" };

    void showObjectSearchMenu (juce::Point<float> spawnPos);

    // Interactive Cmd-1 Object Creation Box & Autocomplete System
    struct AutocompleteItem
    {
        std::string typeName;
        std::string description;
        std::string category;
        std::string keywords;
    };
    std::vector<AutocompleteItem> allRegisteredObjects;
    std::vector<AutocompleteItem> filteredAutocompleteItems;
    int selectedAutocompleteIdx = 0;

    bool isEditingDraftObject = false;
    bool isDraftObjectInvalid = false;
    std::string draftWarningText;
    juce::Point<float> draftObjectCanvasPos;
    std::unique_ptr<juce::TextEditor> draftObjectEditor;
    std::unique_ptr<juce::AlertWindow> activeAlertWindow;

    void initObjectCatalog();
    void spawnInlineObjectEditor (juce::Point<float> spawnCanvasPos);
    void updateDraftObjectBounds();
    void destroyDraftObjectEditor();
    void updateAutocompleteFilter (const juce::String& text);
    void commitDraftObject();

    // Dual Inspector Panel (Top-Down Visual vs Bottom-Up Code)
    bool isBottomUpMode = false;
    juce::Label inspectorTitleLabel  { {}, "INSPECTOR: NO NODE SELECTED" };
    juce::TextButton btnTabTopDown   { "TOP-DOWN (VISUAL)" };
    juce::TextButton btnTabBottomUp  { "BOTTOM-UP (CODE MATH)" };

    struct InspectorPropertyRow
    {
        std::string key;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::TextButton> btnToggle;
        std::unique_ptr<juce::TextEditor> symbolEditor;
        std::unique_ptr<juce::TextEditor> exprEditor;
        std::unique_ptr<juce::TextButton> btnModInlet;
        std::unique_ptr<juce::TextButton> btnTapValue;
    };

    struct InspectorConnectionRow
    {
        int connectionId = 0;
        bool isIncoming = false;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::TextButton> btnRemoveWire;
    };

    std::vector<InspectorPropertyRow> propertyRows;
    std::vector<std::unique_ptr<juce::TextButton>> methodButtons;
    std::vector<InspectorConnectionRow> connectionRows;
    std::unique_ptr<juce::Label> incomingSectionHeader;
    std::unique_ptr<juce::Label> outgoingSectionHeader;

    juce::Viewport inspectorViewport;
    juce::Component inspectorContainer;

    juce::TextButton btnInsertTapDropdown { "TAP SIGNAL POINT" };

    juce::TextEditor inlineLabelEditor;
    int editingNodeId = 0;

    // Port hover inspection & magnetic snapping
    int hoveredPortNodeId = 0;
    bool hoveredPortIsOutlet = false;
    int hoveredPortIdx = -1;
    juce::Point<float> hoveredPortMousePos;

    int snappedInletNodeId = 0;
    int snappedInletIdx = -1;
    juce::Point<float> snappedInletPos;

    juce::TextEditor formulaEditor;
    juce::TextButton btnApplyFormula { "APPLY C++ / MATH FORMULA" };

    std::unique_ptr<AssetPoolDrawerComponent> poolDrawer;

    juce::Label exprFormulaLabel { "exprLabel", "MATH EXPRESSION FORMULA:" };
    juce::TextEditor exprFormulaEditor;
    juce::TextButton btnApplyExprFormula { "APPLY EXPRESSION FORMULA" };

    juce::File currentProjectFile;

    bool hasUnsavedChanges = false;
    void markUnsavedChanges() { hasUnsavedChanges = true; }
    void clearUnsavedChanges() { hasUnsavedChanges = false; }

    void savePatchWithCallback (std::function<void(bool)> onComplete);
    void savePatchAsWithCallback (std::function<void(bool)> onComplete);

    void savePatchAs();
    void savePatch();
    void loadPatch();
    void exportAudioWav();
    void exportAudioWavToFile (const juce::File& file, float durationSec = 10.0f);
    void exportCanvasPngToFile (const juce::File& file);
    void exportCppScript();

    void rebuildInspector();

    void drawNode (juce::Graphics& g, const std::shared_ptr<RelativisticNode>& node);
    void drawCable (juce::Graphics& g, juce::Point<float> p1, juce::Point<float> p2, NodePortType type, bool isFeedbackLoop = false);
    void drawDelaylineDots (juce::Graphics& g, const RelativisticNode& node, float x, float y, float w, float h) const;
    void drawControlPipeDots (juce::Graphics& g, const RelativisticNode& node, float x, float y, float w, float h) const;

    std::map<int, juce::Point<float>> initialNodePositions;

    struct AlignmentGuide
    {
        float pos = 0.0f;
        bool isVertical = true;
        juce::Colour color = juce::Colour (0xff06b6d4);
    };
    std::vector<AlignmentGuide> activeGuides;

    bool showMinimap = true;
    bool isDraggingMinimap = false;

    // Quick Canvas Navigation HUD Toolbar Buttons
    juce::TextButton btnNavZoomOut   { "-" };
    juce::TextButton btnNavResetZoom { "100%" };
    juce::TextButton btnNavZoomIn    { "+" };
    juce::TextButton btnNavFitView   { "FIT ALL" };
    juce::TextButton btnNavTidy      { "TIDY" };

    bool isDebugMode = false;
    juce::TextButton btnToggleDebugMode { "DEBUG: OFF" };

    bool isConsoleVisible = false;
    juce::TextButton btnToggleConsole { "CONSOLE: OFF" };
    juce::TextEditor consoleEditor;
    juce::TextButton btnClearConsole { "CLEAR" };

    void drawDebugOverlay (juce::Graphics& g) const;
    void drawNodeDebugOverlay (juce::Graphics& g, const RelativisticNode& node) const;
    void updateConsoleDrawer();

    void panCanvas (float dx, float dy);
    void fitAllNodesInView();

    void alignSelectedLeft();
    void alignSelectedRight();
    void alignSelectedTop();
    void alignSelectedBottom();
    void distributeSelectedHorizontally();
    void distributeSelectedVertically();
    void autoTidyLayout();

    void showNodeContextMenu (const std::shared_ptr<RelativisticNode>& targetNode, juce::Point<float> mousePos);

    void drawMinimap (juce::Graphics& g) const;
    juce::Rectangle<float> getMinimapBounds() const;

    juce::Point<float> getInletPos (const RelativisticNode& node, int idx) const;
    juce::Point<float> getOutletPos (const RelativisticNode& node, int idx) const;

    void showNotificationBanner (const std::string& text, bool isWarning = false);
    void drawNotificationBanner (juce::Graphics& g) const;

    std::string notificationText;
    bool isNotificationWarning = false;
    uint32_t notificationExpiryTimeMs = 0;

class FormattedHelpContentComponent : public juce::Component
{
public:
    FormattedHelpContentComponent() = default;

    void updateContent (const juce::String& text, float targetWidth)
    {
        rawText = text;
        juce::AttributedString attrStr;
        attrStr.setWordWrap (juce::AttributedString::byWord);

        juce::Font fontHeader    = FontManager::getInstance().getOxaniumFont (15.5f, true);
        juce::Font fontSubHeader = FontManager::getInstance().getOxaniumFont (13.5f, true);
        juce::Font fontBold      = FontManager::getInstance().getOxaniumFont (12.5f, true);
        juce::Font fontBody      = FontManager::getInstance().getOxaniumFont (12.5f, false);
        juce::Font fontCode      = FontManager::getInstance().getOxaniumFont (12.0f, true);

        juce::Colour colorGold   (0xfff59e0b);
        juce::Colour colorCyan   (0xff38bdf8);
        juce::Colour colorViolet (0xffc084fc);
        juce::Colour colorGreen  (0xff34d399);
        juce::Colour colorWhite  (0xfff8fafc);

        juce::StringArray lines;
        lines.addLines (text);

        for (int i = 0; i < lines.size(); ++i)
        {
            juce::String line = lines[i];

            if (line.isEmpty())
            {
                attrStr.append ("\n\n", fontBody, juce::Colours::transparentBlack);
                continue;
            }

            if (i > 0) attrStr.append ("\n", fontBody, colorWhite);

            bool isHeader = line.startsWith ("# ") || line.contains ("MANUAL") || line.contains ("GUIDE") || line.contains ("SYNTAX") || line.endsWith ("SUITE") || line.endsWith ("ENGINE");
            
            if (isHeader)
            {
                juce::String titleLine = line.startsWith ("# ") ? line.substring (2) : line;
                attrStr.append (titleLine, fontHeader, colorGold);
            }
            else if (line.contains (":") && !line.startsWith ("   -") && !line.startsWith ("- "))
            {
                int colonIdx = line.indexOf (":");
                juce::String labelPart = line.substring (0, colonIdx + 1);
                juce::String restPart  = line.substring (colonIdx + 1);

                if (labelPart.contains ("Purple Cable") || labelPart.contains ("Time Dilation"))
                    attrStr.append (labelPart, fontBold, colorViolet);
                else if (labelPart.contains ("Cyan Cable") || labelPart.contains ("Audio"))
                    attrStr.append (labelPart, fontBold, colorCyan);
                else if (labelPart.contains ("Amber Cable") || labelPart.contains ("Control"))
                    attrStr.append (labelPart, fontBold, colorGold);
                else
                    attrStr.append (labelPart, fontBold, colorCyan);

                attrStr.append (restPart, fontBody, colorWhite);
            }
            else
            {
                juce::String remaining = line;
                while (remaining.isNotEmpty())
                {
                    int pCable = remaining.indexOf ("Purple Cable");
                    int cCable = remaining.indexOf ("Cyan Cable");
                    int aCable = remaining.indexOf ("Amber Cable");
                    int brkStart = remaining.indexOf ("[");

                    int firstPos = -1;
                    int tokenType = 0; // 1=Purple, 2=Cyan, 3=Amber, 4=BracketCode

                    auto checkPos = [&] (int pos, int type) {
                        if (pos >= 0 && (firstPos < 0 || pos < firstPos)) {
                            firstPos = pos;
                            tokenType = type;
                        }
                    };

                    checkPos (pCable, 1);
                    checkPos (cCable, 2);
                    checkPos (aCable, 3);
                    checkPos (brkStart, 4);

                    if (firstPos < 0)
                    {
                        attrStr.append (remaining, fontBody, colorWhite);
                        break;
                    }

                    if (firstPos > 0)
                    {
                        attrStr.append (remaining.substring (0, firstPos), fontBody, colorWhite);
                        remaining = remaining.substring (firstPos);
                    }

                    if (tokenType == 1)
                    {
                        attrStr.append ("Purple Cable", fontBold, colorViolet);
                        remaining = remaining.substring (12);
                    }
                    else if (tokenType == 2)
                    {
                        attrStr.append ("Cyan Cable", fontBold, colorCyan);
                        remaining = remaining.substring (10);
                    }
                    else if (tokenType == 3)
                    {
                        attrStr.append ("Amber Cable", fontBold, colorGold);
                        remaining = remaining.substring (11);
                    }
                    else if (tokenType == 4)
                    {
                        int brkEnd = remaining.indexOf ("]");
                        if (brkEnd > 0)
                        {
                            juce::String codeToken = remaining.substring (0, brkEnd + 1);
                            attrStr.append (codeToken, fontCode, colorCyan);
                            remaining = remaining.substring (brkEnd + 1);
                        }
                        else
                        {
                            attrStr.append (remaining, fontBody, colorWhite);
                            break;
                        }
                    }
                }
            }
        }

        textLayout.createLayout (attrStr, targetWidth);
        setSize (static_cast<int>(targetWidth), static_cast<int>(textLayout.getHeight() + 30.0f));
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        textLayout.draw (g, juce::Rectangle<float> (0.0f, 0.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight())));
    }

    juce::String getRawText() const { return rawText; }

private:
    juce::String rawText;
    juce::TextLayout textLayout;
};

class HelpModalOverlayComponent : public juce::Component
{
public:
    HelpModalOverlayComponent()
    {
        setInterceptsMouseClicks (true, true);

        addAndMakeVisible (titleLabel);
        titleLabel.setFont (FontManager::getInstance().getOxaniumFont (15.5f, true));
        titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xff38bdf8));

        addAndMakeVisible (contentViewport);
        contentViewport.setScrollBarsShown (true, false);
        contentViewport.setViewedComponent (&helpContent, false);

        addAndMakeVisible (btnClose);
        btnClose.setButtonText ("CLOSE");
        btnClose.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff374151));
        btnClose.setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff8fafc));
        btnClose.onClick = [this] { setVisible (false); };

        addAndMakeVisible (btnCopy);
        btnCopy.setButtonText ("COPY TO CLIPBOARD");
        btnCopy.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
        btnCopy.setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff59e0b));
        btnCopy.onClick = [this] {
            juce::SystemClipboard::copyTextToClipboard (helpContent.getRawText());
            btnCopy.setButtonText ("COPIED TO CLIPBOARD!");
        };
    }

    void showDialog (const juce::String& topic, const juce::String& content)
    {
        titleLabel.setText ("SYSTEM HELP: " + topic, juce::dontSendNotification);
        
        float viewW = cardBounds.isEmpty() ? 680.0f : (cardBounds.getWidth() - 32.0f);
        helpContent.updateContent (content, viewW - 20.0f);
        contentViewport.setViewedComponent (&helpContent, false);

        btnCopy.setButtonText ("COPY TO CLIPBOARD");
        setVisible (true);
        toFront (true);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        float cardW = std::min (bounds.getWidth() * 0.88f, 740.0f);
        float cardH = std::min (bounds.getHeight() * 0.85f, 580.0f);
        float cardX = (bounds.getWidth() - cardW) * 0.5f;
        float cardY = (bounds.getHeight() - cardH) * 0.5f;

        cardBounds = { cardX, cardY, cardW, cardH };

        titleLabel.setBounds (static_cast<int>(cardX + 16), static_cast<int>(cardY + 12), static_cast<int>(cardW - 32), 28);
        contentViewport.setBounds (static_cast<int>(cardX + 16), static_cast<int>(cardY + 48), static_cast<int>(cardW - 32), static_cast<int>(cardH - 100));

        if (helpContent.getWidth() > 0)
        {
            helpContent.updateContent (helpContent.getRawText(), cardW - 52.0f);
        }

        btnCopy.setBounds (static_cast<int>(cardX + 16), static_cast<int>(cardY + cardH - 42), 170, 30);
        btnClose.setBounds (static_cast<int>(cardX + cardW - 116), static_cast<int>(cardY + cardH - 42), 100, 30);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (!cardBounds.contains (e.position))
        {
            setVisible (false);
        }
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xd9070a12));

        juce::ColourGradient grad (juce::Colour (0xff0b0f19), cardBounds.getX(), cardBounds.getY(),
                                   juce::Colour (0xff111827), cardBounds.getRight(), cardBounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (cardBounds, 8.0f);

        g.setColour (juce::Colour (0xff06b6d4));
        g.drawRoundedRectangle (cardBounds, 8.0f, 1.5f);

        g.setColour (juce::Colour (0xff1e293b));
        g.drawHorizontalLine (static_cast<int>(cardBounds.getY() + 42.0f), cardBounds.getX() + 10.0f, cardBounds.getRight() - 10.0f);
    }

private:
    juce::Rectangle<float> cardBounds;
    juce::Label titleLabel;
    juce::Viewport contentViewport;
    FormattedHelpContentComponent helpContent;
    juce::TextButton btnClose;
    juce::TextButton btnCopy;
};

    HelpModalOverlayComponent helpModalOverlay;
    std::unique_ptr<RelativisticOscServer> oscServer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativisticCanvasComponent)
};

} // namespace time_dilation
