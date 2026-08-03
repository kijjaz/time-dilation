#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/RelativisticNodeGraph.h"

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

    void panCanvas (float dx, float dy);
    void fitAllNodesInView();

    juce::Point<float> getInletPos (const RelativisticNode& node, int idx) const;
    juce::Point<float> getOutletPos (const RelativisticNode& node, int idx) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativisticCanvasComponent)
};

} // namespace time_dilation
