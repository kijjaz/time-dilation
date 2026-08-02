#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_data_structures/juce_data_structures.h>
#include <vector>
#include <memory>
#include <string>
#include <map>

namespace time_dilation
{

class TableNode;

enum class NodePortType
{
    Control, // Bang / Float values (Amber)
    Audio,   // Audio Buffer ~ (Cyan)
    Time     // Time Context / Gamma Dilated Clock (Purple)
};

struct Port
{
    int portId = 0;
    std::string name;
    NodePortType type = NodePortType::Audio;
    float controlValue = 0.0f;
    juce::AudioBuffer<float> audioData;
    juce::AudioBuffer<float> previousBlockBuffer; // 1-Block History Delay Buffer for Feedback Routing Stability!
    double timeGamma = 1.0; // Relativistic Time Dilation Factor
};

class RelativisticNode;
class RelativisticNodeGraph;

struct PatchConnection
{
    int id = 0;
    int sourceNodeId = 0;
    int sourceOutletIdx = 0;
    int destNodeId = 0;
    int destInletIdx = 0;
    bool isFeedbackLoop = false;
    float singleSampleMemoryL = 0.0f;
    float singleSampleMemoryR = 0.0f;
};

struct ParameterInfo
{
    std::string key;
    std::string name;
    float value = 0.0f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    std::string expression;
    int modInletIdx = -1; // -1 if not patched as modulation inlet
};

class RelativisticNode
{
public:
    RelativisticNode (int id, const std::string& typeName, const std::string& label);
    virtual ~RelativisticNode() = default;

    int getId() const { return nodeId; }
    std::string getTypeName() const { return nodeTypeName; }
    std::string getLabel() const { return nodeLabel; }
    void setLabel (const std::string& l) { nodeLabel = l; }

    const std::vector<Port>& getInlets() const { return inlets; }
    const std::vector<Port>& getOutlets() const { return outlets; }

    std::vector<Port>& getInlets() { return inlets; }
    std::vector<Port>& getOutlets() { return outlets; }

    virtual void prepare (double sampleRate, int samplesPerBlock);
    virtual void process (int numSamples) = 0;

    void setPosition (float x, float y) { posX = x; posY = y; }
    float getX() const { return posX; }
    float getY() const { return posY; }

    // Parameter & Formula Scripting
    void setParameter (const std::string& key, float val) { parameters[key] = val; }
    float getParameter (const std::string& key, float defaultVal = 0.0f) const {
        auto it = parameters.find (key);
        return (it != parameters.end()) ? it->second : defaultVal;
    }
    float getModulatedParamValue (const std::string& paramKey, float defaultVal = 0.0f, int sampleIdx = 0) const;
    const std::map<std::string, float>& getParameters() const { return parameters; }

    void setParamExpression (const std::string& key, const std::string& expr) { paramExpressions[key] = expr; }
    std::string getParamExpression (const std::string& key) const {
        auto it = paramExpressions.find (key);
        return (it != paramExpressions.end()) ? it->second : "";
    }

    int addModulationInlet (const std::string& paramKey);

    virtual std::vector<ParameterInfo> getParameterDefs() const;
    virtual std::vector<std::string> getExposedMethods() const;
    virtual void invokeMethod (const std::string& methodName);

    void setFormulaScript (const std::string& script) { formulaScript = script; }
    std::string getFormulaScript() const {
        return formulaScript.empty() ? getDefaultFormulaScript() : formulaScript;
    }
    virtual std::string getDefaultFormulaScript() const;

    juce::ValueTree saveToValueTree() const;
    void loadFromValueTree (const juce::ValueTree& v);

    void setParentGraph (const RelativisticNodeGraph* graph) { parentGraph = graph; }
    const RelativisticNodeGraph* getParentGraph() const { return parentGraph; }

    double getEffectiveGamma() const;
    double updateCoordinateTime (int numSamples);
    double getLocalCoordinateTime() const { return localCoordinateTime; }

protected:
    int nodeId = 0;
    std::string nodeTypeName;
    std::string nodeLabel;
    float posX = 100.0f;
    float posY = 100.0f;
    double localCoordinateTime = 0.0;

    const RelativisticNodeGraph* parentGraph = nullptr;

    std::map<std::string, float> parameters;
    std::map<std::string, std::string> paramExpressions;
    std::map<std::string, int> paramModInlets;
    std::string formulaScript;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    std::vector<Port> inlets;
    std::vector<Port> outlets;

    void addInlet (const std::string& name, NodePortType type);
    void addOutlet (const std::string& name, NodePortType type);
};

class RelativisticNodeGraph
{
public:
    RelativisticNodeGraph();
    ~RelativisticNodeGraph() = default;

    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& masterOutput, int numSamples);

    void setAudioEngineEnabled (bool enabled) { audioEngineEnabled = enabled; }
    bool isAudioEngineEnabled() const { return audioEngineEnabled; }

    int addNode (const std::string& typeName, float x, float y);
    void removeNode (int nodeId);

    int addConnection (int srcNodeId, int srcOutletIdx, int destNodeId, int destInletIdx);
    void removeConnection (int connectionId);

    const std::vector<std::shared_ptr<RelativisticNode>>& getNodes() const { return nodes; }
    const std::vector<PatchConnection>& getConnections() const { return connections; }

    std::shared_ptr<RelativisticNode> getNodeById (int id);
    std::shared_ptr<TableNode> getTableByName (const std::string& name) const;

    void clearGraph();
    void createDefaultPatch();

    // Interactive Relativistic Time Example Patches
    void loadTimeWarpExamplePatch();
    void loadTimeRetroExamplePatch();
    void loadTimeStasisExamplePatch();
    void loadTimeSingularityExamplePatch();
    void loadTimeQuantizeExamplePatch();
    void loadTimeTransportExamplePatch();

    // Undo / Redo Persistent Stack
    void pushUndoState();
    bool undo();
    bool redo();
    bool canUndo() const { return undoIndex > 0; }
    bool canRedo() const { return undoIndex + 1 < static_cast<int>(undoStack.size()); }

    // Clipboard & Duplicate Operations
    juce::ValueTree copyNodes (const std::vector<int>& nodeIds);
    std::vector<int> pasteNodes (const juce::ValueTree& clipboardData, float offsetX = 40.0f, float offsetY = 40.0f);
    std::vector<int> duplicateNodes (const std::vector<int>& nodeIds);
    void cutNodes (const std::vector<int>& nodeIds);

    bool saveProjectToFile (const juce::File& file);
    bool loadProjectFromFile (const juce::File& file);

    void setGlobalVariable (const std::string& key, double val) { globalVariables[key] = val; }
    double getGlobalVariable (const std::string& key, double defaultVal = 0.0) const {
        auto it = globalVariables.find (key);
        return (it != globalVariables.end()) ? it->second : defaultVal;
    }
    const std::map<std::string, double>& getGlobalVariables() const { return globalVariables; }

    double tapSignal (const std::string& target) const;
    void detectFeedbackLoops();

    juce::ValueTree saveToValueTree() const;
    void loadFromValueTree (const juce::ValueTree& v, bool isRestoringUndo = false);

private:
    double sampleRate = 44100.0;
    int blockSize = 512;
    bool audioEngineEnabled = false;

    int nextNodeId = 1;
    int nextConnectionId = 1;

    std::vector<std::shared_ptr<RelativisticNode>> nodes;
    std::vector<PatchConnection> connections;

    std::map<std::string, double> globalVariables;

    // Persistent Undo / Redo Stack
    std::vector<juce::ValueTree> undoStack;
    int undoIndex = -1;

    void propagateTimeDilationHierarchy();
    void propagateSignals();
};

} // namespace time_dilation
