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
    if (!inlets.empty() && inlets[0].type == NodePortType::Time && inlets[0].isConnected)
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

double RelativisticNode::getRelativisticGamma (double defaultTimeAmt, int /*defaultTimeMode*/) const
{
    double rawGamma = getEffectiveGamma();
    float timeAmt = getParameter ("timeAmt", static_cast<float>(defaultTimeAmt));
    return 1.0 + (rawGamma - 1.0) * timeAmt;
}

void RelativisticNode::addMessagePorts()
{
    addInlet ("msgIn", NodePortType::Message);
    addOutlet ("msgOut", NodePortType::Message);
}

void RelativisticNode::addUniversalPorts()
{
    bool hasTimeIn = false;
    for (const auto& in : inlets) {
        if (in.type == NodePortType::Time || in.name == "timeIn" || in.name == "time") { hasTimeIn = true; break; }
    }
    if (!hasTimeIn) addInlet ("timeIn", NodePortType::Time);

    bool hasMsgIn = false;
    for (const auto& in : inlets) {
        if (in.type == NodePortType::Message || in.name == "msgIn" || in.name == "msg") { hasMsgIn = true; break; }
    }
    if (!hasMsgIn) addInlet ("msgIn", NodePortType::Message);

    bool hasTimeOut = false;
    for (const auto& out : outlets) {
        if (out.type == NodePortType::Time || out.name == "timeOut" || out.name == "time") { hasTimeOut = true; break; }
    }
    if (!hasTimeOut) addOutlet ("timeOut", NodePortType::Time);

    bool hasMsgOut = false;
    for (const auto& out : outlets) {
        if (out.type == NodePortType::Message || out.name == "msgOut" || out.name == "msg") { hasMsgOut = true; break; }
    }
    if (!hasMsgOut) addOutlet ("msgOut", NodePortType::Message);
}

void RelativisticNode::receiveMessage (const std::string& msg, float val)
{
    if (msg.empty()) return;

    // Forward message down msgOut outlet if present & connected
    for (size_t i = 0; i < outlets.size(); ++i)
    {
        if (outlets[i].type == NodePortType::Message)
        {
            outlets[i].messageValue = msg;
            if (parentGraph != nullptr)
            {
                parentGraph->sendPortMessage (nodeId, static_cast<int>(i), msg);
            }
        }
    }

    juce::String jMsg (msg);
    auto tokens = juce::StringArray::fromTokens (jMsg.trim(), " ", "");

    if (tokens.isEmpty()) return;

    juce::String cmd = tokens[0].toLowerCase();

    // 1. Play / Start / Trigger
    if (cmd == "play" || cmd == "start" || cmd == "trig" || cmd == "trigger" || cmd == "on")
    {
        for (const auto& method : getExposedMethods())
        {
            if (method.find ("Play") != std::string::npos || method.find ("Trig") != std::string::npos || method.find ("Start") != std::string::npos)
            {
                invokeMethod (method);
                return;
            }
        }
        setParameter ("playState", 1.0f);
        return;
    }

    // 2. Stop / Pause / Off
    if (cmd == "stop" || cmd == "pause" || cmd == "off")
    {
        for (const auto& method : getExposedMethods())
        {
            if (method.find ("Stop") != std::string::npos || method.find ("Pause") != std::string::npos)
            {
                invokeMethod (method);
                return;
            }
        }
        setParameter ("playState", 0.0f);
        return;
    }

    // 3. Parameter Key Value Assignment (e.g., "frequency 440" or "gain 0.8" or "bpm 140")
    if (tokens.size() >= 2)
    {
        std::string paramKey = tokens[0].toStdString();
        float paramVal = tokens[1].getFloatValue();
        setParameter (paramKey, paramVal);
        return;
    }

    // 4. Single parameter value fallback
    if (jMsg.containsOnly ("0123456789.-+"))
    {
        float singleVal = jMsg.getFloatValue();
        if (!parameters.empty())
        {
            parameters.begin()->second = singleVal;
        }
        return;
    }

    // 5. Method name invocation fallback
    for (const auto& method : getExposedMethods())
    {
        if (juce::String (method).equalsIgnoreCase (jMsg))
        {
            invokeMethod (method);
            return;
        }
    }
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
        p.minValue = -100.0f;
        p.maxValue = 100.0f;

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
    audioDelayLine.prepare (sampleRate, 2, 5.0);
}

void RelativisticNode::processAudioTimeDelay (juce::AudioBuffer<float>& buffer, double /*effectiveGamma*/)
{
    int numSamples = buffer.getNumSamples();
    if (numSamples <= 0 || buffer.getNumChannels() <= 0) return;

    const float* src = buffer.getReadPointer (0);
    for (int s = 0; s < numSamples; ++s)
    {
        audioDelayLine.writeSample (0, src[s]);
        audioDelayLine.advanceWritePos();
    }
}

void RelativisticNode::processControlTimePipe (double currentTau, double /*effectiveGamma*/)
{
    for (size_t i = 0; i < outlets.size(); ++i)
    {
        if (outlets[i].type == NodePortType::Control)
        {
            float val = outlets[i].controlValue;
            controlMessagePipe.pushMessage (currentTau, val, "", false, static_cast<int>(i));
        }
    }

    auto pending = controlMessagePipe.popPendingMessages (currentTau);
    for (const auto& msg : pending)
    {
        if (msg.portIndex >= 0 && msg.portIndex < static_cast<int>(outlets.size()))
        {
            outlets[msg.portIndex].controlValue = msg.value;
        }
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
    v.setProperty ("showDelayline", showDelayline, nullptr);
    v.setProperty ("showPipe", showPipe, nullptr);

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
    showDelayline = v.getProperty ("showDelayline", showDelayline);
    showPipe = v.getProperty ("showPipe", showPipe);

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
        "time.warp", "time.warp~", "time.retro", "time.retro~", "time.quantize", "time.quantize~", "time.metro", "time.metro~", "time.stasis", "time.stasis~",
        "time.singularity", "time.singularity~", "time.transport", "time.scope", "time.display", "time.monitor",
        "time.xy", "xy", "xy~", "plot.xy", "spectrometer~", "spectrum~", "fft~",
        "time.future", "time.future~", "future~", "osc~", "phasor~", "sampler~", "filter~", "delay~",
        "dac~", "expr", "expr~", "fexpr~", "gain~", "out~", "out", "env~", "tap", "tap~", "send", "s", "receive", "r",
        "v", "msg", "message", "z~", "snapshot~", "+", "*", "table", "tabwrite~", "tabread~", "tabosc4~",
        "svfilter~", "drive~", "reverb~", "crush~", "adsr~", "mtof", "ftom", "midi2freq",
        "number", "num", "nb", "display", "number.display", "bang", "b", "bang~", "b~", "counter", "cnt", "metro", "metronome", "note",
        "slider", "hslider", "vslider", "toggle", "tgl", "audio2time~", "a2t~", "time2audio~", "t2a~",
        "time.math", "time.math~", "time.combine", "time.combine~", "time.+", "time.-", "time.*", "time.scale", "time.scale~", "time.filter", "time.filter~",
        "time.boost", "time.boost~", "time.lorenz", "time.lorenz~", "time.noise", "time.noise~", "time.rand", "time.rand~", "time.samplehold", "time.samplehold~", "time.sh", "time.sh~",
        "time.invert", "time.invert~", "time.reciprocal", "time.reciprocal~", "time.logic", "time.logic~", "time.gate", "time.gate~", "time.delay", "time.delay~",
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

void RelativisticNodeGraph::sendPortMessage (int sourceNodeId, int sourceOutletIdx, const std::string& msgText) const
{
    if (msgText.empty()) return;

    // 1. Direct cabled message routing down emerald message cords
    for (const auto& conn : connections)
    {
        if (conn.sourceNodeId == sourceNodeId && conn.sourceOutletIdx == sourceOutletIdx)
        {
            auto destNode = const_cast<RelativisticNodeGraph*>(this)->getNodeById (conn.destNodeId);
            if (destNode)
            {
                destNode->receiveMessage (msgText);
            }
        }
    }

    // 2. Wireless Target Object Addressing: "nodeLabel.method" or "nodeLabel.param value"
    size_t dotPos = msgText.find ('.');
    if (dotPos != std::string::npos && dotPos > 0)
    {
        std::string targetLabel = msgText.substr (0, dotPos);
        std::string command = msgText.substr (dotPos + 1);

        for (auto& node : nodes)
        {
            if (node->getLabel() == targetLabel || node->getTypeName() == targetLabel)
            {
                node->receiveMessage (command);
            }
        }
    }

    // 3. Global Variable Assignment: "$var = 120"
    if (msgText.rfind ("$", 0) == 0)
    {
        size_t eqPos = msgText.find ('=');
        if (eqPos != std::string::npos)
        {
            std::string varName = juce::String (msgText.substr (1, eqPos - 1)).trim().toStdString();
            float varVal = juce::String (msgText.substr (eqPos + 1)).trim().getFloatValue();
            const_cast<RelativisticNodeGraph*>(this)->globalVariables[varName] = varVal;
        }
    }
}

void RelativisticNodeGraph::broadcastBusMessage (const std::string& busName, const std::string& msgText, float val) const
{
    if (busName.empty()) return;
    for (auto& node : nodes)
    {
        std::string tName = node->getTypeName();
        std::string lbl = node->getLabel();
        if (tName == "receive" || tName == "r" || lbl == busName)
        {
            std::string nodeBus = lbl;
            if (nodeBus.rfind ("r ", 0) == 0 || nodeBus.rfind ("receive ", 0) == 0)
            {
                auto spacePos = nodeBus.find (' ');
                if (spacePos != std::string::npos) nodeBus = nodeBus.substr (spacePos + 1);
            }
            if (nodeBus == busName || lbl == busName)
            {
                node->receiveMessage (msgText, val);
            }
        }
    }
}

int RelativisticNodeGraph::addNode (const std::string& typeName, float x, float y)
{
    const juce::ScopedLock lock (processLock);
    int id = nextNodeId++;
    std::shared_ptr<RelativisticNode> node = nullptr;

    if (typeName == "time.warp" || typeName == "time.warp~")        node = std::make_shared<TimeWarpNode> (id);
    else if (typeName == "time.retro" || typeName == "time.retro~")   node = std::make_shared<TimeRetroNode> (id);
    else if (typeName == "time.quantize" || typeName == "time.quantize~")node = std::make_shared<TimeQuantizeNode> (id);
    else if (typeName == "time.metro" || typeName == "time.metro~")   node = std::make_shared<TimeMetroNode> (id);
    else if (typeName == "time.stasis" || typeName == "time.stasis~")  node = std::make_shared<TimeStasisNode> (id);
    else if (typeName == "time.singularity" || typeName == "time.singularity~") node = std::make_shared<TimeSingularityNode> (id);
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
    else if (typeName == "+" || typeName == "+~") node = std::make_shared<AddMathNode> (id);
    else if (typeName == "*" || typeName == "*~") node = std::make_shared<MulMathNode> (id);
    else if (typeName == "-" || typeName == "-~") node = std::make_shared<SubMathNode> (id);
    else if (typeName == "/" || typeName == "/~") node = std::make_shared<DivMathNode> (id);
    else if (typeName == "%" || typeName == "%~" || typeName == "mod" || typeName == "mod~") node = std::make_shared<ModMathNode> (id);
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
    else if (typeName == "number" || typeName == "num" || typeName == "nb" || typeName == "f" || typeName == "float" || typeName == "display" || typeName == "number.display") node = std::make_shared<NumberNode> (id, false);
    else if (typeName == "i" || typeName == "int" || typeName == "integer") node = std::make_shared<NumberNode> (id, true);
    else if (typeName == "meter~" || typeName == "vu~") node = std::make_shared<VuMeterAudioNode> (id);
    else if (typeName == "number~" || typeName == "num~") node = std::make_shared<NumberAudioNode> (id);
    else if (typeName == "print" || typeName == "monitor") node = std::make_shared<PrintMonitorNode> (id);
    else if (typeName == "bang" || typeName == "b") node = std::make_shared<BangNode> (id);
    else if (typeName == "bang~" || typeName == "b~") node = std::make_shared<BangAudioNode> (id);
    else if (typeName == "counter" || typeName == "cnt") node = std::make_shared<CounterNode> (id);
    else if (typeName == "metro" || typeName == "metronome" || typeName.rfind ("metro ", 0) == 0 || typeName.rfind ("metronome ", 0) == 0)
    {
        float periodMs = 500.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1 && tokens[1].getFloatValue() > 0.0f)
            periodMs = tokens[1].getFloatValue();
        node = std::make_shared<MetroNode> (id, periodMs);
    }
    else if (typeName == "send" || typeName == "s" || typeName.rfind ("send ", 0) == 0 || typeName.rfind ("s ", 0) == 0)
    {
        std::string bus = "bus1";
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) bus = tokens[1].toStdString();
        node = std::make_shared<SendNode> (id, bus);
    }
    else if (typeName == "receive" || typeName == "r" || typeName.rfind ("receive ", 0) == 0 || typeName.rfind ("r ", 0) == 0)
    {
        std::string bus = "bus1";
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) bus = tokens[1].toStdString();
        node = std::make_shared<ReceiveNode> (id, bus);
    }
    else if (typeName == "slider" || typeName == "hslider" || typeName == "vslider") node = std::make_shared<SliderNode> (id);
    else if (typeName == "toggle" || typeName == "tgl") node = std::make_shared<ToggleNode> (id);
    else if (typeName == "delay" || typeName == "del" || typeName.rfind ("delay ", 0) == 0 || typeName.rfind ("del ", 0) == 0)
    {
        float dMs = 250.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) dMs = tokens[1].getFloatValue();
        node = std::make_shared<DelayControlNode> (id, dMs);
    }
    else if (typeName == "pipe" || typeName.rfind ("pipe ", 0) == 0)
    {
        float dMs = 250.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) dMs = tokens[1].getFloatValue();
        node = std::make_shared<PipeControlNode> (id, dMs);
    }
    else if (typeName == "spigot" || typeName == "gate") node = std::make_shared<SpigotNode> (id);
    else if (typeName == "select" || typeName == "sel" || typeName.rfind ("select ", 0) == 0 || typeName.rfind ("sel ", 0) == 0)
    {
        std::vector<float> targets = { 0.0f };
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1)
        {
            targets.clear();
            for (int t = 1; t < tokens.size(); ++t) targets.push_back (tokens[t].getFloatValue());
        }
        node = std::make_shared<SelectNode> (id, targets);
    }
    else if (typeName == "radio" || typeName == "hradio" || typeName == "vradio" || typeName.rfind ("radio ", 0) == 0)
    {
        int opts = 4;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) opts = std::max (2, tokens[1].getIntValue());
        node = std::make_shared<RadioNode> (id, opts);
    }
    else if (typeName == "==" || typeName == "==~" || typeName.rfind ("== ", 0) == 0)
    {
        float op = 0.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<BoolMathNode> (id, BoolOpType::Equals, op);
    }
    else if (typeName == "!=" || typeName == "!=~" || typeName.rfind ("!= ", 0) == 0)
    {
        float op = 0.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<BoolMathNode> (id, BoolOpType::NotEquals, op);
    }
    else if (typeName == ">" || typeName == ">~" || typeName.rfind ("> ", 0) == 0)
    {
        float op = 0.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<BoolMathNode> (id, BoolOpType::GreaterThan, op);
    }
    else if (typeName == "<" || typeName == "<~" || typeName.rfind ("< ", 0) == 0)
    {
        float op = 0.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<BoolMathNode> (id, BoolOpType::LessThan, op);
    }
    else if (typeName == ">=" || typeName == ">=~" || typeName.rfind (">= ", 0) == 0)
    {
        float op = 0.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<BoolMathNode> (id, BoolOpType::GreaterEqual, op);
    }
    else if (typeName == "<=" || typeName == "<=~" || typeName.rfind ("<= ", 0) == 0)
    {
        float op = 0.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<BoolMathNode> (id, BoolOpType::LessEqual, op);
    }
    else if (typeName == "&&" || typeName == "and") node = std::make_shared<BoolMathNode> (id, BoolOpType::And);
    else if (typeName == "||" || typeName == "or") node = std::make_shared<BoolMathNode> (id, BoolOpType::Or);
    else if (typeName == "!" || typeName == "not") node = std::make_shared<BoolMathNode> (id, BoolOpType::Not);
    else if (typeName == "audio2time~" || typeName == "a2t~") node = std::make_shared<AudioToTimeNode> (id);
    else if (typeName == "time2audio~" || typeName == "t2a~") node = std::make_shared<TimeToAudioNode> (id);
    else if (typeName == "time.+" || typeName.rfind ("time.+ ", 0) == 0)
    {
        float op = 0.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<TimeAddNode> (id, op);
    }
    else if (typeName == "time.-" || typeName.rfind ("time.- ", 0) == 0)
    {
        float op = 0.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<TimeSubNode> (id, op);
    }
    else if (typeName == "time.*" || typeName.rfind ("time.* ", 0) == 0)
    {
        float op = 1.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<TimeMulNode> (id, op);
    }
    else if (typeName == "time./" || typeName.rfind ("time./ ", 0) == 0)
    {
        float op = 1.0f;
        auto tokens = juce::StringArray::fromTokens (typeName, " ", "");
        if (tokens.size() > 1) op = tokens[1].getFloatValue();
        node = std::make_shared<TimeDivNode> (id, op);
    }
    else if (typeName == "time.expr" || typeName.rfind ("time.expr ", 0) == 0)
    {
        node = std::make_shared<TimeExprNode> (id);
        if (typeName.length() > 10) node->setParamExpression ("formula", typeName.substr (10));
    }
    else if (typeName == "time.math" || typeName == "time.math~" || typeName == "time.combine" || typeName == "time.combine~") node = std::make_shared<TimeMathNode> (id);
    else if (typeName == "time.scale" || typeName == "time.scale~") node = std::make_shared<TimeScaleNode> (id);
    else if (typeName == "time.filter" || typeName == "time.filter~") node = std::make_shared<TimeFilterNode> (id);
    else if (typeName == "time.boost" || typeName == "time.boost~" || typeName == "time.lorenz" || typeName == "time.lorenz~") node = std::make_shared<TimeLorentzBoostNode> (id);
    else if (typeName == "time.noise" || typeName == "time.noise~" || typeName == "time.rand" || typeName == "time.rand~") node = std::make_shared<TimeNoiseNode> (id);
    else if (typeName == "time.samplehold" || typeName == "time.samplehold~" || typeName == "time.sh" || typeName == "time.sh~") node = std::make_shared<TimeSampleHoldNode> (id);
    else if (typeName == "time.invert" || typeName == "time.invert~" || typeName == "time.reciprocal" || typeName == "time.reciprocal~") node = std::make_shared<TimeInvertNode> (id);
    else if (typeName == "time.logic" || typeName == "time.logic~" || typeName == "time.gate" || typeName == "time.gate~") node = std::make_shared<TimeLogicNode> (id);
    else if (typeName == "time.delay" || typeName == "time.delay~") node = std::make_shared<TimeDelayNode> (id);
    else if (typeName == "note")           node = std::make_shared<NoteGenNode> (id);
    else if (typeName == "time.future" || typeName == "time.future~" || typeName == "future" || typeName == "future~") node = std::make_shared<TimeFutureNode> (id);
    else if (typeName == "seq" || typeName == "step") node = std::make_shared<StepSequencerNode> (id);
    else if (typeName == "euclid") node = std::make_shared<EuclideanSequencerNode> (id);
    else if (typeName == "markov") node = std::make_shared<MarkovSequencerNode> (id);
    else if (typeName == "tidal" || typeName == "tidal~") node = std::make_shared<TidalPatternSequencerNode> (id);
    else if (typeName == "fbdrum~" || typeName == "drum~" || typeName == "drums~") node = std::make_shared<FutureBassDrumNode> (id);
    else if (typeName == "drumseq" || typeName == "drumstep") node = std::make_shared<DrumSequencerNode> (id);
    else if (typeName == "timeline" || typeName == "arrangement") node = std::make_shared<TimelineNode> (id);
    else                                   node = std::make_shared<OscNode> (id);


    node->addUniversalPorts();
    node->setParentGraph (this);
    node->setPosition (x, y);
    node->prepare (sampleRate, blockSize);
    nodes.push_back (node);
    notifyGraphModified();
    return id;
}

std::shared_ptr<TableNode> RelativisticNodeGraph::getTableByName (const std::string& name) const
{
    for (const auto& n : nodes)
    {
        if (n->getTypeName() == "table" && n->getLabel() == name)
            return std::dynamic_pointer_cast<TableNode> (n);
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

    notifyGraphModified();
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
    notifyGraphModified();
    return c.id;
}

void RelativisticNodeGraph::removeConnection (int connectionId)
{
    const juce::ScopedLock lock (processLock);
    connections.erase (std::remove_if (connections.begin(), connections.end(),
        [connectionId] (const PatchConnection& c) { return c.id == connectionId; }), connections.end());
    detectFeedbackLoops();
    notifyGraphModified();
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

std::shared_ptr<RelativisticNode> RelativisticNodeGraph::getNodeByLabel (const std::string& label) const
{
    for (auto& n : nodes)
    {
        if (n->getLabel() == label || n->getTypeName() == label) return n;
    }
    return nullptr;
}

void RelativisticNodeGraph::removeConnection (int srcNodeId, int srcOutletIdx, int destNodeId, int destInletIdx)
{
    int targetId = 0;
    for (const auto& conn : connections)
    {
        if (conn.sourceNodeId == srcNodeId && conn.sourceOutletIdx == srcOutletIdx &&
            conn.destNodeId == destNodeId && conn.destInletIdx == destInletIdx)
        {
            targetId = conn.id;
            break;
        }
    }
    if (targetId > 0)
    {
        removeConnection (targetId);
    }
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

void RelativisticNodeGraph::loadBasicCounterExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nBang    = addNode ("bang",    100.0f, 100.0f);
    int nCounter = addNode ("counter", 100.0f, 220.0f);
    int nNum     = addNode ("number",  100.0f, 360.0f);
    int nMsg     = addNode ("msg",     260.0f, 360.0f);

    auto counter = getNodeById (nCounter);
    if (counter)
    {
        counter->setParameter ("min", 0.0f);
        counter->setParameter ("max", 16.0f);
        counter->setParameter ("step", 1.0f);
    }

    auto msg = getNodeById (nMsg);
    if (msg) msg->setParamExpression ("value", "Value Changed!");

    addConnection (nBang,    0, nCounter, 0); // bang pulse -> counter trigger
    addConnection (nCounter, 0, nNum,     0); // counter value -> number box
    addConnection (nCounter, 0, nMsg,     0); // counter value -> msg box
}

void RelativisticNodeGraph::loadStepSequencerExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nMetro = addNode ("time.metro~", 100.0f, 100.0f);
    int nSeq   = addNode ("seq",         300.0f, 100.0f);
    int nMtof  = addNode ("mtof",        500.0f, 100.0f);
    int nNum   = addNode ("number",      500.0f, 220.0f);

    auto metro = getNodeById (nMetro);
    if (metro) metro->setParameter ("rate", 2.0f);

    auto seq = std::dynamic_pointer_cast<StepSequencerNode> (getNodeById (nSeq));
    if (seq) seq->setPatternString ("60 62 64 65 67 69 71 72");

    addConnection (nMetro, 0, nSeq,  0); // metro time -> seq timeIn
    addConnection (nSeq,   0, nMtof, 0); // seq MIDI pitch -> mtof
    addConnection (nMtof,  0, nNum,  0); // mtof Hz -> number display
}

void RelativisticNodeGraph::loadMathExpressionExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nNum1 = addNode ("number", 100.0f, 100.0f);
    int nExpr = addNode ("expr",   300.0f, 100.0f);
    int nNum2 = addNode ("number", 500.0f, 100.0f);

    auto num1 = getNodeById (nNum1);
    if (num1) num1->setParameter ("value", 10.0f);

    auto expr = getNodeById (nExpr);
    if (expr)
    {
        expr->setLabel ("expr $v1 * 2.5 + 12");
        expr->setFormulaScript ("val = $v1 * 2.5 + 12;");
    }

    addConnection (nNum1, 0, nExpr, 0); // num1 -> expr inlet 0 ($v1)
    addConnection (nExpr, 0, nNum2, 0); // expr out -> num2 display
}

void RelativisticNodeGraph::loadWirelessTappingExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nOsc  = addNode ("osc~",   100.0f, 100.0f);
    int nExpr = addNode ("expr",   350.0f, 100.0f);
    int nNum  = addNode ("number", 550.0f, 100.0f);

    auto osc = getNodeById (nOsc);
    if (osc)
    {
        osc->setLabel ("osc1");
        osc->setParameter ("frequency", 440.0f);
    }

    auto expr = getNodeById (nExpr);
    if (expr)
    {
        expr->setLabel ("expr tap('osc1.frequency') * 2");
        expr->setFormulaScript ("val = tap('osc1.frequency') * 2.0;");
    }

    addConnection (nExpr, 0, nNum, 0); // expr wireless tap out -> number box
}

void RelativisticNodeGraph::loadSimpleOscillatorExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nOsc  = addNode ("osc~",  100.0f, 100.0f);
    int nGain = addNode ("gain~", 320.0f, 100.0f);
    int nOut  = addNode ("out~",  540.0f, 100.0f);

    auto osc = getNodeById (nOsc);
    if (osc) osc->setParameter ("frequency", 440.0f);

    auto gain = getNodeById (nGain);
    if (gain) gain->setParameter ("gain", 0.5f);

    addConnection (nOsc,  0, nGain, 0); // osc~ audio -> gain~
    addConnection (nGain, 0, nOut,  0); // gain~ audio -> out~ L
    addConnection (nGain, 0, nOut,  1); // gain~ audio -> out~ R
}

void RelativisticNodeGraph::loadTableWavetableExamplePatch()
{
    loadTableExamplePatch();
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

void RelativisticNodeGraph::loadRelativisticTimeModulationExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nMetro   = addNode ("time.metro~", 80.0f, 100.0f);
    int nWarp    = addNode ("time.warp~",  80.0f, 260.0f);
    int nTMath   = addNode ("time.math~",  280.0f, 180.0f);
    int nSlider  = addNode ("slider",      480.0f, 80.0f);
    int nSeq     = addNode ("seq",         480.0f, 220.0f);
    int nMtof    = addNode ("mtof",        480.0f, 360.0f);
    int nOsc     = addNode ("osc~",        480.0f, 480.0f);
    int nFilter  = addNode ("filter~",     480.0f, 620.0f);
    int nGain    = addNode ("gain~",       480.0f, 740.0f);
    int nOut     = addNode ("out~",        480.0f, 860.0f);

    auto nodeMetro = getNodeById (nMetro);
    if (nodeMetro) nodeMetro->setParameter ("rate", 1.5f);

    auto nodeWarp = getNodeById (nWarp);
    if (nodeWarp) nodeWarp->setParameter ("dilationGamma", 1.25f);

    auto nodeTMath = getNodeById (nTMath);
    if (nodeTMath) nodeTMath->setParameter ("mode", 1.0f); // Lorentz Boost Composition!

    auto nodeSlider = getNodeById (nSlider);
    if (nodeSlider)
    {
        nodeSlider->setParameter ("min", 1.0f);
        nodeSlider->setParameter ("max", 16.0f);
        nodeSlider->setParameter ("isInteger", 1.0f);
        nodeSlider->setParameter ("value", 8.0f);
    }

    auto nodeSeq = std::dynamic_pointer_cast<StepSequencerNode> (getNodeById (nSeq));
    if (nodeSeq) nodeSeq->setPatternString ("48 52 55 59 60 64 67 71");

    auto nodeFilter = getNodeById (nFilter);
    if (nodeFilter) nodeFilter->setParameter ("cutoff", 2400.0f);

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.65f);

    auto nodeOut = getNodeById (nOut);
    if (nodeOut) nodeOut->setParameter ("volume", 0.6f);

    // Patch Connections: Time Modulating Time & Sequential Control
    addConnection (nMetro, 0, nTMath, 0);     // time.metro~ gamma1 -> time.math~ inlet 0
    addConnection (nWarp, 0, nTMath, 1);      // time.warp~ gamma2 -> time.math~ inlet 1
    addConnection (nTMath, 0, nSeq, 0);       // Lorentz-combined time gamma -> seq timeIn!
    addConnection (nTMath, 0, nOsc, 0);       // Lorentz-combined time gamma -> osc~ timeIn!
    addConnection (nSlider, 0, nSeq, 1);      // slider (integer) -> seq steps
    addConnection (nSeq, 0, nMtof, 1);        // seq note -> mtof midiIn
    addConnection (nMtof, 0, nOsc, 1);        // mtof freq -> osc~ freq
    addConnection (nOsc, 0, nFilter, 1);      // osc~ out~ -> filter~ in~
    addConnection (nFilter, 0, nGain, 1);     // filter~ out~ -> gain~ in~
    addConnection (nGain, 0, nOut, 1);        // gain~ out~ -> out~ inL~
    addConnection (nGain, 0, nOut, 2);        // gain~ out~ -> out~ inR~

    detectFeedbackLoops();
}

void RelativisticNodeGraph::loadModularSubtractiveSynthesizerExamplePatch()
{
    pushUndoState();
    clearGraph();

    int nTidal   = addNode ("tidal",   80.0f, 100.0f);
    int nMtof    = addNode ("mtof",    300.0f, 100.0f);
    int nOsc     = addNode ("osc~",    500.0f, 100.0f);
    int nAdsr    = addNode ("adsr~",   300.0f, 260.0f);
    int nFilter  = addNode ("filter~", 500.0f, 260.0f);
    int nGain    = addNode ("gain~",   500.0f, 420.0f);
    int nOut     = addNode ("out~",    500.0f, 580.0f);

    auto nodeTidal = std::dynamic_pointer_cast<TidalPatternSequencerNode> (getNodeById (nTidal));
    if (nodeTidal) nodeTidal->setPatternString ("scale 'minor' '0 [3 5] 7 [10 12]*1.5 ~ [7 5]*0.8'");

    auto nodeOsc = getNodeById (nOsc);
    if (nodeOsc)
    {
        nodeOsc->setParameter ("waveform", 1.0f); // Sawtooth wave
        nodeOsc->setParameter ("gain", 0.7f);
    }

    auto nodeAdsr = getNodeById (nAdsr);
    if (nodeAdsr)
    {
        nodeAdsr->setParameter ("attack", 0.01f);
        nodeAdsr->setParameter ("decay", 0.25f);
        nodeAdsr->setParameter ("sustain", 0.4f);
        nodeAdsr->setParameter ("release", 0.3f);
    }

    auto nodeFilter = getNodeById (nFilter);
    if (nodeFilter)
    {
        nodeFilter->setParameter ("cutoff", 1800.0f);
        nodeFilter->setParameter ("resonance", 2.5f);
    }

    auto nodeGain = getNodeById (nGain);
    if (nodeGain) nodeGain->setParameter ("gain", 0.6f);

    // 1. Tidal MIDI Note (Out 0) -> mtof (In 0) -> osc~ (In 1: Freq)
    addConnection (nTidal, 0, nMtof, 0);
    addConnection (nMtof,  0, nOsc,  1);

    // 2. Tidal Gate Trigger (Out 1: gate~) -> adsr~ (In 0: gate~)
    addConnection (nTidal, 1, nAdsr, 0);

    // 3. osc~ audio (Out 0) -> filter~ audio (In 0: in~)
    addConnection (nOsc, 0, nFilter, 0);

    // 4. adsr~ envelope (Out 0) -> filter~ cutoff mod (In 2: cutoff)
    addConnection (nAdsr, 0, nFilter, 2);

    // 5. filter~ audio (Out 0) -> gain~ audio (In 0: in~)
    addConnection (nFilter, 0, nGain, 0);

    // 6. adsr~ envelope (Out 0) -> gain~ gain mod (In 1: gain)
    addConnection (nAdsr, 0, nGain, 1);

    // 7. gain~ audio (Out 0) -> out~ (In 0: L, In 1: R)
    addConnection (nGain, 0, nOut, 0);
    addConnection (nGain, 0, nOut, 1);

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

juce::String RelativisticNodeGraph::exportPatchToJson() const
{
    auto rootObj = std::make_unique<juce::DynamicObject>();
    rootObj->setProperty ("version", "0.0.1");
    rootObj->setProperty ("audioEngineEnabled", audioEngineEnabled);

    juce::Array<juce::var> nodesArray;
    for (const auto& node : nodes)
    {
        auto nodeObj = std::make_unique<juce::DynamicObject>();
        nodeObj->setProperty ("id", node->getId());
        nodeObj->setProperty ("type", juce::String (node->getTypeName()));
        nodeObj->setProperty ("label", juce::String (node->getLabel()));
        nodeObj->setProperty ("posX", node->getX());
        nodeObj->setProperty ("posY", node->getY());
        nodeObj->setProperty ("formula", juce::String (node->getFormulaScript()));

        auto paramsObj = std::make_unique<juce::DynamicObject>();
        for (const auto& pDef : node->getParameterDefs())
        {
            juce::Identifier pKey (pDef.key);
            if (pDef.type == ParameterType::Symbol)
                paramsObj->setProperty (pKey, juce::String (pDef.expression.empty() ? pDef.stringValue : pDef.expression));
            else
                paramsObj->setProperty (pKey, pDef.value);
        }
        nodeObj->setProperty ("parameters", paramsObj.release());
        nodesArray.add (juce::var (nodeObj.release()));
    }
    rootObj->setProperty ("nodes", nodesArray);

    juce::Array<juce::var> connsArray;
    for (const auto& c : connections)
    {
        auto connObj = std::make_unique<juce::DynamicObject>();
        connObj->setProperty ("id", c.id);
        connObj->setProperty ("sourceNodeId", c.sourceNodeId);
        connObj->setProperty ("sourceOutletIdx", c.sourceOutletIdx);
        connObj->setProperty ("destNodeId", c.destNodeId);
        connObj->setProperty ("destInletIdx", c.destInletIdx);
        connsArray.add (juce::var (connObj.release()));
    }
    rootObj->setProperty ("connections", connsArray);

    return juce::JSON::toString (juce::var (rootObj.release()), false);
}

bool RelativisticNodeGraph::importPatchFromJson (const juce::String& jsonString)
{
    juce::var parsed = juce::JSON::parse (jsonString);
    if (!parsed.isObject()) return false;

    clearGraph();
    audioEngineEnabled = parsed.getProperty ("audioEngineEnabled", false);

    int maxLoadedId = 0;
    auto nodesVar = parsed.getProperty ("nodes", juce::var());
    if (nodesVar.isArray())
    {
        for (int i = 0; i < nodesVar.size(); ++i)
        {
            auto nObj = nodesVar[i];
            if (nObj.isObject())
            {
                std::string typeName = nObj.getProperty ("type", "").toString().toStdString();
                float x = nObj.getProperty ("posX", 100.0f);
                float y = nObj.getProperty ("posY", 100.0f);
                std::string label = nObj.getProperty ("label", "").toString().toStdString();

                int nodeId = addNode (typeName, x, y);
                auto n = getNodeById (nodeId);
                if (n)
                {
                    if (!label.empty()) n->setLabel (label);
                    std::string formula = nObj.getProperty ("formula", "").toString().toStdString();
                    if (!formula.empty()) n->setFormulaScript (formula);

                    auto paramsVar = nObj.getProperty ("parameters", juce::var());
                    if (paramsVar.isObject())
                    {
                        auto pObj = paramsVar.getDynamicObject();
                        for (const auto& prop : pObj->getProperties())
                        {
                            std::string key = prop.name.toString().toStdString();
                            if (prop.value.isString())
                                n->setParamExpression (key, prop.value.toString().toStdString());
                            else
                                n->setParameter (key, static_cast<float>(prop.value));
                        }
                    }
                    maxLoadedId = std::max (maxLoadedId, n->getId());
                }
            }
        }
    }

    if (maxLoadedId >= nextNodeId)
        nextNodeId = maxLoadedId + 1;

    auto connsVar = parsed.getProperty ("connections", juce::var());
    if (connsVar.isArray())
    {
        for (int i = 0; i < connsVar.size(); ++i)
        {
            auto cObj = connsVar[i];
            if (cObj.isObject())
            {
                int srcId = cObj.getProperty ("sourceNodeId", 0);
                int srcOutlet = cObj.getProperty ("sourceOutletIdx", 0);
                int dstId = cObj.getProperty ("destNodeId", 0);
                int dstInlet = cObj.getProperty ("destInletIdx", 0);
                addConnection (srcId, srcOutlet, dstId, dstInlet);
            }
        }
    }

    detectFeedbackLoops();
    return true;
}

bool RelativisticNodeGraph::saveProjectToFile (const juce::File& file)
{
    if (file.getFileExtension().equalsIgnoreCase (".patch") || file.getFileExtension().equalsIgnoreCase (".json"))
    {
        juce::String jsonStr = exportPatchToJson();
        return file.replaceWithText (jsonStr);
    }

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

    juce::String content = file.loadFileAsString();
    if (file.getFileExtension().equalsIgnoreCase (".patch") || file.getFileExtension().equalsIgnoreCase (".json") || content.trimStart().startsWith ("{"))
    {
        if (importPatchFromJson (content))
            return true;
    }

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
            in.isConnected = false;
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
                    destPort.isConnected = true;

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

std::vector<std::shared_ptr<RelativisticNode>> RelativisticNodeGraph::getTopologicallySortedNodes() const
{
    std::map<int, std::vector<int>> adj;
    std::map<int, int> inDegree;

    for (const auto& n : nodes)
    {
        inDegree[n->getId()] = 0;
    }

    for (const auto& conn : connections)
    {
        if (!conn.isFeedbackLoop)
        {
            adj[conn.sourceNodeId].push_back (conn.destNodeId);
            inDegree[conn.destNodeId]++;
        }
    }

    std::vector<int> q;
    for (const auto& n : nodes)
    {
        if (inDegree[n->getId()] == 0)
        {
            q.push_back (n->getId());
        }
    }

    std::vector<std::shared_ptr<RelativisticNode>> sorted;
    size_t head = 0;

    while (head < q.size())
    {
        int currId = q[head++];
        auto n = const_cast<RelativisticNodeGraph*>(this)->getNodeById (currId);
        if (n) sorted.push_back (n);

        for (int neighborId : adj[currId])
        {
            inDegree[neighborId]--;
            if (inDegree[neighborId] == 0)
            {
                q.push_back (neighborId);
            }
        }
    }

    if (sorted.size() < nodes.size())
    {
        std::set<int> visited;
        for (const auto& n : sorted) visited.insert (n->getId());
        for (const auto& n : nodes)
        {
            if (visited.find (n->getId()) == visited.end())
            {
                sorted.push_back (n);
            }
        }
    }

    return sorted;
}

void RelativisticNodeGraph::pushNodeOutletsToConnectedInlets (RelativisticNode* srcNode)
{
    if (!srcNode) return;
    int srcId = srcNode->getId();
    const auto& srcOutlets = srcNode->getOutlets();

    for (const auto& conn : connections)
    {
        if (conn.sourceNodeId == srcId)
        {
            auto destNode = getNodeById (conn.destNodeId);
            if (destNode)
            {
                auto& destInlets = destNode->getInlets();
                if (conn.sourceOutletIdx < static_cast<int>(srcOutlets.size()) &&
                    conn.destInletIdx < static_cast<int>(destInlets.size()))
                {
                    const auto& srcPort = srcOutlets[conn.sourceOutletIdx];
                    auto& destPort = destInlets[conn.destInletIdx];

                    destPort.isConnected = true;

                    const auto& bufferToUse = conn.isFeedbackLoop ? srcPort.previousBlockBuffer : srcPort.audioData;

                    int numCopy = std::min ({ destPort.audioData.getNumSamples(), bufferToUse.getNumSamples(), blockSize });
                    if (numCopy > 0)
                    {
                        destPort.audioData.addFrom (0, 0, bufferToUse, 0, 0, numCopy);
                        if (bufferToUse.getNumChannels() > 1 && destPort.audioData.getNumChannels() > 1)
                            destPort.audioData.addFrom (1, 0, bufferToUse, 1, 0, numCopy);
                    }

                    destPort.controlValue = srcPort.controlValue;
                    destPort.messageValue = srcPort.messageValue;
                    destPort.timeGamma = srcPort.timeGamma;

                    if (destPort.type == NodePortType::Message && !srcPort.messageValue.empty())
                    {
                        destNode->receiveMessage (srcPort.messageValue);
                    }
                }
            }
        }
    }
}

void RelativisticNodeGraph::process (juce::AudioBuffer<float>& masterOutput, int numSamples)
{
    const juce::ScopedLock lock (processLock);
    masterOutput.clear();
    blockSize = numSamples;

    // 1. Control & Time Math Propagation runs ALWAYS (even when audio is OFF)
    for (auto& node : nodes)
    {
        node->ensureBufferSize (numSamples);
        for (auto& in : node->getInlets())
        {
            in.audioData.clear();
            in.controlValue = 0.0f;
            in.timeGamma = 1.0;
            in.isConnected = false;
        }
    }

    propagateSignals();

    // 2. Process all Node Objects in Topological Execution Order
    auto sortedNodes = getTopologicallySortedNodes();
    for (auto& node : sortedNodes)
    {
        double tau = node->updateCoordinateTime (numSamples);
        double gamma = node->getEffectiveGamma();

        node->process (numSamples);

        // Feed Audio Delay Line visualizer buffer & Control Pipe telemetry
        for (auto& out : node->getOutlets())
        {
            if (out.type == NodePortType::Audio)
            {
                node->processAudioTimeDelay (out.audioData, gamma);

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

        node->processControlTimePipe (tau, gamma);

        // Immediately push newly generated outlets to downstream node inlets
        pushNodeOutletsToConnectedInlets (node.get());
    }

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

void RelativisticNodeGraph::logToConsole (const std::string& sourceLabel, const std::string& message, bool isWarning)
{
    const juce::SpinLock::ScopedLockType sl (consoleLock);
    ConsoleLogEntry entry;
    entry.timestampSec = currentCausalityHorizonSec;
    entry.sourceLabel = sourceLabel;
    entry.message = message;
    entry.isWarning = isWarning;

    consoleLogs.push_back (entry);
    if (consoleLogs.size() > 500)
    {
        consoleLogs.erase (consoleLogs.begin(), consoleLogs.begin() + 100);
    }
}

std::vector<ConsoleLogEntry> RelativisticNodeGraph::getConsoleLogs() const
{
    const juce::SpinLock::ScopedLockType sl (consoleLock);
    return consoleLogs;
}

void RelativisticNodeGraph::clearConsoleLogs()
{
    const juce::SpinLock::ScopedLockType sl (consoleLock);
    consoleLogs.clear();
}

} // namespace time_dilation
