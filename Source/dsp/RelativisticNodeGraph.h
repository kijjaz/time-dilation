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
    bool isConnected = false; // Port connection status
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

enum class ParameterType
{
    Float,
    Integer,
    Toggle,
    Symbol
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
    bool isInteger = false;
    ParameterType type = ParameterType::Float;
    std::string stringValue = "";
};

// Fractional Hermite 4-Point Delay Line for Audio Node Time Dilation
class AudioTimeDelayLine
{
private:
    mutable juce::SpinLock lock;
    juce::AudioBuffer<float> buffer;
    int writePos = 0;
    double fs = 44100.0;

public:
    AudioTimeDelayLine() = default;

    void prepare (double sampleRate, int numChannels, double maxDelaySeconds = 10.0)
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        fs = sampleRate;
        int maxSamples = static_cast<int>(sampleRate * maxDelaySeconds) + 32;
        buffer.setSize (numChannels, maxSamples);
        buffer.clear();
        writePos = 0;
    }

    void writeSample (int ch, float sample)
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        if (ch >= buffer.getNumChannels() || buffer.getNumSamples() <= 0) return;
        buffer.setSample (ch, writePos, sample);
    }

    void advanceWritePos()
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        int maxS = buffer.getNumSamples();
        if (maxS > 0)
            writePos = (writePos + 1) % maxS;
    }

    float readHermite (int ch, double delaySamples) const
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        int numS = buffer.getNumSamples();
        if (numS <= 0 || ch >= buffer.getNumChannels()) return 0.0f;

        double rPos = static_cast<double>(writePos) - delaySamples;
        while (rPos < 0.0) rPos += numS;
        while (rPos >= numS) rPos -= numS;

        int i1 = static_cast<int>(rPos);
        double f = rPos - i1;

        int i0 = (i1 - 1 + numS) % numS;
        int i2 = (i1 + 1) % numS;
        int i3 = (i1 + 2) % numS;

        const float* d = buffer.getReadPointer (ch);
        float y0 = d[i0], y1 = d[i1], y2 = d[i2], y3 = d[i3];

        float c0 = y1;
        float c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return c0 + static_cast<float>(f) * (c1 + static_cast<float>(f) * (c2 + static_cast<float>(f) * c3));
    }

    int getBufferSamples() const
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        return buffer.getNumSamples();
    }

    int getWritePos() const
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        return writePos;
    }

    void getSampleSnapshot (std::vector<float>& destDots, int numDots, int& outWritePos, int& outTotalSamples) const
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        outTotalSamples = buffer.getNumSamples();
        outWritePos = writePos;
        destDots.assign (numDots, 0.0f);
        if (outTotalSamples <= 0 || buffer.getNumChannels() <= 0) return;
        const float* readPtr = buffer.getReadPointer (0);
        float stepS = static_cast<float>(outTotalSamples) / static_cast<float>(numDots);
        for (int i = 0; i < numDots; ++i)
        {
            int sIdx = static_cast<int>(i * stepS) % outTotalSamples;
            destDots[i] = readPtr[sIdx];
        }
    }
};

// Time-Stamped Control Message Structure
struct ControlMessage
{
    double targetTau = 0.0;
    float value = 0.0f;
    std::string textMessage;
    bool isBang = false;
    int portIndex = 0;
};

// Time-Stamped Control Message Pipe for Control Node Time Dilation
class ControlMessagePipe
{
private:
    mutable juce::SpinLock lock;
    std::vector<ControlMessage> messages;
    std::vector<ControlMessage> history;

public:
    ControlMessagePipe() = default;

    void clear()
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        messages.clear();
        history.clear();
    }

    void pushMessage (double targetTau, float val, const std::string& msg = "", bool bang = false, int port = 0)
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        ControlMessage m;
        m.targetTau = targetTau;
        m.value = val;
        m.textMessage = msg;
        m.isBang = bang;
        m.portIndex = port;
        messages.push_back (m);
    }

    std::vector<ControlMessage> popPendingMessages (double currentTau)
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        std::vector<ControlMessage> pending;
        auto it = messages.begin();
        while (it != messages.end())
        {
            if (it->targetTau <= currentTau)
            {
                pending.push_back (*it);
                history.push_back (*it);
                it = messages.erase (it);
            }
            else
            {
                ++it;
            }
        }
        return pending;
    }

    std::vector<ControlMessage> popRetrogradeMessages (double currentTau)
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        std::vector<ControlMessage> pending;
        auto it = history.rbegin();
        while (it != history.rend())
        {
            if (it->targetTau >= currentTau)
            {
                pending.push_back (*it);
                messages.push_back (*it);
                it = decltype(it)(history.erase ((it + 1).base()));
            }
            else
            {
                ++it;
            }
        }
        return pending;
    }

    size_t getPendingCount() const
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        return messages.size();
    }

    size_t getHistoryCount() const
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        return history.size();
    }

    std::vector<ControlMessage> getSnapshotMessages() const
    {
        const juce::SpinLock::ScopedLockType sl (lock);
        return messages;
    }
};

class RelativisticNode
{
public:
    RelativisticNode (int id, const std::string& typeName, const std::string& label);
    virtual ~RelativisticNode() = default;

    int getId() const { return nodeId; }
    std::string getTypeName() const { return nodeTypeName; }
    std::string getLabel() const { return nodeLabel; }
    virtual void setLabel (const std::string& l) { nodeLabel = l; }

    const std::vector<Port>& getInlets() const { return inlets; }
    const std::vector<Port>& getOutlets() const { return outlets; }

    std::vector<Port>& getInlets() { return inlets; }
    std::vector<Port>& getOutlets() { return outlets; }

    virtual void prepare (double sampleRate, int samplesPerBlock);
    void ensureBufferSize (int requiredSamples);
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

    bool isBypassed() const { return getParameter ("bypass", 0.0f) > 0.5f; }
    void setBypassed (bool b) { setParameter ("bypass", b ? 1.0f : 0.0f); }

    void setParamExpression (const std::string& key, const std::string& expr) { paramExpressions[key] = expr; }
    std::string getParamExpression (const std::string& key) const {
        auto it = paramExpressions.find (key);
        return (it != paramExpressions.end()) ? it->second : "";
    }

    int addModulationInlet (const std::string& paramKey);
    bool hasModulationInlet (const std::string& paramKey) const;
    int getModulationInletIndex (const std::string& paramKey) const;
    bool removeModulationInlet (const std::string& paramKey);

    virtual std::vector<ParameterInfo> getParameterDefs() const;
    virtual std::vector<std::string> getExposedMethods() const;
    virtual void invokeMethod (const std::string& methodName);

    void setFormulaScript (const std::string& script) { formulaScript = script; }
    std::string getFormulaScript() const {
        return formulaScript.empty() ? getDefaultFormulaScript() : formulaScript;
    }
    virtual std::string getDefaultFormulaScript() const;

    virtual juce::ValueTree saveToValueTree() const;
    virtual void loadFromValueTree (const juce::ValueTree& v, bool preserveExistingId = false);

    void setParentGraph (const RelativisticNodeGraph* graph) { parentGraph = graph; }
    const RelativisticNodeGraph* getParentGraph() const { return parentGraph; }

    double getEffectiveGamma() const;
    virtual double getRequestedFutureHorizonSec() const;
    double updateCoordinateTime (int numSamples);
    double getLocalCoordinateTime() const { return localCoordinateTime; }

    bool isAudioNode() const;
    bool isControlNode() const;

    bool isShowDelaylineEnabled() const { return showDelayline; }
    void setShowDelaylineEnabled (bool show) { showDelayline = show; }

    bool isShowPipeEnabled() const { return showPipe; }
    void setShowPipeEnabled (bool show) { showPipe = show; }

    AudioTimeDelayLine& getAudioDelayLine() { return audioDelayLine; }
    const AudioTimeDelayLine& getAudioDelayLine() const { return audioDelayLine; }
    ControlMessagePipe& getControlMessagePipe() { return controlMessagePipe; }
    const ControlMessagePipe& getControlMessagePipe() const { return controlMessagePipe; }

    void processAudioTimeDelay (juce::AudioBuffer<float>& buffer, double effectiveGamma);
    void processControlTimePipe (double currentTau, double effectiveGamma);

    virtual void parseLabelArguments (const std::string& label);

protected:
    int nodeId = 0;
    std::string nodeTypeName;
    std::string nodeLabel;
    float posX = 100.0f;
    float posY = 100.0f;
    double localCoordinateTime = 0.0;
    mutable double smoothedGamma = 1.0;
    double readDelayOffset = 0.0;

    bool showDelayline = false;
    bool showPipe = false;

    AudioTimeDelayLine audioDelayLine;
    ControlMessagePipe controlMessagePipe;

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

    static bool isValidObjectType (const std::string& typeName);

    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& masterOutput, int numSamples);

    void setAudioEngineEnabled (bool enabled) { audioEngineEnabled = enabled; }
    bool isAudioEngineEnabled() const { return audioEngineEnabled; }

    double getCurrentCausalityHorizonSec() const { return currentCausalityHorizonSec; }
    void resetCausalityHorizon()
    {
        const juce::ScopedLock lock (processLock);
        currentCausalityHorizonSec = 0.0;
        targetCausalityHorizonSec = 0.0;
        globalDelayWritePos = 0;
        globalDelayRingBuffer.clear();
    }

    int addNode (const std::string& typeName, float x, float y);
    void removeNode (int nodeId);
    bool removeModulationInlet (int nodeId, const std::string& paramKey);

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
    void loadTableExamplePatch();
    void loadFutureBassDrumExamplePatch();
    void loadRhythmicTimeWarpingExamplePatch();
    void loadSoundPitchWarpingExamplePatch();
    void loadRelativisticTimeModulationExamplePatch();

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
    std::vector<std::shared_ptr<RelativisticNode>> getTopologicallySortedNodes() const;
    void pushNodeOutletsToConnectedInlets (RelativisticNode* srcNode);

    juce::ValueTree saveToValueTree() const;
    void loadFromValueTree (const juce::ValueTree& v, bool isRestoringUndo = false);

private:
    mutable juce::CriticalSection processLock;

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

    // Global Causality Horizon Engine State
    double currentCausalityHorizonSec = 0.0;
    double targetCausalityHorizonSec = 0.0;
    double causalitySmoothingHz = 10.0;
    juce::AudioBuffer<float> globalDelayRingBuffer;
    int globalDelayWritePos = 0;

    void propagateTimeDilationHierarchy();
    void propagateSignals();
};

} // namespace time_dilation
