#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/RelativisticNodeGraph.h"
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

    // Top Header Menu Bar Buttons
    juce::TextButton btnMenuFile    { "File" };
    juce::TextButton btnMenuEdit    { "Edit" };
    juce::TextButton btnMenuView    { "View" };
    juce::TextButton btnMenuObjects { "Objects" };
    juce::TextButton btnMenuAudio   { "Audio" };
    juce::TextButton btnMenuHelp    { "Help" };

    void showMenuFile();
    void showMenuEdit();
    void showMenuView();
    void showMenuObjects();
    void showMenuAudio();
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
    };
    std::vector<AutocompleteItem> allRegisteredObjects;
    std::vector<AutocompleteItem> filteredAutocompleteItems;
    int selectedAutocompleteIdx = 0;

    bool isEditingDraftObject = false;
    bool isDraftObjectInvalid = false;
    std::string draftWarningText;
    juce::Point<float> draftObjectCanvasPos;
    std::unique_ptr<juce::TextEditor> draftObjectEditor;

    void initObjectCatalog();
    void spawnInlineObjectEditor (juce::Point<float> spawnCanvasPos);
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

    juce::TextButton btnInsertTapDropdown { "TAP SIGNAL POINT" };

    juce::TextEditor inlineLabelEditor;
    juce::TextEditor formulaEditor;
    juce::TextButton btnApplyFormula { "APPLY C++ / MATH FORMULA" };

    juce::Label exprFormulaLabel { "exprLabel", "MATH EXPRESSION FORMULA:" };
    juce::TextEditor exprFormulaEditor;
    juce::TextButton btnApplyExprFormula { "APPLY EXPRESSION FORMULA" };

    juce::File currentProjectFile;

    void savePatchAs();
    void savePatch();
    void loadPatch();
    void exportAudioWav();
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

    void drawDebugOverlay (juce::Graphics& g) const;
    void drawNodeDebugOverlay (juce::Graphics& g, const RelativisticNode& node) const;

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

class HelpModalOverlayComponent : public juce::Component
{
public:
    HelpModalOverlayComponent()
    {
        setInterceptsMouseClicks (true, true);

        addAndMakeVisible (titleLabel);
        titleLabel.setFont (FontManager::getInstance().getOxaniumFont (15.5f, true));
        titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xff38bdf8));

        addAndMakeVisible (contentEditor);
        contentEditor.setMultiLine (true);
        contentEditor.setReadOnly (true);
        contentEditor.setScrollbarsShown (true);
        contentEditor.setCaretVisible (false);
        contentEditor.setPopupMenuEnabled (true);
        contentEditor.setFont (FontManager::getInstance().getOxaniumFont (13.0f, false));
        contentEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff050811));
        contentEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff1e293b));
        contentEditor.setColour (juce::TextEditor::textColourId, juce::Colour (0xfff8fafc));

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
            juce::SystemClipboard::copyTextToClipboard (contentEditor.getText());
            btnCopy.setButtonText ("COPIED TO CLIPBOARD!");
        };
    }

    void showDialog (const juce::String& topic, const juce::String& content)
    {
        titleLabel.setText ("SYSTEM HELP: " + topic, juce::dontSendNotification);
        contentEditor.setText (content, false);
        contentEditor.moveCaretToTop (false);
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
        contentEditor.setBounds (static_cast<int>(cardX + 16), static_cast<int>(cardY + 48), static_cast<int>(cardW - 32), static_cast<int>(cardH - 100));

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
    juce::TextEditor contentEditor;
    juce::TextButton btnClose;
    juce::TextButton btnCopy;
};

    HelpModalOverlayComponent helpModalOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativisticCanvasComponent)
};

} // namespace time_dilation
