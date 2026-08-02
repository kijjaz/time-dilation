#include "RelativisticNodeGraph.h"
#include "RelativisticNodeObjects.h"
#include <algorithm>

namespace time_dilation
{

RelativisticNode::RelativisticNode (int id, const std::string& typeName, const std::string& label)
    : nodeId (id), nodeTypeName (typeName), nodeLabel (label)
{
}

double RelativisticNode::getEffectiveGamma() const
{
    double g = 1.0;
    if (!inlets.empty() && inlets[0].type == NodePortType::Time && inlets[0].timeGamma != 0.0)
    {
        g = inlets[0].timeGamma;
    }
    else
    {
        auto it = parameters.find ("gamma");
        if (it != parameters.end()) g = static_cast<double>(it->second);
    }
    return std::clamp (g, -16.0, 16.0);
}

double RelativisticNode::updateCoordinateTime (int numSamples)
{
    double effGamma = getEffectiveGamma();
    localCoordinateTime += effGamma * (static_cast<double>(numSamples) / currentSampleRate);
    return localCoordinateTime;
}

void RelativisticNode::addInlet (const std::string& name, NodePortType type)
{
    Port p;
    p.portId = static_cast<int>(inlets.size());
    p.name = name;
    p.type = type;
    p.audioData.setSize (2, 512);
    inlets.push_back (p);
}

void RelativisticNode::addOutlet (const std::string& name, NodePortType type)
{
    Port p;
    p.portId = static_cast<int>(outlets.size());
    p.name = name;
    p.type = type;
    p.audioData.setSize (2, 512);
    outlets.push_back (p);
}

int RelativisticNode::addModulationInlet (const std::string& paramKey)
{
    auto it = paramModInlets.find (paramKey);
    if (it != paramModInlets.end()) return it->second;

    int inletIdx = static_cast<int>(inlets.size());
    addInlet (paramKey + "_mod", NodePortType::Control);
    paramModInlets[paramKey] = inletIdx;
    return inletIdx;
}

float RelativisticNode::getModulatedParamValue (const std::string& paramKey, float defaultVal, int sampleIdx) const
{
    float baseVal = getParameter (paramKey, defaultVal);
    auto it = paramModInlets.find (paramKey);
    if (it != paramModInlets.end())
    {
        int inletIdx = it->second;
        if (inletIdx >= 0 && inletIdx < static_cast<int>(inlets.size()))
        {
            const auto& port = inlets[static_cast<size_t>(inletIdx)];
            if (port.audioData.getNumSamples() > 0 && port.audioData.getMagnitude (0, port.audioData.getNumSamples()) > 0.0001f)
            {
                int samplePos = std::clamp (sampleIdx, 0, port.audioData.getNumSamples() - 1);
                return port.audioData.getSample (0, samplePos);
            }
            else if (port.controlValue != 0.0f)
            {
                return port.controlValue;
            }
        }
    }
    return baseVal;
}

std::vector<ParameterInfo> RelativisticNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    for (const auto& kv : parameters)
    {
        ParameterInfo p;
        p.key = kv.first;
        p.name = kv.first;
        p.value = kv.second;
        p.minValue = 0.0f;
        p.maxValue = 20000.0f;

        auto exprIt = paramExpressions.find (kv.first);
        if (exprIt != paramExpressions.end()) p.expression = exprIt->second;

        auto modIt = paramModInlets.find (kv.first);
        if (modIt != paramModInlets.end()) p.modInletIdx = modIt->second;

        defs.push_back (p);
    }
    return defs;
}

std::vector<std::string> RelativisticNode::getExposedMethods() const
{
    return { "resetState()" };
}

void RelativisticNode::invokeMethod (const std::string& /*methodName*/)
{
}

std::string RelativisticNode::getDefaultFormulaScript() const
{
    return "// C++ / DSP Math Expression Script for [" + nodeTypeName + "]\n// Dynamic signals: $v1, $v2, $t, $gt, $gamma\n\nval = $v1;";
}

void RelativisticNode::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    for (auto& p : inlets)
    {
        p.audioData.setSize (2, samplesPerBlock);
        p.previousBlockBuffer.setSize (2, samplesPerBlock);
    }
    for (auto& p : outlets)
    {
        p.audioData.setSize (2, samplesPerBlock);
        p.previousBlockBuffer.setSize (2, samplesPerBlock);
    }
}

juce::ValueTree RelativisticNode::saveToValueTree() const
{
    juce::ValueTree v ("Node");
    v.setProperty ("id", nodeId, nullptr);
    v.setProperty ("type", juce::String (nodeTypeName), nullptr);
    v.setProperty ("label", juce::String (nodeLabel), nullptr);
    v.setProperty ("posX", posX, nullptr);
    v.setProperty ("posY", posY, nullptr);
    v.setProperty ("formulaScript", juce::String (formulaScript), nullptr);

    juce::ValueTree paramsTree ("Parameters");
    for (const auto& kv : parameters)
    {
        juce::ValueTree p ("Param");
        p.setProperty ("key", juce::String (kv.first), nullptr);
        p.setProperty ("val", kv.second, nullptr);
        paramsTree.addChild (p, -1, nullptr);
    }
    v.addChild (paramsTree, -1, nullptr);
    return v;
}

void RelativisticNode::loadFromValueTree (const juce::ValueTree& v)
{
    nodeId = v.getProperty ("id", nodeId);
    nodeTypeName = v.getProperty ("type", juce::String (nodeTypeName)).toString().toStdString();
    nodeLabel = v.getProperty ("label", juce::String (nodeLabel)).toString().toStdString();
    posX = v.getProperty ("posX", posX);
    posY = v.getProperty ("posY", posY);
    formulaScript = v.getProperty ("formulaScript", juce::String (formulaScript)).toString().toStdString();

    auto paramsTree = v.getChildWithName ("Parameters");
    if (paramsTree.isValid())
    {
        parameters.clear();
        for (int i = 0; i < paramsTree.getNumChildren(); ++i)
        {
            auto p = paramsTree.getChild (i);
            parameters[p.getProperty ("key").toString().toStdString()] = p.getProperty ("val");
        }
    }
}

// ----------------------------------------------------
// RelativisticNodeGraph Engine Implementation
// ----------------------------------------------------

RelativisticNodeGraph::RelativisticNodeGraph()
{
    pushUndoState();
}

void RelativisticNodeGraph::prepare (double sr, int samplesPerBlock)
{
    sampleRate = sr;
    blockSize = samplesPerBlock;

    for (auto& n : nodes)
    {
        n->prepare (sampleRate, blockSize);
    }
}

int RelativisticNodeGraph::addNode (const std::string& typeName, float x, float y)
{
    int id = nextNodeId++;
    std::shared_ptr<RelativisticNode> node = nullptr;

    if (typeName == "time.warp~")        node = std::make_shared<TimeWarpNode> (id);
    else if (typeName == "time.retro~")   node = std::make_shared<TimeRetroNode> (id);
    else if (typeName == "time.quantize~")node = std::make_shared<TimeQuantizeNode> (id);
    else if (typeName == "time.metro~")   node = std::make_shared<TimeMetroNode> (id);
    else if (typeName == "time.stasis~")  node = std::make_shared<TimeStasisNode> (id);
    else if (typeName == "time.singularity~") node = std::make_shared<TimeSingularityNode> (id);
    else if (typeName == "time.transport")node = std::make_shared<TimeTransportNode> (id);
    else if (typeName == "osc~")           node = std::make_shared<OscNode> (id);
    else if (typeName == "phasor~")        node = std::make_shared<PhasorNode> (id);
    else if (typeName == "sampler~")       node = std::make_shared<SamplerNode> (id);
    else if (typeName == "filter~")        node = std::make_shared<FilterNode> (id);
    else if (typeName == "delay~")         node = std::make_shared<DelayNode> (id);
    else if (typeName == "dac~")           node = std::make_shared<DacNode> (id);
    else if (typeName == "expr")           node = std::make_shared<ExprNode> (id);
    else if (typeName == "expr~")          node = std::make_shared<ExprAudioNode> (id);
    else if (typeName == "fexpr~")         node = std::make_shared<FexprAudioNode> (id);
    else if (typeName == "gain~")          node = std::make_shared<GainNode> (id);
    else if (typeName == "out~")           node = std::make_shared<OutNode> (id);
    else if (typeName == "env~")           node = std::make_shared<EnvFollowerNode> (id);
    else if (typeName == "tap")            node = std::make_shared<TapControlNode> (id);
    else if (typeName == "tap~")           node = std::make_shared<TapAudioNode> (id);
    else if (typeName == "v")              node = std::make_shared<ValueNode> (id);
    else if (typeName == "z~")             node = std::make_shared<OneSampleDelayNode> (id);
    else if (typeName == "snapshot~")      node = std::make_shared<SnapshotNode> (id);
    else if (typeName == "+")              node = std::make_shared<AddMathNode> (id);
    else if (typeName == "*")              node = std::make_shared<MulMathNode> (id);
    else if (typeName == "table")          node = std::make_shared<TableNode> (id);
    else if (typeName == "tabwrite~")      node = std::make_shared<TabWriteNode> (id);
    else if (typeName == "tabread~")       node = std::make_shared<TabReadNode> (id);
    else if (typeName == "tabosc4~")       node = std::make_shared<TabOscNode> (id);
    else if (typeName == "svfilter~")      node = std::make_shared<SvFilterNode> (id);
    else if (typeName == "drive~")         node = std::make_shared<DriveNode> (id);
    else if (typeName == "reverb~")        node = std::make_shared<ReverbNode> (id);
    else if (typeName == "crush~")         node = std::make_shared<CrushNode> (id);
    else if (typeName == "adsr~")          node = std::make_shared<AdsrNode> (id);
    else if (typeName == "mtof")           node = std::make_shared<MtofNode> (id);
    else if (typeName == "ftom")           node = std::make_shared<FtomNode> (id);
    else if (typeName == "note")           node = std::make_shared<NoteGenNode> (id);
    else                                   node = std::make_shared<OscNode> (id);

    node->setParentGraph (this);
    node->setPosition (x, y);
    node->prepare (sampleRate, blockSize);
    nodes.push_back (node);
    return id;
}

std::shared_ptr<TableNode> RelativisticNodeGraph::getTableByName (const std::string& name) const
{
    for (const auto& n : nodes)
    {
        if (n->getTypeName() == "table")
        {
            auto tbl = std::dynamic_pointer_cast<TableNode> (n);
            if (tbl && tbl->getTableName() == name)
            {
                return tbl;
            }
        }
    }
    return nullptr;
}

void RelativisticNodeGraph::removeNode (int nodeId)
{
    nodes.erase (std::remove_if (nodes.begin(), nodes.end(),
        [nodeId] (const std::shared_ptr<RelativisticNode>& n) { return n->getId() == nodeId; }), nodes.end());

    connections.erase (std::remove_if (connections.begin(), connections.end(),
        [nodeId] (const PatchConnection& c) {
            return c.sourceNodeId == nodeId || c.destNodeId == nodeId;
        }), connections.end());
}

int RelativisticNodeGraph::addConnection (int srcNodeId, int srcOutletIdx, int destNodeId, int destInletIdx)
{
    PatchConnection c;
    c.id = nextConnectionId++;
    c.sourceNodeId = srcNodeId;
    c.sourceOutletIdx = srcOutletIdx;
    c.destNodeId = destNodeId;
    c.destInletIdx = destInletIdx;

    connections.push_back (c);
    detectFeedbackLoops();
    return c.id;
}

void RelativisticNodeGraph::removeConnection (int connectionId)
{
    connections.erase (std::remove_if (connections.begin(), connections.end(),
        [connectionId] (const PatchConnection& c) { return c.id == connectionId; }), connections.end());
    detectFeedbackLoops();
}

void RelativisticNodeGraph::detectFeedbackLoops()
{
    for (auto& conn : connections)
    {
        conn.isFeedbackLoop = false;
    }

    for (auto& conn : connections)
    {
        std::set<int> visited;
        std::vector<int> stack;
        stack.push_back (conn.destNodeId);
        visited.insert (conn.destNodeId);

        bool cycleFound = false;
        while (!stack.empty())
        {
            int curr = stack.back();
            stack.pop_back();

            if (curr == conn.sourceNodeId)
            {
                cycleFound = true;
                break;
            }

            for (const auto& nextConn : connections)
            {
                if (nextConn.sourceNodeId == curr)
                {
                    if (visited.find (nextConn.destNodeId) == visited.end())
                    {
                        visited.insert (nextConn.destNodeId);
                        stack.push_back (nextConn.destNodeId);
                    }
                }
            }
        }

        if (cycleFound)
        {
            conn.isFeedbackLoop = true;
        }
    }
}

std::shared_ptr<RelativisticNode> RelativisticNodeGraph::getNodeById (int id)
{
    for (auto& n : nodes)
    {
        if (n->getId() == id) return n;
    }
    return nullptr;
}

void RelativisticNodeGraph::clearGraph()
{
    nodes.clear();
    connections.clear();
    nextNodeId = 1;
    nextConnectionId = 1;
}

void RelativisticNodeGraph::createDefaultPatch()
{
    loadTimeWarpExamplePatch();
}

void RelativisticNodeGraph::loadTimeWarpExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nWarp = addNode ("time.warp~", 80.0f, 80.0f);
    int nOsc  = addNode ("osc~",       280.0f, 80.0f);
    int nFilt = addNode ("filter~",    280.0f, 240.0f);
    int nOut  = addNode ("out~",       280.0f, 400.0f);
    int nDac  = addNode ("dac~",       480.0f, 400.0f);

    addConnection (nWarp, 0, nOsc, 0);  // time.warp~ -> osc~
    addConnection (nOsc, 0, nFilt, 0);  // osc~ -> filter~
    addConnection (nFilt, 0, nOut, 0);  // filter~ -> out~
    addConnection (nFilt, 0, nDac, 0);  // filter~ -> dac~ L
    addConnection (nFilt, 0, nDac, 1);  // filter~ -> dac~ R

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeRetroExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nRetro  = addNode ("time.retro~", 80.0f, 80.0f);
    int nSamp   = addNode ("sampler~",    280.0f, 80.0f);
    int nDrive  = addNode ("drive~",      280.0f, 240.0f);
    int nOut    = addNode ("out~",        280.0f, 400.0f);
    int nDac    = addNode ("dac~",        480.0f, 400.0f);

    addConnection (nRetro, 0, nSamp, 0);  // time.retro~ -> sampler~
    addConnection (nSamp, 0, nDrive, 0);  // sampler~ -> drive~
    addConnection (nDrive, 0, nOut, 0);   // drive~ -> out~
    addConnection (nDrive, 0, nDac, 0);   // drive~ -> dac~ L
    addConnection (nDrive, 0, nDac, 1);   // drive~ -> dac~ R

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeStasisExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nStasis = addNode ("time.stasis~", 80.0f, 80.0f);
    int nOsc    = addNode ("osc~",         280.0f, 80.0f);
    int nReverb = addNode ("reverb~",      280.0f, 240.0f);
    int nOut    = addNode ("out~",         280.0f, 400.0f);
    int nDac    = addNode ("dac~",         480.0f, 400.0f);

    addConnection (nStasis, 0, nOsc, 0);   // time.stasis~ -> osc~
    addConnection (nOsc, 0, nReverb, 0);   // osc~ -> reverb~
    addConnection (nReverb, 0, nOut, 0);   // reverb~ -> out~
    addConnection (nReverb, 0, nDac, 0);   // reverb~ -> dac~ L
    addConnection (nReverb, 0, nDac, 1);   // reverb~ -> dac~ R

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeSingularityExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nSing = addNode ("time.singularity~", 80.0f, 80.0f);
    int nOsc  = addNode ("osc~",              280.0f, 80.0f);
    int nSv   = addNode ("svfilter~",         280.0f, 240.0f);
    int nOut  = addNode ("out~",              280.0f, 400.0f);
    int nDac  = addNode ("dac~",              480.0f, 400.0f);

    addConnection (nSing, 0, nOsc, 0); // time.singularity~ -> osc~
    addConnection (nOsc, 0, nSv, 0);   // osc~ -> svfilter~
    addConnection (nSv, 0, nOut, 0);   // svfilter~ -> out~
    addConnection (nSv, 0, nDac, 0);   // svfilter~ -> dac~ L
    addConnection (nSv, 0, nDac, 1);   // svfilter~ -> dac~ R

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeQuantizeExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nQuant = addNode ("time.quantize~", 80.0f, 80.0f);
    int nPhas  = addNode ("phasor~",        280.0f, 80.0f);
    int nCrush = addNode ("crush~",         280.0f, 240.0f);
    int nOut   = addNode ("out~",           280.0f, 400.0f);
    int nDac   = addNode ("dac~",           480.0f, 400.0f);

    addConnection (nQuant, 0, nPhas, 0);  // time.quantize~ -> phasor~
    addConnection (nPhas, 0, nCrush, 0);  // phasor~ -> crush~
    addConnection (nCrush, 0, nOut, 0);   // crush~ -> out~
    addConnection (nCrush, 0, nDac, 0);   // crush~ -> dac~ L
    addConnection (nCrush, 0, nDac, 1);   // crush~ -> dac~ R

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeTransportExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nTrans = addNode ("time.transport", 80.0f, 80.0f);
    int nMetro = addNode ("time.metro~",    280.0f, 80.0f);
    int nAdsr  = addNode ("adsr~",          280.0f, 240.0f);
    int nOsc   = addNode ("osc~",           480.0f, 240.0f);
    int nOut   = addNode ("out~",           380.0f, 420.0f);
    int nDac   = addNode ("dac~",           580.0f, 420.0f);

    addConnection (nTrans, 0, nMetro, 0); // time.transport timeOut -> time.metro~
    addConnection (nMetro, 0, nAdsr, 0);  // time.metro~ pulse -> adsr~ trig
    addConnection (nAdsr, 0, nOsc, 1);   // adsr~ -> osc~ gain mod
    addConnection (nOsc, 0, nOut, 0);    // osc~ -> out~
    addConnection (nOsc, 0, nDac, 0);    // osc~ -> dac~ L
    addConnection (nOsc, 0, nDac, 1);    // osc~ -> dac~ R

    detectFeedbackLoops();
}

void RelativisticNodeGraph::pushUndoState()
{
    auto currentState = saveToValueTree();

    // Truncate any redo branch if we are pushing a new action
    if (undoIndex >= 0 && undoIndex < static_cast<int>(undoStack.size()) - 1)
    {
        undoStack.erase (undoStack.begin() + undoIndex + 1, undoStack.end());
    }

    undoStack.push_back (currentState);
    undoIndex = static_cast<int>(undoStack.size()) - 1;
}

bool RelativisticNodeGraph::undo()
{
    if (canUndo())
    {
        undoIndex--;
        loadFromValueTree (undoStack[undoIndex], true);
        return true;
    }
    return false;
}

bool RelativisticNodeGraph::redo()
{
    if (canRedo())
    {
        undoIndex++;
        loadFromValueTree (undoStack[undoIndex], true);
        return true;
    }
    return false;
}

juce::ValueTree RelativisticNodeGraph::copyNodes (const std::vector<int>& nodeIds)
{
    juce::ValueTree clip ("Clipboard");

    juce::ValueTree nodesTree ("Nodes");
    std::set<int> idSet (nodeIds.begin(), nodeIds.end());

    for (int id : nodeIds)
    {
        auto n = getNodeById (id);
        if (n) nodesTree.addChild (n->saveToValueTree(), -1, nullptr);
    }
    clip.addChild (nodesTree, -1, nullptr);

    juce::ValueTree connsTree ("Connections");
    for (const auto& c : connections)
    {
        if (idSet.count (c.sourceNodeId) > 0 && idSet.count (c.destNodeId) > 0)
        {
            juce::ValueTree conn ("Conn");
            conn.setProperty ("id", c.id, nullptr);
            conn.setProperty ("srcNode", c.sourceNodeId, nullptr);
            conn.setProperty ("srcOutlet", c.sourceOutletIdx, nullptr);
            conn.setProperty ("destNode", c.destNodeId, nullptr);
            conn.setProperty ("destInlet", c.destInletIdx, nullptr);
            connsTree.addChild (conn, -1, nullptr);
        }
    }
    clip.addChild (connsTree, -1, nullptr);

    return clip;
}

std::vector<int> RelativisticNodeGraph::pasteNodes (const juce::ValueTree& clipboardData, float offsetX, float offsetY)
{
    std::vector<int> newIds;
    if (!clipboardData.isValid()) return newIds;

    pushUndoState();
    std::map<int, int> oldToNewIdMap;

    auto nodesTree = clipboardData.getChildWithName ("Nodes");
    if (nodesTree.isValid())
    {
        for (int i = 0; i < nodesTree.getNumChildren(); ++i)
        {
            auto nv = nodesTree.getChild (i);
            int oldId = nv.getProperty ("id");
            std::string typeName = nv.getProperty ("type").toString().toStdString();
            float x = static_cast<float>(nv.getProperty ("posX")) + offsetX;
            float y = static_cast<float>(nv.getProperty ("posY")) + offsetY;

            int newId = addNode (typeName, x, y);
            oldToNewIdMap[oldId] = newId;
            newIds.push_back (newId);

            auto n = getNodeById (newId);
            if (n) n->loadFromValueTree (nv);
            if (n) n->setPosition (x, y); // Maintain offset position
        }
    }

    auto connsTree = clipboardData.getChildWithName ("Connections");
    if (connsTree.isValid())
    {
        for (int i = 0; i < connsTree.getNumChildren(); ++i)
        {
            auto cv = connsTree.getChild (i);
            int oldSrc = cv.getProperty ("srcNode");
            int oldDest = cv.getProperty ("destNode");

            if (oldToNewIdMap.count (oldSrc) > 0 && oldToNewIdMap.count (oldDest) > 0)
            {
                addConnection (oldToNewIdMap[oldSrc], cv.getProperty ("srcOutlet"),
                               oldToNewIdMap[oldDest], cv.getProperty ("destInlet"));
            }
        }
    }

    return newIds;
}

std::vector<int> RelativisticNodeGraph::duplicateNodes (const std::vector<int>& nodeIds)
{
    auto clip = copyNodes (nodeIds);
    return pasteNodes (clip, 30.0f, 30.0f);
}

void RelativisticNodeGraph::cutNodes (const std::vector<int>& nodeIds)
{
    pushUndoState();
    for (int id : nodeIds)
    {
        removeNode (id);
    }
}

juce::ValueTree RelativisticNodeGraph::saveToValueTree() const
{
    juce::ValueTree tree ("RelativisticPatch");
    tree.setProperty ("audioEngineEnabled", audioEngineEnabled, nullptr);

    juce::ValueTree nodesTree ("Nodes");
    for (const auto& node : nodes)
    {
        nodesTree.addChild (node->saveToValueTree(), -1, nullptr);
    }
    tree.addChild (nodesTree, -1, nullptr);

    juce::ValueTree connsTree ("Connections");
    for (const auto& c : connections)
    {
        juce::ValueTree conn ("Conn");
        conn.setProperty ("id", c.id, nullptr);
        conn.setProperty ("srcNode", c.sourceNodeId, nullptr);
        conn.setProperty ("srcOutlet", c.sourceOutletIdx, nullptr);
        conn.setProperty ("destNode", c.destNodeId, nullptr);
        conn.setProperty ("destInlet", c.destInletIdx, nullptr);
        connsTree.addChild (conn, -1, nullptr);
    }
    tree.addChild (connsTree, -1, nullptr);

    return tree;
}

void RelativisticNodeGraph::loadFromValueTree (const juce::ValueTree& tree, bool isRestoringUndo)
{
    clearGraph();
    audioEngineEnabled = tree.getProperty ("audioEngineEnabled", false);

    auto nodesTree = tree.getChildWithName ("Nodes");
    if (nodesTree.isValid())
    {
        for (int i = 0; i < nodesTree.getNumChildren(); ++i)
        {
            auto nv = nodesTree.getChild (i);
            std::string typeName = nv.getProperty ("type").toString().toStdString();
            float x = nv.getProperty ("posX", 100.0f);
            float y = nv.getProperty ("posY", 100.0f);

            int nodeId = addNode (typeName, x, y);
            auto n = getNodeById (nodeId);
            if (n) n->loadFromValueTree (nv);
        }
    }

    auto connsTree = tree.getChildWithName ("Connections");
    if (connsTree.isValid())
    {
        for (int i = 0; i < connsTree.getNumChildren(); ++i)
        {
            auto cv = connsTree.getChild (i);
            addConnection (cv.getProperty ("srcNode"), cv.getProperty ("srcOutlet"),
                           cv.getProperty ("destNode"), cv.getProperty ("destInlet"));
        }
    }

    // Restore Persistent Undo Stack from File ONLY if not restoring during undo/redo!
    if (!isRestoringUndo)
    {
        auto undoHistoryTree = tree.getChildWithName ("UndoHistory");
        if (undoHistoryTree.isValid())
        {
            undoStack.clear();
            undoIndex = undoHistoryTree.getProperty ("undoIndex", -1);
            for (int i = 0; i < undoHistoryTree.getNumChildren(); ++i)
            {
                undoStack.push_back (undoHistoryTree.getChild (i).createCopy());
            }
        }
    }
}

bool RelativisticNodeGraph::saveProjectToFile (const juce::File& file)
{
    auto tree = saveToValueTree();
    std::unique_ptr<juce::XmlElement> xml (tree.createXml());
    if (xml != nullptr)
    {
        return xml->writeTo (file);
    }
    return false;
}

bool RelativisticNodeGraph::loadProjectFromFile (const juce::File& file)
{
    if (!file.existsAsFile()) return false;
    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));
    if (xml != nullptr)
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        if (tree.isValid())
        {
            loadFromValueTree (tree);
            return true;
        }
    }
    return false;
}

double RelativisticNodeGraph::tapSignal (const std::string& target) const
{
    if (target.empty()) return 0.0;

    std::string nodeQuery = target;
    std::string propQuery;

    size_t dotPos = target.find (".");
    if (dotPos != std::string::npos)
    {
        nodeQuery = target.substr (0, dotPos);
        propQuery = target.substr (dotPos + 1);
    }

    int targetId = 0;
    try { targetId = std::stoi (nodeQuery); } catch (...) {}

    for (const auto& node : nodes)
    {
        bool match = false;
        if (targetId > 0 && node->getId() == targetId) match = true;
        else if (node->getLabel().find (nodeQuery) != std::string::npos) match = true;
        else if (node->getTypeName().find (nodeQuery) != std::string::npos) match = true;

        if (match)
        {
            if (!propQuery.empty())
            {
                // Property Tapping (e.g. tap("filter1.cutoff"), tap("osc1.frequency"))
                if (propQuery == "gamma" || propQuery == "t")
                {
                    const auto& inlets = node->getInlets();
                    if (!inlets.empty()) return inlets[0].timeGamma;
                }
                return node->getParameter (propQuery, 0.0f);
            }

            const auto& outlets = node->getOutlets();
            if (!outlets.empty())
            {
                if (outlets[0].type == NodePortType::Control)
                {
                    return outlets[0].controlValue;
                }
                else if (outlets[0].type == NodePortType::Audio)
                {
                    // Sample peak/RMS level of tapped audio stream
                    const auto* buf = outlets[0].audioData.getReadPointer (0);
                    float p = 0.0f;
                    int nSamples = std::min (outlets[0].audioData.getNumSamples(), 64);
                    for (int s = 0; s < nSamples; ++s) p = std::max (p, std::abs (buf[s]));
                    return p;
                }
                else if (outlets[0].type == NodePortType::Time)
                {
                    return outlets[0].timeGamma;
                }
            }
        }
    }
    return 0.0;
}

void RelativisticNodeGraph::propagateTimeDilationHierarchy()
{
    // Hierarchical Time Dilation Inheritance:
    // If a node does NOT have an explicit time inlet connected, it inherits time dilation from its upstream parent nodes!
    for (const auto& conn : connections)
    {
        auto srcNode = getNodeById (conn.sourceNodeId);
        auto destNode = getNodeById (conn.destNodeId);

        if (srcNode && destNode)
        {
            const auto& srcOutlets = srcNode->getOutlets();
            auto& destInlets = destNode->getInlets();

            if (conn.sourceOutletIdx < static_cast<int>(srcOutlets.size()) &&
                conn.destInletIdx < static_cast<int>(destInlets.size()))
            {
                if (srcOutlets[conn.sourceOutletIdx].type == NodePortType::Time &&
                    destInlets[conn.destInletIdx].type != NodePortType::Time)
                {
                    // Propagate inherited time gamma to non-time inlets
                    if (!destInlets.empty())
                    {
                        destInlets[0].timeGamma = srcOutlets[conn.sourceOutletIdx].timeGamma;
                    }
                }
            }
        }
    }
}

void RelativisticNodeGraph::propagateSignals()
{
    for (auto& node : nodes)
    {
        for (auto& in : node->getInlets())
        {
            in.audioData.clear();
            in.controlValue = 0.0f;
            in.timeGamma = 1.0;
        }
    }

    propagateTimeDilationHierarchy();

    for (const auto& conn : connections)
    {
        auto srcNode = getNodeById (conn.sourceNodeId);
        auto destNode = getNodeById (conn.destNodeId);

        if (srcNode && destNode)
        {
            const auto& srcOutlets = srcNode->getOutlets();
            auto& destInlets = destNode->getInlets();

            if (conn.sourceOutletIdx < static_cast<int>(srcOutlets.size()) &&
                conn.destInletIdx < static_cast<int>(destInlets.size()))
            {
                const auto& srcPort = srcOutlets[conn.sourceOutletIdx];
                auto& destPort = destInlets[conn.destInletIdx];

                if (srcPort.type == NodePortType::Audio)
                {
                    // If Feedback Loop (destNode ID <= srcNode ID), use 1-Block History Delay Buffer (previousBlockBuffer)
                    const auto& bufferToUse = (destNode->getId() <= srcNode->getId()) ? srcPort.previousBlockBuffer : srcPort.audioData;

                    destPort.audioData.addFrom (0, 0, bufferToUse, 0, 0, bufferToUse.getNumSamples());
                    if (bufferToUse.getNumChannels() > 1)
                        destPort.audioData.addFrom (1, 0, bufferToUse, 1, 0, bufferToUse.getNumSamples());
                }
                else if (srcPort.type == NodePortType::Time)
                {
                    destPort.timeGamma = srcPort.timeGamma;
                }
                else if (srcPort.type == NodePortType::Control)
                {
                    destPort.controlValue = srcPort.controlValue;
                }
            }
        }
    }
}

void RelativisticNodeGraph::process (juce::AudioBuffer<float>& masterOutput, int numSamples)
{
    masterOutput.clear();

    // 1. Control & Time Math Propagation runs ALWAYS (even when audio is OFF)
    propagateSignals();

    // 2. Process all Node Objects
    for (auto& node : nodes)
    {
        node->process (numSamples);

        // Store 1-Block History Buffer for Feedback Loops
        for (auto& out : node->getOutlets())
        {
            if (out.type == NodePortType::Audio)
            {
                out.previousBlockBuffer.copyFrom (0, 0, out.audioData, 0, 0, numSamples);
                if (out.audioData.getNumChannels() > 1)
                    out.previousBlockBuffer.copyFrom (1, 0, out.audioData, 1, 0, numSamples);
            }
        }

        // Sum output from [dac~] objects ONLY if Audio Engine is ON
        if (audioEngineEnabled && node->getTypeName() == "dac~")
        {
            const auto& inlets = node->getInlets();
            if (inlets.size() >= 2)
            {
                masterOutput.addFrom (0, 0, inlets[0].audioData, 0, 0, numSamples, 0.7f);
                masterOutput.addFrom (1, 0, inlets[1].audioData, 0, 0, numSamples, 0.7f);
            }
        }
    }
}

} // namespace time_dilation
