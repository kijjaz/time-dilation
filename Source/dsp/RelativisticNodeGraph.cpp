#include "RelativisticNodeGraph.h"
#include "RelativisticNodeObjects.h"
#include "RelativisticTimeline.h"
#include "RelativisticSequencers.h"
#include <algorithm>

namespace time_dilation
{

static inline float interpolateHermite (float y0, float y1, float y2, float y3, float frac)
{
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * frac + c2) * frac + c1) * frac + c0;
}

RelativisticNode::RelativisticNode (int id, const std::string& typeName, const std::string& label)
    : nodeId (id), nodeTypeName (typeName), nodeLabel (label)
{
}

double RelativisticNode::getEffectiveGamma() const
{
    double targetG = 1.0;
    if (!inlets.empty() && inlets[0].type == NodePortType::Time && inlets[0].timeGamma != 0.0)
    {
        targetG = inlets[0].timeGamma;
    }
    else
    {
        auto it = parameters.find ("gamma");
        if (it != parameters.end()) targetG = static_cast<double>(it->second);
    }
    targetG = std::clamp (targetG, -16.0, 16.0);

    float filterHz = getParameter ("timeFilterHz", 10.0f);
    float bypass = getParameter ("bypassTimeFilter", 0.0f);
    float autoMatch = getParameter ("autoMatchTime", 1.0f);
    float slewRateLimit = getParameter ("maxSlewRate", 2.0f);

    if (bypass > 0.5f)
    {
        smoothedGamma = targetG;
        return targetG;
    }

    // Adaptive Cutoff Frequency Matching Future Lead Travel Time
    if (autoMatch > 0.5f)
    {
        double travelSec = std::abs (targetG - smoothedGamma) * 0.5;
        if (travelSec > 0.01)
        {
            filterHz = static_cast<float>(1.0 / (2.0 * juce::MathConstants<double>::pi * travelSec));
            filterHz = std::clamp (filterHz, 0.1f, 100.0f);
        }
    }

    // Slew-Rate Limiting + Lowpass Filter (Constant Doppler Pitch Shift)
    double diff = targetG - smoothedGamma;
    double maxStep = (slewRateLimit / currentSampleRate);

    if (std::abs (diff) > maxStep)
    {
        smoothedGamma += (diff > 0 ? maxStep : -maxStep);
    }
    else
    {
        double alpha = 1.0 - std::exp (-2.0 * juce::MathConstants<double>::pi * filterHz / currentSampleRate);
        smoothedGamma += alpha * diff;
    }

    return smoothedGamma;
}

double RelativisticNode::getRequestedFutureHorizonSec() const
{
    double effGamma = getEffectiveGamma();
    if (effGamma > 1.0)
    {
        return (effGamma - 1.0) * 0.5; // Positive time dilation generates future horizon lead
    }
    auto it = parameters.find ("lookahead");
    if (it != parameters.end()) return std::max (0.0f, it->second);

    return 0.0;
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

bool RelativisticNode::hasModulationInlet (const std::string& paramKey) const
{
    auto it = paramModInlets.find (paramKey);
    return (it != paramModInlets.end() && it->second >= 0);
}

int RelativisticNode::getModulationInletIndex (const std::string& paramKey) const
{
    auto it = paramModInlets.find (paramKey);
    return (it != paramModInlets.end()) ? it->second : -1;
}

bool RelativisticNode::removeModulationInlet (const std::string& paramKey)
{
    auto it = paramModInlets.find (paramKey);
    if (it == paramModInlets.end()) return false;

    int inletIdx = it->second;
    paramModInlets.erase (it);

    if (inletIdx >= 0 && inletIdx < static_cast<int>(inlets.size()))
    {
        inlets.erase (inlets.begin() + static_cast<size_t>(inletIdx));

        for (auto& kv : paramModInlets)
        {
            if (kv.second > inletIdx)
            {
                kv.second -= 1;
            }
        }
    }
    return true;
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

void RelativisticNode::ensureBufferSize (int requiredSamples)
{
    if (requiredSamples <= 0) return;
    int targetSamples = std::max (requiredSamples, 4096);

    for (auto& p : inlets)
    {
        if (p.audioData.getNumSamples() < targetSamples)
            p.audioData.setSize (2, targetSamples, true, true, true);
        if (p.previousBlockBuffer.getNumSamples() < targetSamples)
            p.previousBlockBuffer.setSize (2, targetSamples, true, true, true);
    }
    for (auto& p : outlets)
    {
        if (p.audioData.getNumSamples() < targetSamples)
            p.audioData.setSize (2, targetSamples, true, true, true);
        if (p.previousBlockBuffer.getNumSamples() < targetSamples)
            p.previousBlockBuffer.setSize (2, targetSamples, true, true, true);
    }
}

void RelativisticNode::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    ensureBufferSize (samplesPerBlock);
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

void RelativisticNode::loadFromValueTree (const juce::ValueTree& v, bool preserveExistingId)
{
    if (!preserveExistingId)
    {
        nodeId = v.getProperty ("id", nodeId);
    }
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

void RelativisticNode::parseLabelArguments (const std::string& label)
{
    juce::StringArray tokens;
    tokens.addTokens (label, " ", "");
    if (tokens.size() > 1)
    {
        float val = tokens[1].getFloatValue();
        if (val != 0.0f || tokens[1] == "0")
        {
            if (nodeTypeName == "osc~") setParameter ("frequency", val);
            else if (nodeTypeName == "filter~" || nodeTypeName == "svfilter~") setParameter ("cutoff", val);
            else if (nodeTypeName == "time.warp~") setParameter ("dilationGamma", val);
            else if (nodeTypeName == "time.metro~") setParameter ("bpm", val);
            else if (nodeTypeName == "gain~" || nodeTypeName == "out~") setParameter ("gain", val);
            else if (nodeTypeName == "delay~") setParameter ("delayTimeMs", val);
            else if (nodeTypeName == "number" || nodeTypeName == "num") setParameter ("value", val);
        }
    }
}

// ----------------------------------------------------
// RelativisticNodeGraph Engine Implementation
// ----------------------------------------------------

bool RelativisticNodeGraph::isValidObjectType (const std::string& typeName)
{
    static const std::set<std::string> validTypes = {
        "time.warp~", "time.retro~", "time.quantize~", "time.metro~", "time.stasis~",
        "time.singularity~", "time.transport", "time.scope", "time.display", "time.monitor",
        "time.xy", "xy", "xy~", "plot.xy", "spectrometer~", "spectrum~", "fft~",
        "time.future~", "future~", "osc~", "phasor~", "sampler~", "filter~", "delay~",
        "dac~", "expr", "expr~", "fexpr~", "gain~", "out~", "out", "env~", "tap", "tap~",
        "v", "msg", "message", "z~", "snapshot~", "+", "*", "table", "tabwrite~", "tabread~", "tabosc4~",
        "svfilter~", "drive~", "reverb~", "crush~", "adsr~", "mtof", "ftom", "midi2freq",
        "number", "num", "nb", "display", "number.display", "bang", "b", "bang~", "b~", "counter", "cnt", "note",
        "slider", "hslider", "vslider", "toggle", "tgl", "audio2time~", "a2t~", "time2audio~", "t2a~",
        "time.math~", "time.combine~", "time.+", "time.-", "time.*", "time.scale~", "time.filter~",
        "seq", "step", "euclid", "markov", "tidal", "tidal~", "fbdrum~", "drum~", "drums~",
        "drumseq", "drumstep", "timeline", "arrangement"
    };
    return validTypes.find (typeName) != validTypes.end();
}

RelativisticNodeGraph::RelativisticNodeGraph()
{
    pushUndoState();
}

void RelativisticNodeGraph::prepare (double sr, int samplesPerBlock)
{
    const juce::ScopedLock lock (processLock);
    sampleRate = sr;
    blockSize = samplesPerBlock;

    globalDelayRingBuffer.setSize (2, 524288, true, true, true);
    globalDelayRingBuffer.clear();
    globalDelayWritePos = 0;
    currentCausalityHorizonSec = 0.0;
    targetCausalityHorizonSec = 0.0;

    for (auto& n : nodes)
    {
        n->prepare (sampleRate, blockSize);
    }
}

int RelativisticNodeGraph::addNode (const std::string& typeName, float x, float y)
{
    const juce::ScopedLock lock (processLock);
    int id = nextNodeId++;
    std::shared_ptr<RelativisticNode> node = nullptr;

    if (typeName == "time.warp~")        node = std::make_shared<TimeWarpNode> (id);
    else if (typeName == "time.retro~")   node = std::make_shared<TimeRetroNode> (id);
    else if (typeName == "time.quantize~")node = std::make_shared<TimeQuantizeNode> (id);
    else if (typeName == "time.metro~")   node = std::make_shared<TimeMetroNode> (id);
    else if (typeName == "time.stasis~")  node = std::make_shared<TimeStasisNode> (id);
    else if (typeName == "time.singularity~") node = std::make_shared<TimeSingularityNode> (id);
    else if (typeName == "time.transport")node = std::make_shared<TimeTransportNode> (id);
    else if (typeName == "time.scope" || typeName == "time.display" || typeName == "time.monitor") node = std::make_shared<TimeScopeNode> (id);
    else if (typeName == "time.xy" || typeName == "xy" || typeName == "xy~" || typeName == "plot.xy") node = std::make_shared<TimeXYNode> (id);
    else if (typeName == "spectrometer~" || typeName == "spectrum~" || typeName == "fft~") node = std::make_shared<SpectrometerAudioNode> (id);
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
    else if (typeName == "v" || typeName == "msg" || typeName == "message") node = std::make_shared<ValueNode> (id);
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
    else if (typeName == "mtof" || typeName == "midi2freq") node = std::make_shared<MtofNode> (id);
    else if (typeName == "ftom")           node = std::make_shared<FtomNode> (id);
    else if (typeName == "number" || typeName == "num" || typeName == "nb" || typeName == "display" || typeName == "number.display") node = std::make_shared<NumberNode> (id);
    else if (typeName == "meter~" || typeName == "vu~") node = std::make_shared<VuMeterAudioNode> (id);
    else if (typeName == "number~" || typeName == "num~") node = std::make_shared<NumberAudioNode> (id);
    else if (typeName == "print" || typeName == "monitor") node = std::make_shared<PrintMonitorNode> (id);
    else if (typeName == "bang" || typeName == "b") node = std::make_shared<BangNode> (id);
    else if (typeName == "bang~" || typeName == "b~") node = std::make_shared<BangAudioNode> (id);
    else if (typeName == "counter" || typeName == "cnt") node = std::make_shared<CounterNode> (id);
    else if (typeName == "slider" || typeName == "hslider" || typeName == "vslider") node = std::make_shared<SliderNode> (id);
    else if (typeName == "toggle" || typeName == "tgl") node = std::make_shared<ToggleNode> (id);
    else if (typeName == "audio2time~" || typeName == "a2t~") node = std::make_shared<AudioToTimeNode> (id);
    else if (typeName == "time2audio~" || typeName == "t2a~") node = std::make_shared<TimeToAudioNode> (id);
    else if (typeName == "time.math~" || typeName == "time.combine~" || typeName == "time.+" || typeName == "time.-" || typeName == "time.*") node = std::make_shared<TimeMathNode> (id);
    else if (typeName == "time.scale~") node = std::make_shared<TimeScaleNode> (id);
    else if (typeName == "time.filter~") node = std::make_shared<TimeFilterNode> (id);
    else if (typeName == "note")           node = std::make_shared<NoteGenNode> (id);
    else if (typeName == "time.future~" || typeName == "future~") node = std::make_shared<TimeFutureNode> (id);
    else if (typeName == "seq" || typeName == "step") node = std::make_shared<StepSequencerNode> (id);
    else if (typeName == "euclid") node = std::make_shared<EuclideanSequencerNode> (id);
    else if (typeName == "markov") node = std::make_shared<MarkovSequencerNode> (id);
    else if (typeName == "tidal" || typeName == "tidal~") node = std::make_shared<TidalPatternSequencerNode> (id);
    else if (typeName == "fbdrum~" || typeName == "drum~" || typeName == "drums~") node = std::make_shared<FutureBassDrumNode> (id);
    else if (typeName == "drumseq" || typeName == "drumstep") node = std::make_shared<DrumSequencerNode> (id);
    else if (typeName == "timeline" || typeName == "arrangement") node = std::make_shared<TimelineNode> (id);
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
    const juce::ScopedLock lock (processLock);
    nodes.erase (std::remove_if (nodes.begin(), nodes.end(),
        [nodeId] (const std::shared_ptr<RelativisticNode>& n) { return n->getId() == nodeId; }), nodes.end());

    connections.erase (std::remove_if (connections.begin(), connections.end(),
        [nodeId] (const PatchConnection& c) {
            return c.sourceNodeId == nodeId || c.destNodeId == nodeId;
        }), connections.end());
}

int RelativisticNodeGraph::addConnection (int srcNodeId, int srcOutletIdx, int destNodeId, int destInletIdx)
{
    const juce::ScopedLock lock (processLock);
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
    const juce::ScopedLock lock (processLock);
    connections.erase (std::remove_if (connections.begin(), connections.end(),
        [connectionId] (const PatchConnection& c) { return c.id == connectionId; }), connections.end());
    detectFeedbackLoops();
}

bool RelativisticNodeGraph::removeModulationInlet (int nodeId, const std::string& paramKey)
{
    const juce::ScopedLock lock (processLock);
    auto node = getNodeById (nodeId);
    if (!node) return false;

    int inletIdx = node->getModulationInletIndex (paramKey);
    if (inletIdx < 0) return false;

    pushUndoState();

    std::vector<int> toRemove;
    for (const auto& c : connections)
    {
        if (c.destNodeId == nodeId && c.destInletIdx == inletIdx)
        {
            toRemove.push_back (c.id);
        }
    }
    for (int cid : toRemove)
    {
        removeConnection (cid);
    }

    for (auto& c : connections)
    {
        if (c.destNodeId == nodeId && c.destInletIdx > inletIdx)
        {
            c.destInletIdx -= 1;
        }
    }

    bool res = node->removeModulationInlet (paramKey);
    detectFeedbackLoops();
    return res;
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
    clearGraph();
}

void RelativisticNodeGraph::loadTimeWarpExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nWarp = addNode ("time.warp~", 100.0f, 100.0f);
    int nSeq  = addNode ("seq",        420.0f, 100.0f);
    int nMtof = addNode ("mtof",       420.0f, 240.0f);
    int nOsc  = addNode ("osc~",       420.0f, 380.0f);
    int nFilt = addNode ("filter~",    420.0f, 520.0f);
    int nGain = addNode ("gain~",      420.0f, 660.0f);
    int nOut  = addNode ("out~",       420.0f, 800.0f);

    auto nodeSeq = std::dynamic_pointer_cast<StepSequencerNode> (getNodeById (nSeq));
    if (nodeSeq) nodeSeq->setPatternString ("60 63 65 67 70 72 75 77");

    auto nodeOsc = getNodeById (nOsc);
    if (nodeOsc) nodeOsc->setParameter ("gain", 0.4f);

    auto nodeFilt = getNodeById (nFilt);
    if (nodeFilt) nodeFilt->setParameter ("cutoff", 1600.0f);

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.5f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.6f);

    addConnection (nWarp, 0, nSeq, 0);   // time.warp~ timeOut -> seq timeIn
    addConnection (nWarp, 0, nOsc, 0);   // time.warp~ timeOut -> osc~ timeIn
    addConnection (nSeq, 0, nMtof, 0);   // seq pitch (MIDI note) -> mtof note
    addConnection (nMtof, 0, nOsc, 1);   // mtof freq (Hz) -> osc~ freq (Inlet 1)
    addConnection (nOsc, 0, nFilt, 1);   // osc~ out~ -> filter~ in~ (Inlet 1)
    addConnection (nFilt, 0, nGain, 1);  // filter~ out~ -> gain~ in~ (Inlet 1)
    addConnection (nGain, 0, nOut, 1);   // gain~ out~ -> out~ inL~ (Inlet 1)
    addConnection (nGain, 0, nOut, 2);   // gain~ out~ -> out~ inR~ (Inlet 2)

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeRetroExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nRetro = addNode ("time.retro~", 100.0f, 100.0f);
    int nMetro = addNode ("time.metro~", 420.0f, 100.0f);
    int nSamp  = addNode ("sampler~",    420.0f, 240.0f);
    int nDrive = addNode ("drive~",      420.0f, 380.0f);
    int nGain  = addNode ("gain~",       420.0f, 520.0f);
    int nOut   = addNode ("out~",        420.0f, 660.0f);

    auto nodeMetro = getNodeById (nMetro);
    if (nodeMetro) nodeMetro->setParameter ("bpm", 130.0f);

    auto nodeSamp = getNodeById (nSamp);
    if (nodeSamp)
    {
        nodeSamp->setParameter ("gain", 0.45f);
        nodeSamp->setParameter ("playbackSpeed", -1.0f);
        nodeSamp->setParameter ("loopMode", 0.0f); // Continuous loop
    }

    auto nodeDrive = getNodeById (nDrive);
    if (nodeDrive) nodeDrive->setParameter ("drive", 1.3f);

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.55f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.6f);

    addConnection (nRetro, 0, nMetro, 0); // time.retro~ timeOut -> time.metro~ timeIn
    addConnection (nRetro, 0, nSamp, 0);  // time.retro~ timeOut -> sampler~ timeIn
    addConnection (nMetro, 0, nSamp, 1);  // time.metro~ pulse~ -> sampler~ scrub (Inlet 1)
    addConnection (nSamp, 0, nDrive, 1);  // sampler~ out~ -> drive~ in~ (Inlet 1)
    addConnection (nDrive, 0, nGain, 1);  // drive~ out~ -> gain~ in~ (Inlet 1)
    addConnection (nGain, 0, nOut, 1);   // gain~ out~ -> out~ inL~ (Inlet 1)
    addConnection (nGain, 0, nOut, 2);   // gain~ out~ -> out~ inR~ (Inlet 2)

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeStasisExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nStasis = addNode ("time.stasis~", 100.0f, 100.0f);
    int nSeq    = addNode ("seq",          420.0f, 100.0f);
    int nMtof   = addNode ("mtof",         420.0f, 240.0f);
    int nOsc    = addNode ("osc~",         420.0f, 380.0f);
    int nReverb = addNode ("reverb~",      420.0f, 520.0f);
    int nGain   = addNode ("gain~",        420.0f, 660.0f);
    int nOut    = addNode ("out~",         420.0f, 800.0f);

    auto nodeStasis = getNodeById (nStasis);
    if (nodeStasis) nodeStasis->setParameter ("freeze", 0.0f);

    auto nodeSeq = std::dynamic_pointer_cast<StepSequencerNode> (getNodeById (nSeq));
    if (nodeSeq) nodeSeq->setPatternString ("62 65 67 69 74 77");

    auto nodeOsc = getNodeById (nOsc);
    if (nodeOsc) nodeOsc->setParameter ("gain", 0.35f);

    auto nodeRev = getNodeById (nReverb);
    if (nodeRev)
    {
        nodeRev->setParameter ("roomSize", 0.8f);
        nodeRev->setParameter ("damping", 0.3f);
    }

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.45f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.6f);

    addConnection (nStasis, 0, nSeq, 0);   // time.stasis~ timeOut -> seq timeIn
    addConnection (nStasis, 0, nOsc, 0);   // time.stasis~ timeOut -> osc~ timeIn
    addConnection (nSeq, 0, nMtof, 0);    // seq pitch -> mtof note
    addConnection (nMtof, 0, nOsc, 1);    // mtof freq -> osc~ freq (Inlet 1)
    addConnection (nOsc, 0, nReverb, 1);   // osc~ out~ -> reverb~ inL~ (Inlet 1)
    addConnection (nReverb, 0, nGain, 1);  // reverb~ out~ -> gain~ in~ (Inlet 1)
    addConnection (nGain, 0, nOut, 1);    // gain~ out~ -> out~ inL~ (Inlet 1)
    addConnection (nGain, 0, nOut, 2);    // gain~ out~ -> out~ inR~ (Inlet 2)

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeSingularityExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nSing   = addNode ("time.singularity~", 100.0f, 100.0f);
    int nPhasor = addNode ("phasor~",           420.0f, 100.0f);
    int nSv     = addNode ("svfilter~",         420.0f, 240.0f);
    int nGain   = addNode ("gain~",             420.0f, 380.0f);
    int nOut    = addNode ("out~",              420.0f, 520.0f);

    auto nodePhasor = getNodeById (nPhasor);
    if (nodePhasor) nodePhasor->setParameter ("frequency", 110.0f);

    auto nodeSv = getNodeById (nSv);
    if (nodeSv)
    {
        nodeSv->setParameter ("cutoff", 1200.0f);
        nodeSv->setParameter ("resonance", 1.2f);
    }

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.45f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.6f);

    addConnection (nSing, 0, nPhasor, 0); // time.singularity~ timeOut -> phasor~ timeIn
    addConnection (nPhasor, 0, nSv, 1);   // phasor~ out~ -> svfilter~ in~ (Inlet 1)
    addConnection (nSv, 0, nGain, 1);     // svfilter~ lp~ (Outlet 0) -> gain~ in~ (Inlet 1)
    addConnection (nGain, 0, nOut, 1);    // gain~ out~ -> out~ inL~ (Inlet 1)
    addConnection (nGain, 0, nOut, 2);    // gain~ out~ -> out~ inR~ (Inlet 2)

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeQuantizeExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nQuant = addNode ("time.quantize~", 100.0f, 100.0f);
    int nSeq   = addNode ("seq",            420.0f, 100.0f);
    int nOsc   = addNode ("osc~",           420.0f, 240.0f);
    int nCrush = addNode ("crush~",         420.0f, 380.0f);
    int nGain  = addNode ("gain~",          420.0f, 520.0f);
    int nOut   = addNode ("out~",           420.0f, 660.0f);

    auto nodeSeq = std::dynamic_pointer_cast<StepSequencerNode> (getNodeById (nSeq));
    if (nodeSeq) nodeSeq->setPatternString ("60 64 67 71 72 76 79 83");

    auto nodeOsc = getNodeById (nOsc);
    if (nodeOsc) nodeOsc->setParameter ("gain", 0.35f);

    auto nodeCrush = getNodeById (nCrush);
    if (nodeCrush)
    {
        nodeCrush->setParameter ("bits", 6.0f);
        nodeCrush->setParameter ("downsample", 2.0f);
    }

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.45f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.6f);

    addConnection (nQuant, 0, nSeq, 0);   // time.quantize~ timeOut -> seq timeIn
    addConnection (nQuant, 0, nOsc, 0);   // time.quantize~ timeOut -> osc~ timeIn
    addConnection (nSeq, 0, nOsc, 1);     // seq pitch -> osc~ freq (Inlet 1)
    addConnection (nOsc, 0, nCrush, 1);   // osc~ out~ -> crush~ in~ (Inlet 1)
    addConnection (nCrush, 0, nGain, 1);  // crush~ out~ -> gain~ in~ (Inlet 1)
    addConnection (nGain, 0, nOut, 1);    // gain~ out~ -> out~ inL~ (Inlet 1)
    addConnection (nGain, 0, nOut, 2);    // gain~ out~ -> out~ inR~ (Inlet 2)

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTimeTransportExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nTrans = addNode ("time.transport", 80.0f, 100.0f);
    int nMetro = addNode ("time.metro~",    420.0f, 100.0f);
    int nSeq   = addNode ("seq",            420.0f, 240.0f);
    int nOsc   = addNode ("osc~",           420.0f, 380.0f);
    int nFilt  = addNode ("filter~",        420.0f, 520.0f);
    int nGain  = addNode ("gain~",          420.0f, 660.0f);
    int nOut   = addNode ("out~",           420.0f, 800.0f);

    auto nodeSeq = std::dynamic_pointer_cast<StepSequencerNode> (getNodeById (nSeq));
    if (nodeSeq) nodeSeq->setPatternString ("55 58 60 63 65 67 70 72");

    auto nodeOsc = getNodeById (nOsc);
    if (nodeOsc) nodeOsc->setParameter ("gain", 0.35f);

    auto nodeFilt = getNodeById (nFilt);
    if (nodeFilt) nodeFilt->setParameter ("cutoff", 2200.0f);

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.45f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.6f);

    addConnection (nTrans, 0, nMetro, 0); // time.transport timeOut -> time.metro~ timeIn
    addConnection (nMetro, 0, nSeq, 1);   // time.metro~ pulse~ -> seq trig (Inlet 1)
    addConnection (nSeq, 0, nOsc, 1);     // seq pitch -> osc~ freq (Inlet 1)
    addConnection (nOsc, 0, nFilt, 1);    // osc~ out~ -> filter~ in~ (Inlet 1)
    addConnection (nFilt, 0, nGain, 1);   // filter~ out~ -> gain~ in~ (Inlet 1)
    addConnection (nGain, 0, nOut, 1);    // gain~ out~ -> out~ inL~ (Inlet 1)
    addConnection (nGain, 0, nOut, 2);    // gain~ out~ -> out~ inR~ (Inlet 2)

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadTableExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nTable  = addNode ("table",   100.0f, 100.0f);
    int nSeq    = addNode ("seq",     420.0f, 100.0f);
    int nTabOsc = addNode ("tabosc4~", 420.0f, 240.0f);
    int nFilter = addNode ("filter~", 420.0f, 380.0f);
    int nGain   = addNode ("gain~",   420.0f, 520.0f);
    int nOut    = addNode ("out~",    420.0f, 660.0f);

    auto nodeSeq = std::dynamic_pointer_cast<StepSequencerNode> (getNodeById (nSeq));
    if (nodeSeq) nodeSeq->setPatternString ("50 53 57 60 62 65 69 72");

    auto nodeFilter = getNodeById (nFilter);
    if (nodeFilter) nodeFilter->setParameter ("cutoff", 1600.0f);

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.45f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.6f);

    addConnection (nSeq, 0, nTabOsc, 1);    // seq pitch -> tabosc4~ freq (Inlet 1)
    addConnection (nTabOsc, 0, nFilter, 1); // tabosc4~ out~ -> filter~ in~ (Inlet 1)
    addConnection (nFilter, 0, nGain, 1);   // filter~ out~ -> gain~ in~ (Inlet 1)
    addConnection (nGain, 0, nOut, 1);      // gain~ out~ -> out~ inL~ (Inlet 1)
    addConnection (nGain, 0, nOut, 2);      // gain~ out~ -> out~ inR~ (Inlet 2)

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadFutureBassDrumExamplePatch()
{
    pushUndoState();
    clearGraph();
    int nWarp  = addNode ("time.warp~", 100.0f, 100.0f);
    int nDSeq  = addNode ("drumseq",    420.0f, 100.0f);
    int nFDrum = addNode ("fbdrum~",    420.0f, 260.0f);
    int nDrive = addNode ("drive~",      420.0f, 420.0f);
    int nGain  = addNode ("gain~",       420.0f, 560.0f);
    int nOut   = addNode ("out~",        420.0f, 700.0f);

    auto nodeWarp = getNodeById (nWarp);
    if (nodeWarp) nodeWarp->setParameter ("gamma", 1.15f);

    auto nodeDSeq = getNodeById (nDSeq);
    if (nodeDSeq) nodeDSeq->setLabel ("Future Bass Trap Beat");

    auto nodeFDrum = getNodeById (nFDrum);
    if (nodeFDrum)
    {
        nodeFDrum->setParameter ("kickPitch", 45.0f);
        nodeFDrum->setParameter ("drive", 1.8f);
    }

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.7f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.65f);

    addConnection (nWarp, 0, nDSeq, 0);     // time.warp~ -> drumseq timeIn
    addConnection (nWarp, 0, nFDrum, 0);    // time.warp~ -> fbdrum~ timeIn
    addConnection (nDSeq, 0, nFDrum, 1);    // drumseq midiNote -> fbdrum~ midiIn
    addConnection (nDSeq, 1, nFDrum, 2);    // drumseq trig -> fbdrum~ trig
    addConnection (nDSeq, 2, nFDrum, 3);    // drumseq vel -> fbdrum~ vel
    addConnection (nFDrum, 0, nDrive, 1);   // fbdrum~ outL~ -> drive~ inL~
    addConnection (nDrive, 0, nGain, 1);    // drive~ out~ -> gain~ in~
    addConnection (nGain, 0, nOut, 1);      // gain~ out~ -> out~ inL~
    addConnection (nGain, 0, nOut, 2);      // gain~ out~ -> out~ inR~

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadRhythmicTimeWarpingExamplePatch()
{
    pushUndoState();
    clearGraph();
    int nWarp  = addNode ("time.warp~", 100.0f, 100.0f);
    int nDSeq  = addNode ("drumseq",    420.0f, 100.0f);
    int nFDrum = addNode ("fbdrum~",    420.0f, 260.0f);
    int nGain  = addNode ("gain~",       420.0f, 420.0f);
    int nOut   = addNode ("out~",        420.0f, 560.0f);

    auto nodeWarp = getNodeById (nWarp);
    if (nodeWarp) nodeWarp->setParameter ("dilationGamma", 2.0f);

    auto nodeDSeq = getNodeById (nDSeq);
    if (nodeDSeq) nodeDSeq->setLabel ("Jersey Club / Future Bounce");

    auto nodeFDrum = getNodeById (nFDrum);
    if (nodeFDrum)
    {
        nodeFDrum->setParameter ("pitchDilation", 0.0f); // STEADY PITCH (No Doppler pitch shift!)
        nodeFDrum->setParameter ("kickPitch", 45.0f);
    }

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.75f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.65f);

    addConnection (nWarp, 0, nDSeq, 0);     // time.warp~ -> drumseq timeIn (WARPS RHYTHM TEMPO & ROLLS!)
    addConnection (nDSeq, 0, nFDrum, 1);    // drumseq midiNote -> fbdrum~ midiIn
    addConnection (nDSeq, 1, nFDrum, 2);    // drumseq trig -> fbdrum~ trig
    addConnection (nDSeq, 2, nFDrum, 3);    // drumseq vel -> fbdrum~ vel
    addConnection (nFDrum, 0, nGain, 1);    // fbdrum~ outL~ -> gain~ in~
    addConnection (nGain, 0, nOut, 1);      // gain~ out~ -> out~ inL~
    addConnection (nGain, 0, nOut, 2);      // gain~ out~ -> out~ inR~

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadSoundPitchWarpingExamplePatch()
{
    pushUndoState();
    clearGraph();
    int nWarp  = addNode ("time.warp~", 100.0f, 100.0f);
    int nDSeq  = addNode ("drumseq",    420.0f, 100.0f);
    int nFDrum = addNode ("fbdrum~",    420.0f, 260.0f);
    int nGain  = addNode ("gain~",       420.0f, 420.0f);
    int nOut   = addNode ("out~",        420.0f, 560.0f);

    auto nodeWarp = getNodeById (nWarp);
    if (nodeWarp) nodeWarp->setParameter ("dilationGamma", 1.8f);

    auto nodeDSeq = getNodeById (nDSeq);
    if (nodeDSeq) nodeDSeq->setLabel ("Future Bass Trap Beat");

    auto nodeFDrum = getNodeById (nFDrum);
    if (nodeFDrum)
    {
        nodeFDrum->setParameter ("pitchDilation", 1.0f); // SOUND PITCH DOPPLER WARPING ENABLED!
        nodeFDrum->setParameter ("kickPitch", 45.0f);
    }

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.75f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.65f);

    addConnection (nWarp, 0, nFDrum, 0);    // time.warp~ -> fbdrum~ timeIn (WARPS AUDIO SOUND PITCH!)
    addConnection (nDSeq, 0, nFDrum, 1);    // drumseq midiNote -> fbdrum~ midiIn
    addConnection (nDSeq, 1, nFDrum, 2);    // drumseq trig -> fbdrum~ trig
    addConnection (nDSeq, 2, nFDrum, 3);    // drumseq vel -> fbdrum~ vel
    addConnection (nFDrum, 0, nGain, 1);    // fbdrum~ outL~ -> gain~ in~
    addConnection (nGain, 0, nOut, 1);      // gain~ out~ -> out~ inL~
    addConnection (nGain, 0, nOut, 2);      // gain~ out~ -> out~ inR~

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
            if (n) n->loadFromValueTree (nv, true);
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

    int maxLoadedId = 0;

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
            if (n)
            {
                n->loadFromValueTree (nv, false);
                maxLoadedId = std::max (maxLoadedId, n->getId());
            }
        }
    }

    if (maxLoadedId >= nextNodeId)
    {
        nextNodeId = maxLoadedId + 1;
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
    // Multi-pass Topological Time Dilation Inheritance:
    // Propagate time dilation factor (gamma) down multi-stage node chains
    for (int pass = 0; pass < 4; ++pass)
    {
        for (const auto& conn : connections)
        {
            auto srcNode = getNodeById (conn.sourceNodeId);
            auto destNode = getNodeById (conn.destNodeId);

            if (srcNode && destNode)
            {
                double srcGamma = srcNode->getEffectiveGamma();

                const auto& srcOutlets = srcNode->getOutlets();
                auto& destInlets = destNode->getInlets();

                if (conn.sourceOutletIdx < static_cast<int>(srcOutlets.size()) &&
                    conn.destInletIdx < static_cast<int>(destInlets.size()))
                {
                    if (srcOutlets[conn.sourceOutletIdx].type == NodePortType::Time)
                    {
                        destInlets[conn.destInletIdx].timeGamma = srcOutlets[conn.sourceOutletIdx].timeGamma;
                    }
                    else if (!destInlets.empty())
                    {
                        if (destInlets[0].timeGamma == 1.0)
                        {
                            destInlets[0].timeGamma = srcGamma;
                        }
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

                if (srcPort.type == NodePortType::Audio || srcPort.type == NodePortType::Control || srcPort.type == NodePortType::Time)
                {
                    // If Feedback Loop, use 1-Block History Delay Buffer (previousBlockBuffer)
                    const auto& bufferToUse = conn.isFeedbackLoop ? srcPort.previousBlockBuffer : srcPort.audioData;

                    int numCopy = std::min ({ destPort.audioData.getNumSamples(), bufferToUse.getNumSamples(), blockSize });
                    if (numCopy > 0)
                    {
                        destPort.audioData.addFrom (0, 0, bufferToUse, 0, 0, numCopy);
                        if (bufferToUse.getNumChannels() > 1 && destPort.audioData.getNumChannels() > 1)
                            destPort.audioData.addFrom (1, 0, bufferToUse, 1, 0, numCopy);
                    }

                    destPort.controlValue = srcPort.controlValue;
                    destPort.timeGamma = srcPort.timeGamma;
                }
            }
        }
    }
}

void RelativisticNodeGraph::process (juce::AudioBuffer<float>& masterOutput, int numSamples)
{
    const juce::ScopedLock lock (processLock);
    masterOutput.clear();

    // 1. Control & Time Math Propagation runs ALWAYS (even when audio is OFF)
    propagateSignals();

    // 2. Process all Node Objects
    for (auto& node : nodes)
    {
        node->ensureBufferSize (numSamples);
        node->process (numSamples);

        // Store 1-Block History Buffer for Feedback Loops & Sanitize Audio Output
        for (auto& out : node->getOutlets())
        {
            if (out.type == NodePortType::Audio)
            {
                for (int ch = 0; ch < out.audioData.getNumChannels(); ++ch)
                {
                    auto* samples = out.audioData.getWritePointer (ch);
                    for (int s = 0; s < numSamples; ++s)
                    {
                        float v = samples[s];
                        if (!std::isfinite (v)) v = 0.0f;
                        else if (v > 4.0f) v = 4.0f;
                        else if (v < -4.0f) v = -4.0f;
                        samples[s] = v;
                    }
                }

                out.previousBlockBuffer.copyFrom (0, 0, out.audioData, 0, 0, numSamples);
                if (out.audioData.getNumChannels() > 1)
                    out.previousBlockBuffer.copyFrom (1, 0, out.audioData, 1, 0, numSamples);
            }
            else if (out.type == NodePortType::Time)
            {
                float gVal = static_cast<float>(out.timeGamma);
                for (int ch = 0; ch < out.audioData.getNumChannels(); ++ch)
                {
                    float* d = out.audioData.getWritePointer (ch);
                    for (int s = 0; s < numSamples; ++s) d[s] = gVal;
                }
                out.previousBlockBuffer.copyFrom (0, 0, out.audioData, 0, 0, numSamples);
                if (out.audioData.getNumChannels() > 1)
                    out.previousBlockBuffer.copyFrom (1, 0, out.audioData, 1, 0, numSamples);
            }
        }
    }

    propagateSignals();

    // Sum output from [out~], [out], and [dac~] objects ONLY if Audio Engine is ON
    if (audioEngineEnabled)
    {
        for (auto& node : nodes)
        {
            if (node->getTypeName() == "out~" || node->getTypeName() == "out")
            {
                const auto& inlets = node->getInlets();
                float vol = node->getParameter ("volume", 0.8f);
                if (inlets.size() >= 3 && inlets[1].audioData.getNumSamples() >= numSamples)
                {
                    masterOutput.addFrom (0, 0, inlets[1].audioData, 0, 0, numSamples, vol);
                    if (inlets[2].audioData.getMagnitude (0, numSamples) > 0.00001f)
                        masterOutput.addFrom (1, 0, inlets[2].audioData, 0, 0, numSamples, vol);
                    else
                        masterOutput.addFrom (1, 0, inlets[1].audioData, 0, 0, numSamples, vol);
                }
            }
            else if (node->getTypeName() == "dac~")
            {
                const auto& inlets = node->getInlets();
                if (inlets.size() >= 2 && inlets[0].audioData.getNumSamples() >= numSamples)
                {
                    masterOutput.addFrom (0, 0, inlets[0].audioData, 0, 0, numSamples, 0.7f);
                    masterOutput.addFrom (1, 0, inlets[1].audioData, 0, 0, numSamples, 0.7f);
                }
                else if (!inlets.empty() && inlets[0].audioData.getNumSamples() >= numSamples)
                {
                    masterOutput.addFrom (0, 0, inlets[0].audioData, 0, 0, numSamples, 0.7f);
                    masterOutput.addFrom (1, 0, inlets[0].audioData, 0, 0, numSamples, 0.7f);
                }
            }
        }

        // Master Output Safety Limiter (Prevents clipping & Inf/NaN output)
        for (int ch = 0; ch < masterOutput.getNumChannels(); ++ch)
        {
            auto* samples = masterOutput.getWritePointer (ch);
            for (int s = 0; s < numSamples; ++s)
            {
                float v = samples[s];
                if (!std::isfinite (v)) v = 0.0f;
                else v = std::tanh (v * 0.95f);
                samples[s] = v;
            }
        }

        // Global Causality Horizon Engine: Render Master Audio Output through Hermite Fractional Delay
        double maxFutureSec = 0.0;
        for (const auto& n : nodes)
        {
            maxFutureSec = std::max (maxFutureSec, n->getRequestedFutureHorizonSec());
        }
        targetCausalityHorizonSec = std::clamp (maxFutureSec, 0.0, 10.0);

        double alphaH = 1.0 - std::exp (-2.0 * juce::MathConstants<double>::pi * causalitySmoothingHz / sampleRate);

        const int ringLen = globalDelayRingBuffer.getNumSamples();
        if (ringLen > 0)
        {
            auto* ringL = globalDelayRingBuffer.getWritePointer (0);
            auto* ringR = (globalDelayRingBuffer.getNumChannels() > 1) ? globalDelayRingBuffer.getWritePointer (1) : ringL;

            auto* outL = masterOutput.getWritePointer (0);
            auto* outR = (masterOutput.getNumChannels() > 1) ? masterOutput.getWritePointer (1) : outL;

            for (int s = 0; s < numSamples; ++s)
            {
                currentCausalityHorizonSec += alphaH * (targetCausalityHorizonSec - currentCausalityHorizonSec);

                float rawL = outL[s];
                float rawR = outR[s];

                ringL[globalDelayWritePos] = rawL;
                ringR[globalDelayWritePos] = rawR;

                double delaySamples = currentCausalityHorizonSec * sampleRate;
                if (delaySamples > 0.5)
                {
                    double readPos = globalDelayWritePos - delaySamples;
                    while (readPos < 0.0) readPos += ringLen;
                    while (readPos >= ringLen) readPos -= ringLen;

                    int rIdx = static_cast<int>(readPos);
                    float frac = static_cast<float>(readPos - rIdx);

                    int i0 = (rIdx - 1 + ringLen) % ringLen;
                    int i1 = rIdx;
                    int i2 = (rIdx + 1) % ringLen;
                    int i3 = (rIdx + 2) % ringLen;

                    outL[s] = interpolateHermite (ringL[i0], ringL[i1], ringL[i2], ringL[i3], frac);
                    outR[s] = interpolateHermite (ringR[i0], ringR[i1], ringR[i2], ringR[i3], frac);
                }

                globalDelayWritePos = (globalDelayWritePos + 1) % ringLen;
            }
        }
    }
}

} // namespace time_dilation
