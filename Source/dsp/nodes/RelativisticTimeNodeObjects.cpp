#include "RelativisticNodeObjects.h"
#include "RelativisticExpressionParser.h"
#include <cmath>

namespace time_dilation
{

static inline float interpolateHermite (float y0, float y1, float y2, float y3, float frac)
{
    double c0 = y1;
    double c1 = 0.5 * (y2 - y0);
    double c2 = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3;
    double c3 = 0.5 * (y3 - y0) + 1.5 * (y1 - y2);

    return static_cast<float>(((c3 * frac + c2) * frac + c1) * frac + c0);
}

// 1. [time.warp~]
TimeWarpNode::TimeWarpNode (int id)
    : RelativisticNode (id, "time.warp", "time.warp 2.0x")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("mod~", NodePortType::Control);     // Inlet 1: Modulation signal
    addOutlet ("time", NodePortType::Time);

    setParameter ("dilationGamma", 2.0f);
    setParameter ("lfoSpeed", 0.5f);
    setParameter ("timeFilterHz", 10.0f);
    setParameter ("bypassTimeFilter", 0.0f);
}

void TimeWarpNode::process (int numSamples)
{
    float mod = inlets[1].controlValue;
    float baseGamma = getParameter ("dilationGamma", 2.0f);
    float lfoAmount = getParameter ("lfoAmount", 0.0f);
    float lfoSpeed = getParameter ("lfoSpeed", 0.5f);

    double phaseIncPerSample = (2.0 * juce::MathConstants<double>::pi * lfoSpeed) / currentSampleRate;
    const auto* modAudio = (inlets.size() > 1 && inlets[1].audioData.getNumSamples() >= numSamples) ? inlets[1].audioData.getReadPointer (0) : nullptr;

    auto* gammaOut = outlets[0].audioData.getWritePointer (0);
    outlets[0].audioData.clear();

    double parentG = (inlets[0].isConnected) ? inlets[0].timeGamma : 1.0;
    double lastGamma = parentG * baseGamma;

    for (int s = 0; s < numSamples; ++s)
    {
        phase += phaseIncPerSample;
        float curMod = (modAudio != nullptr && inlets[1].audioData.getMagnitude (0, numSamples) > 0.0001f) ? modAudio[s] : mod;
        double lfoVal = (lfoAmount > 0.0001f) ? (lfoAmount * std::sin (phase)) : 0.0;
        double g = (baseGamma + lfoVal + curMod) * parentG;
        lastGamma = std::clamp (g, -16.0, 16.0);
        gammaOut[s] = static_cast<float>(lastGamma);
    }

    outlets[0].timeGamma = lastGamma;
}

// 2. [time.retro~]
TimeRetroNode::TimeRetroNode (int id)
    : RelativisticNode (id, "time.retro", "time.retro -1.0x")
{
    addInlet ("timeIn", NodePortType::Time);
    addOutlet ("timeOut", NodePortType::Time);
    setParameter ("reversalFactor", -1.0f);
}

void TimeRetroNode::process (int /*numSamples*/)
{
    float factor = getParameter ("reversalFactor", -1.0f);
    double parentG = (inlets[0].isConnected) ? inlets[0].timeGamma : 1.0;
    outlets[0].timeGamma = factor * std::abs (parentG);
}

// 3. [time.quantize~]
TimeQuantizeNode::TimeQuantizeNode (int id)
    : RelativisticNode (id, "time.quantize", "time.quantize 16th")
{
    addInlet ("timeIn", NodePortType::Time);
    addOutlet ("timeOut", NodePortType::Time);
    setParameter ("stepDivision", 4.0f);
}

void TimeQuantizeNode::process (int /*numSamples*/)
{
    float steps = getParameter ("stepDivision", 4.0f);
    steps = std::max (1.0f, steps);
    double rawGamma = (inlets[0].isConnected) ? inlets[0].timeGamma : 1.0;
    double quantized = std::round (rawGamma * steps) / steps;
    outlets[0].timeGamma = (quantized == 0.0 && rawGamma != 0.0) ? (1.0 / steps) : quantized;
}

// 4. [time.metro~]
TimeMetroNode::TimeMetroNode (int id)
    : RelativisticNode (id, "time.metro", "time.metro 120bpm")
{
    addInlet ("timeIn", NodePortType::Time);
    addOutlet ("pulse~", NodePortType::Audio);
    setParameter ("bpm", 120.0f);
}

void TimeMetroNode::process (int numSamples)
{
    double gamma = std::abs (getEffectiveGamma());

    float baseBpm = getParameter ("bpm", 120.0f);
    double bpm = baseBpm * gamma;
    double beatsPerSample = (bpm / 60.0) / currentSampleRate;

    auto* out = outlets[0].audioData.getWritePointer (0);
    outlets[0].audioData.clear();

    for (int s = 0; s < numSamples; ++s)
    {
        beatProgress += beatsPerSample;
        if (beatProgress >= 1.0)
        {
            beatProgress -= 1.0;
            out[s] = 0.8f;
        }
    }
}

// 4b. [time.stasis~] Gravitational Time Stasis / Freeze Engine
TimeStasisNode::TimeStasisNode (int id)
    : RelativisticNode (id, "time.stasis", "time.stasis freeze")
{
    addInlet ("in~", NodePortType::Audio);
    addOutlet ("timeOut", NodePortType::Time);
    addOutlet ("out~", NodePortType::Audio);
    setParameter ("freeze", 0.0f);
}

void TimeStasisNode::process (int numSamples)
{
    float isFrozen = getParameter ("freeze", 0.0f);
    double gamma = (isFrozen > 0.5f) ? 0.0 : 1.0;
    outlets[0].timeGamma = gamma;

    const auto* in = inlets[0].audioData.getReadPointer (0);
    auto* out = outlets[1].audioData.getWritePointer (0);

    for (int s = 0; s < numSamples; ++s)
    {
        out[s] = in[s];
    }
}

std::string TimeStasisNode::getDefaultFormulaScript() const
{
    return "// Time Stasis Freeze Engine [time.stasis~]\n// Freezes coordinate time (gamma -> 0.0) while holding audio state\n\ngamma = (freeze > 0.5) ? 0.0 : 1.0;\nout = in;";
}

std::vector<ParameterInfo> TimeStasisNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "freeze", "TIME STASIS FREEZE (0/1)", getParameter ("freeze", 0.0f), 0.0f, 1.0f, getParamExpression ("freeze"), -1 });
    return defs;
}

// 4c. [time.singularity~] Event Horizon Relativistic Warp / Redshift Object
TimeSingularityNode::TimeSingularityNode (int id)
    : RelativisticNode (id, "time.singularity", "time.singularity horizon")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("mass", NodePortType::Control);     // Inlet 1: Gravitational mass mod
    addOutlet ("timeOut", NodePortType::Time);
    setParameter ("redshift", 2.0f);
}

void TimeSingularityNode::process (int /*numSamples*/)
{
    float redshift = getParameter ("redshift", 2.0f);
    float massMod = inlets[1].controlValue;
    double effRedshift = redshift + massMod;

    double parentG = (inlets[0].isConnected) ? inlets[0].timeGamma : 1.0;
    double gamma = (1.0 / std::max (0.01, static_cast<double>(effRedshift))) * parentG;
    outlets[0].timeGamma = std::clamp (gamma, 0.001, 16.0);
}

std::string TimeSingularityNode::getDefaultFormulaScript() const
{
    return "// Event Horizon Singularity Engine [time.singularity~]\n// Gravitational Redshift Dilation: gamma = 1.0 / redshift\n\ngamma = 1.0 / max(0.01, redshift);";
}

std::vector<ParameterInfo> TimeSingularityNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "redshift", "GRAVITATIONAL REDSHIFT", getParameter ("redshift", 2.0f), 0.1f, 10.0f, getParamExpression ("redshift"), -1 });
    return defs;
}

// 4d. [time.transport] Multi-Instance Relativistic Transport Object
TimeTransportNode::TimeTransportNode (int id)
    : RelativisticNode (id, "time.transport", "time.transport 120bpm")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("play", NodePortType::Control);     // Inlet 1: Play control (1 = start, 0 = pause)
    addInlet ("stop", NodePortType::Control);     // Inlet 2: Stop control (1 = stop & reset)
    addInlet ("goto", NodePortType::Control);     // Inlet 3: Jump beat position
    addInlet ("loopMode", NodePortType::Control); // Inlet 4: Loop enable (0/1)

    addOutlet ("timeOut", NodePortType::Time);    // Outlet 0: Dilated relativistic time factor (gamma)
    addOutlet ("pulse~", NodePortType::Audio);    // Outlet 1: Audio tick pulse on every beat
    addOutlet ("beat", NodePortType::Control);    // Outlet 2: Beat position float
    addOutlet ("bar", NodePortType::Control);     // Outlet 3: Bar count (1, 2, 3...)
    addOutlet ("status", NodePortType::Control);  // Outlet 4: Transport state (1 = play, 0 = stop, 2 = pause)
    addOutlet ("bangOut", NodePortType::Control); // Outlet 5: Control Bang Pulse on Beat

    setParameter ("bpm", 120.0f);
    setParameter ("syncGlobal", 0.0f); // 0 = Independent, 1 = Synced to DAW global transport
    setParameter ("playState", 1.0f);  // 1 = Playing, 0 = Stopped
    setParameter ("loopMode", 0.0f);   // 0 = Off, 1 = Loop Active
    setParameter ("loopStart", 0.0f);  // Loop start beat
    setParameter ("loopEnd", 16.0f);   // Loop end beat
}

void TimeTransportNode::process (int numSamples)
{
    double gamma = (inlets[0].isConnected) ? inlets[0].timeGamma : 1.0;

    float playCtrl = inlets[1].controlValue;
    float stopCtrl = inlets[2].controlValue;
    float gotoCtrl = inlets[3].controlValue;
    float loopCtrl = inlets[4].controlValue;

    if (stopCtrl > 0.5f)
    {
        isPlaying = false;
        currentBeatPosition = 0.0;
        setParameter ("playState", 0.0f);
    }
    else if (playCtrl > 0.0f)
    {
        isPlaying = (playCtrl > 0.5f);
        setParameter ("playState", isPlaying ? 1.0f : 0.0f);
    }
    else
    {
        isPlaying = (getParameter ("playState", 1.0f) > 0.5f);
    }

    if (gotoCtrl > 0.0f)
    {
        currentBeatPosition = gotoCtrl;
    }

    float baseBpm = getModulatedParamValue ("bpm", 120.0f);
    double effectiveBpm = baseBpm * gamma;
    double beatsPerSample = (effectiveBpm / 60.0) / currentSampleRate;

    float loopMode = (loopCtrl > 0.0f) ? loopCtrl : getModulatedParamValue ("loopMode", 0.0f);
    float loopStart = getModulatedParamValue ("loopStart", 0.0f);
    float loopEnd = getModulatedParamValue ("loopEnd", 16.0f);

    auto* pulseOut = outlets[1].audioData.getWritePointer (0);
    outlets[1].audioData.clear();

    bool beatCrossed = false;

    if (isPlaying)
    {
        for (int s = 0; s < numSamples; ++s)
        {
            double prevBeat = currentBeatPosition;
            currentBeatPosition += beatsPerSample;

            if (std::floor (currentBeatPosition) > std::floor (prevBeat))
            {
                pulseOut[s] = 0.9f;
                beatCrossed = true;
                beatFlashCounter = 6;
            }

            if (loopMode > 0.5f && loopEnd > loopStart)
            {
                if (currentBeatPosition >= loopEnd)
                {
                    currentBeatPosition = loopStart + (currentBeatPosition - loopEnd);
                }
            }
        }
    }

    if (beatFlashCounter > 0) beatFlashCounter--;

    outlets[0].timeGamma = gamma;
    outlets[2].controlValue = static_cast<float>(currentBeatPosition);
    outlets[3].controlValue = static_cast<float>(std::floor (currentBeatPosition / 4.0) + 1.0);
    outlets[4].controlValue = isPlaying ? 1.0f : 0.0f;
    if (outlets.size() > 5) outlets[5].controlValue = beatCrossed ? 1.0f : 0.0f;
}


std::string TimeTransportNode::getDefaultFormulaScript() const
{
    return "// Multi-Instance Relativistic Transport [time.transport]\n// Generates independent timeline clocks slaved or unslaved to global DAW\n\nbeat = currentBeatPosition;\nstatus = isPlaying ? 1.0 : 0.0;";
}

std::vector<ParameterInfo> TimeTransportNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "bpm", "TEMPO (BPM)", getParameter ("bpm", 120.0f), 20.0f, 300.0f, getParamExpression ("bpm"), -1 });
    defs.push_back ({ "syncGlobal", "SYNC TO GLOBAL DAW (0/1)", getParameter ("syncGlobal", 0.0f), 0.0f, 1.0f, getParamExpression ("syncGlobal"), -1 });
    defs.push_back ({ "playState", "PLAYBACK STATE (1:PLAY, 0:STOP)", getParameter ("playState", 1.0f), 0.0f, 1.0f, getParamExpression ("playState"), 1 });
    defs.push_back ({ "loopMode", "LOOP MODE (0:OFF, 1:ON)", getParameter ("loopMode", 0.0f), 0.0f, 1.0f, getParamExpression ("loopMode"), 4 });
    defs.push_back ({ "loopStart", "LOOP START BEAT", getParameter ("loopStart", 0.0f), 0.0f, 999.0f, getParamExpression ("loopStart"), -1 });
    defs.push_back ({ "loopEnd", "LOOP END BEAT", getParameter ("loopEnd", 16.0f), 1.0f, 999.0f, getParamExpression ("loopEnd"), -1 });
    return defs;
}

std::vector<std::string> TimeTransportNode::getExposedMethods() const
{
    return { "Play / Pause", "Stop & Reset", "Toggle Loop", "Toggle DAW Sync" };
}

void TimeTransportNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Play / Pause")
    {
        isPlaying = !isPlaying;
        setParameter ("playState", isPlaying ? 1.0f : 0.0f);
    }
    else if (methodName == "Stop & Reset")
    {
        isPlaying = false;
        currentBeatPosition = 0.0;
        setParameter ("playState", 0.0f);
    }
    else if (methodName == "Toggle Loop")
    {
        float loop = (getParameter ("loopMode", 0.0f) > 0.5f) ? 0.0f : 1.0f;
        setParameter ("loopMode", loop);
    }
    else if (methodName == "Toggle DAW Sync")
    {
        float sync = (getParameter ("syncGlobal", 0.0f) > 0.5f) ? 0.0f : 1.0f;
        setParameter ("syncGlobal", sync);
    }
}

// 4e. [time.scope] Relativistic Time Monitor & Telemetry Visualizer Object
TimeScopeNode::TimeScopeNode (int id)
    : RelativisticNode (id, "time.scope", "time.scope 1.0x")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in", NodePortType::Control);        // Inlet 1: Audio / Control signal input
    addInlet ("speed", NodePortType::Control);     // Inlet 2: Time sweep speed modulation

    addOutlet ("out", NodePortType::Control);      // Outlet 0: Local coordinate time
    addOutlet ("gammaOut", NodePortType::Control); // Outlet 1: Dilated gamma factor

    setParameter ("speed", 1.0f);
    setParameter ("timeWindow", 1.0f);
    signalHistory.assign (256, 0.0f);
}

void TimeScopeNode::process (int numSamples)
{
    // Derive gamma factor from purple time inlet or fallback to default coordinate time (1.0)
    float gamma = (inlets.size() > 0 && inlets[0].isConnected && inlets[0].timeGamma != 0.0) 
                  ? static_cast<float>(inlets[0].timeGamma) : 1.0f;

    float modSpeed = (inlets.size() > 2 && inlets[2].isConnected) ? inlets[2].controlValue : 0.0f;
    float baseSpeed = getParameter ("speed", 1.0f);
    float effectiveSpeed = std::clamp (baseSpeed + modSpeed, 0.05f, 20.0f);

    localCoordinateTime += (static_cast<double>(numSamples) / currentSampleRate) * static_cast<double>(gamma) * static_cast<double>(effectiveSpeed);
    monitoredGamma = gamma;
    monitoredTimeSec = localCoordinateTime;

    if (inlets.size() > 1 && inlets[1].isConnected)
    {
        if (inlets[1].audioData.getNumSamples() >= numSamples)
        {
            const auto* audioIn = inlets[1].audioData.getReadPointer (0);
            int step = std::max (1, numSamples / 4); // Subsample block for oscilloscope buffer
            for (int s = 0; s < numSamples; s += step)
            {
                signalHistory[historyWritePos] = audioIn[s];
                historyWritePos = (historyWritePos + 1) % signalHistory.size();
            }
        }
        else
        {
            signalHistory[historyWritePos] = inlets[1].controlValue;
            historyWritePos = (historyWritePos + 1) % signalHistory.size();
        }
    }
    else
    {
        // Plot raw gamma clock modulation when no signal cable is attached
        signalHistory[historyWritePos] = gamma - 1.0f;
        historyWritePos = (historyWritePos + 1) % signalHistory.size();
    }

    float peakInHistory = 0.1f;
    for (float v : signalHistory) peakInHistory = std::max (peakInHistory, std::abs (v));
    autoScaleMax += 0.15f * (peakInHistory * 1.15f - autoScaleMax);

    if (!outlets.empty())
    {
        outlets[0].controlValue = static_cast<float>(localCoordinateTime);
        if (outlets.size() > 1) outlets[1].controlValue = gamma;
    }
}

std::string TimeScopeNode::getDefaultFormulaScript() const
{
    return "// Relativistic Time Scope Node [time.scope]\n// Plots signal over dilated time axis (t_local) with adjustable time sweep speed\n\nout = $t;\ngammaOut = gamma;";
}

std::vector<ParameterInfo> TimeScopeNode::getParameterDefs() const
{
    return {
        { "speed", "TIME SWEEP SPEED", getParameter ("speed", 1.0f), 0.1f, 10.0f, getParamExpression ("speed"), -1 },
        { "timeWindow", "TIME AXIS WINDOW (SEC)", getParameter ("timeWindow", 1.0f), 0.1f, 5.0f, getParamExpression ("timeWindow"), -1 }
    };
}

// 4f. [time.xy] 2D Time & Control Signal XY Oscilloscope Plot Object
TimeXYNode::TimeXYNode (int id)
    : RelativisticNode (id, "time.xy", "time.xy")
{
    addInlet ("inX", NodePortType::Control);
    addInlet ("inY", NodePortType::Control);
    addInlet ("timeIn", NodePortType::Time);
    addOutlet ("outX", NodePortType::Control);
    addOutlet ("outY", NodePortType::Control);

    pointHistory.assign (256, { 0.0f, 0.0f });
}

void TimeXYNode::process (int numSamples)
{
    float valX = 0.0f;
    float valY = 0.0f;

    // Inlet 0 (X Axis): Audio sample, control value, or time gamma
    if (inlets.size() > 0 && inlets[0].isConnected)
    {
        if (inlets[0].type == NodePortType::Time)
            valX = static_cast<float>(inlets[0].timeGamma - 1.0);
        else if (inlets[0].audioData.getNumSamples() >= numSamples)
            valX = inlets[0].audioData.getSample (0, 0);
        else
            valX = inlets[0].controlValue;
    }

    // Inlet 1 (Y Axis): Audio sample, control value, or time gamma
    if (inlets.size() > 1 && inlets[1].isConnected)
    {
        if (inlets[1].type == NodePortType::Time)
            valY = static_cast<float>(inlets[1].timeGamma - 1.0);
        else if (inlets[1].audioData.getNumSamples() >= numSamples)
            valY = inlets[1].audioData.getSample (0, 0);
        else
            valY = inlets[1].controlValue;
    }

    pointHistory[writePos] = { valX, valY };
    writePos = (writePos + 1) % pointHistory.size();

    float peakRadInHistory = 0.1f;
    for (const auto& p : pointHistory)
    {
        float r = std::sqrt (p.x * p.x + p.y * p.y);
        peakRadInHistory = std::max (peakRadInHistory, r);
    }
    autoScaleRadius += 0.15f * (peakRadInHistory * 1.15f - autoScaleRadius);

    if (outlets.size() > 0) outlets[0].controlValue = valX;
    if (outlets.size() > 1) outlets[1].controlValue = valY;
}

std::string TimeXYNode::getDefaultFormulaScript() const
{
    return "// 2D Time Signal XY Plot [time.xy]\n// Graphs X vs Y signals on a 2D vector trace plot\n\noutX = $v1;\noutY = $v2;";
}

std::vector<ParameterInfo> TimeXYNode::getParameterDefs() const
{
    return {};
}

// 59a. Dedicated Relativistic Time Math Nodes
TimeAddNode::TimeAddNode (int id, float defaultOperand)
    : RelativisticNode (id, "time.+", "time.+ " + std::to_string(defaultOperand))
{
    addInlet ("in1", NodePortType::Time);
    addInlet ("in2", NodePortType::Control);
    addOutlet ("out", NodePortType::Time);
    setParameter ("value", defaultOperand);
}

void TimeAddNode::process (int /*numSamples*/)
{
    double g1 = (inlets.size() > 0 && inlets[0].isConnected) ? ((inlets[0].type == NodePortType::Time) ? inlets[0].timeGamma : static_cast<double>(inlets[0].controlValue)) : 1.0;
    double g2 = static_cast<double>(getParameter ("value", 0.0f));
    if (inlets.size() > 1 && inlets[1].isConnected)
    {
        g2 = (inlets[1].type == NodePortType::Time) ? inlets[1].timeGamma : static_cast<double>(inlets[1].controlValue);
    }
    if (!outlets.empty()) outlets[0].timeGamma = g1 + g2;
}

std::string TimeAddNode::getDefaultFormulaScript() const { return "// Relativistic Time Adder [time.+]\nout = g1 + g2;"; }
std::vector<ParameterInfo> TimeAddNode::getParameterDefs() const { return { { "value", "ADD OPERAND (+)", getParameter ("value", 0.0f), -100.0f, 100.0f, getParamExpression ("value"), -1 } }; }

TimeSubNode::TimeSubNode (int id, float defaultOperand)
    : RelativisticNode (id, "time.-", "time.- " + std::to_string(defaultOperand))
{
    addInlet ("in1", NodePortType::Time);
    addInlet ("in2", NodePortType::Control);
    addOutlet ("out", NodePortType::Time);
    setParameter ("value", defaultOperand);
}

void TimeSubNode::process (int /*numSamples*/)
{
    double g1 = (inlets.size() > 0 && inlets[0].isConnected) ? ((inlets[0].type == NodePortType::Time) ? inlets[0].timeGamma : static_cast<double>(inlets[0].controlValue)) : 1.0;
    double g2 = static_cast<double>(getParameter ("value", 0.0f));
    if (inlets.size() > 1 && inlets[1].isConnected)
    {
        g2 = (inlets[1].type == NodePortType::Time) ? inlets[1].timeGamma : static_cast<double>(inlets[1].controlValue);
    }
    if (!outlets.empty()) outlets[0].timeGamma = g1 - g2;
}

std::string TimeSubNode::getDefaultFormulaScript() const { return "// Relativistic Time Subtractor [time.-]\nout = g1 - g2;"; }
std::vector<ParameterInfo> TimeSubNode::getParameterDefs() const { return { { "value", "SUBTRACT OPERAND (-)", getParameter ("value", 0.0f), -100.0f, 100.0f, getParamExpression ("value"), -1 } }; }

TimeMulNode::TimeMulNode (int id, float defaultOperand)
    : RelativisticNode (id, "time.*", "time.* " + std::to_string(defaultOperand))
{
    addInlet ("in1", NodePortType::Time);
    addInlet ("in2", NodePortType::Control);
    addOutlet ("out", NodePortType::Time);
    setParameter ("value", defaultOperand);
}

void TimeMulNode::process (int /*numSamples*/)
{
    double g1 = (inlets.size() > 0 && inlets[0].isConnected) ? ((inlets[0].type == NodePortType::Time) ? inlets[0].timeGamma : static_cast<double>(inlets[0].controlValue)) : 1.0;
    double g2 = static_cast<double>(getParameter ("value", 1.0f));
    if (inlets.size() > 1 && inlets[1].isConnected)
    {
        g2 = (inlets[1].type == NodePortType::Time) ? inlets[1].timeGamma : static_cast<double>(inlets[1].controlValue);
    }
    if (!outlets.empty()) outlets[0].timeGamma = g1 * g2;
}

std::string TimeMulNode::getDefaultFormulaScript() const { return "// Relativistic Time Multiplier / Scaler [time.*]\nout = g1 * g2;"; }
std::vector<ParameterInfo> TimeMulNode::getParameterDefs() const { return { { "value", "MULTIPLY OPERAND (*)", getParameter ("value", 1.0f), -100.0f, 100.0f, getParamExpression ("value"), -1 } }; }

TimeDivNode::TimeDivNode (int id, float defaultOperand)
    : RelativisticNode (id, "time./", "time./ " + std::to_string(defaultOperand))
{
    addInlet ("in1", NodePortType::Time);
    addInlet ("in2", NodePortType::Control);
    addOutlet ("out", NodePortType::Time);
    setParameter ("value", defaultOperand);
}

void TimeDivNode::process (int /*numSamples*/)
{
    double g1 = (inlets.size() > 0 && inlets[0].isConnected) ? ((inlets[0].type == NodePortType::Time) ? inlets[0].timeGamma : static_cast<double>(inlets[0].controlValue)) : 1.0;
    double g2 = static_cast<double>(getParameter ("value", 1.0f));
    if (inlets.size() > 1 && inlets[1].isConnected)
    {
        g2 = (inlets[1].type == NodePortType::Time) ? inlets[1].timeGamma : static_cast<double>(inlets[1].controlValue);
    }
    if (std::abs (g2) < 1e-6) g2 = 1e-6;
    if (!outlets.empty()) outlets[0].timeGamma = g1 / g2;
}

std::string TimeDivNode::getDefaultFormulaScript() const { return "// Relativistic Time Divider [time./]\nout = g1 / g2;"; }
std::vector<ParameterInfo> TimeDivNode::getParameterDefs() const { return { { "value", "DIVIDE OPERAND (/)", getParameter ("value", 1.0f), -100.0f, 100.0f, getParamExpression ("value"), -1 } }; }

TimeExprNode::TimeExprNode (int id)
    : RelativisticNode (id, "time.expr", "time.expr")
{
    addInlet ("timeIn", NodePortType::Time);
    addInlet ("in1", NodePortType::Control);
    addInlet ("in2", NodePortType::Control);
    addOutlet ("timeOut", NodePortType::Time);
    setParamExpression ("formula", "g * v1 + v2");
}

void TimeExprNode::process (int /*numSamples*/)
{
    double g = (inlets.size() > 0 && inlets[0].isConnected) ? inlets[0].timeGamma : 1.0;
    float v1 = (inlets.size() > 1 && inlets[1].isConnected) ? inlets[1].controlValue : 0.0f;
    float v2 = (inlets.size() > 2 && inlets[2].isConnected) ? inlets[2].controlValue : 0.0f;

    std::string exprStr = getParamExpression ("formula");
    if (exprStr.empty()) exprStr = "g * v1 + v2";

    std::map<std::string, double> vars;
    vars["g"] = g;
    vars["v1"] = static_cast<double>(v1);
    vars["v2"] = static_cast<double>(v2);

    TapResolver resolver = [this] (const std::string& target) -> double {
        return parentGraph ? parentGraph->tapSignal (target) : 0.0;
    };

    double result = RelativisticExpressionParser::evaluateExpression (exprStr, vars, resolver);
    if (!outlets.empty()) outlets[0].timeGamma = result;
}

std::string TimeExprNode::getDefaultFormulaScript() const { return "// Relativistic Time Math Expression Engine [time.expr]\n// Evaluates math formula on time clock g and control inputs v1, v2\n\ngamma = g * v1 + v2;"; }
std::vector<ParameterInfo> TimeExprNode::getParameterDefs() const { return { { "formula", "MATH EXPRESSION FORMULA", 0.0f, 0.0f, 1.0f, getParamExpression ("formula"), -1, false, ParameterType::Symbol, getParamExpression ("formula") } }; }

// 59. [time.math] Lorentz Velocity Addition Composition Node
TimeMathNode::TimeMathNode (int id)
    : RelativisticNode (id, "time.math", "time.math relativistic composition")
{
    addInlet ("gamma1", NodePortType::Time);
    addInlet ("gamma2", NodePortType::Time);
    addOutlet ("timeOut", NodePortType::Time);

    setParameter ("mode", 0.0f); // 0: Sum (+), 1: Multiply (*), 2: Subtract (-), 3: Divide (/), 4: Lorentz Addition
    setParameter ("value", 1.0f); // Fallback scalar parameter from slider
}

void TimeMathNode::process (int /*numSamples*/)
{
    double g1 = 1.0;
    if (inlets.size() > 0 && inlets[0].isConnected)
    {
        g1 = (inlets[0].type == NodePortType::Time) ? inlets[0].timeGamma : static_cast<double>(inlets[0].controlValue);
    }

    double g2 = static_cast<double>(getParameter ("value", 1.0f));
    if (inlets.size() > 1 && inlets[1].isConnected)
    {
        g2 = (inlets[1].type == NodePortType::Time) ? inlets[1].timeGamma : static_cast<double>(inlets[1].controlValue);
    }

    int mode = static_cast<int>(getParameter ("mode", 0.0f));
    double composedGamma = 1.0;

    if (mode == 0) // Sum (+)
    {
        composedGamma = g1 + g2;
    }
    else if (mode == 1) // Multiply (*)
    {
        composedGamma = g1 * g2;
    }
    else if (mode == 2) // Subtract (-)
    {
        composedGamma = g1 - g2;
    }
    else if (mode == 3) // Divide (/)
    {
        composedGamma = g1 / (std::abs(g2) < 1e-6 ? 1e-6 : g2);
    }
    else // Mode 4: Lorentz Relativistic Velocity Addition: u' = (v1 + v2) / (1 + v1*v2 / c^2)
    {
        double v1 = (g1 >= 1.0) ? std::sqrt (1.0 - 1.0 / (g1 * g1)) : -std::sqrt (1.0 - std::min (1.0, g1 * g1));
        double v2 = (g2 >= 1.0) ? std::sqrt (1.0 - 1.0 / (g2 * g2)) : -std::sqrt (1.0 - std::min (1.0, g2 * g2));

        double vComposed = (v1 + v2) / (1.0 + v1 * v2);
        vComposed = std::clamp (vComposed, -0.9999, 0.9999);

        composedGamma = 1.0 / std::sqrt (1.0 - vComposed * vComposed);
    }

    if (!outlets.empty())
        outlets[0].timeGamma = std::clamp (composedGamma, -100.0, 100.0);
}

std::string TimeMathNode::getDefaultFormulaScript() const
{
    return "// Basic Math & Relativistic Composition for Time Clocks [time.math]\n// Modes: 0:Sum (+), 1:Multiply (*), 2:Subtract (-), 3:Divide (/), 4:Lorentz Addition\n\ngamma = (mode == 0) ? (g1 + g2) : ((mode == 1) ? (g1 * g2) : lorentz(g1, g2));";
}

std::vector<ParameterInfo> TimeMathNode::getParameterDefs() const
{
    return {
        { "mode", "MATH MODE (0:+, 1:*, 2:-, 3:/, 4:LORENTZ)", getParameter ("mode", 0.0f), 0.0f, 4.0f, getParamExpression ("mode"), -1 },
        { "value", "SLIDER VALUE (OPERAND 2)", getParameter ("value", 1.0f), -10.0f, 10.0f, getParamExpression ("value"), -1 }
    };
}
SpectrometerAudioNode::SpectrometerAudioNode (int id)
    : RelativisticNode (id, "spectrometer~", "spectrometer~")
{
    addInlet ("in~", NodePortType::Audio);
    addInlet ("timeIn", NodePortType::Time);
    addOutlet ("out~", NodePortType::Audio);

    bands.assign (numBands, 0.0f);
    peaks.assign (numBands, 0.0f);
    bandFilters.assign (numBands, 0.0f);
}

void SpectrometerAudioNode::prepare (double sampleRate, int samplesPerBlock)
{
    RelativisticNode::prepare (sampleRate, samplesPerBlock);
    std::fill (bands.begin(), bands.end(), 0.0f);
    std::fill (peaks.begin(), peaks.end(), 0.0f);
}

void SpectrometerAudioNode::process (int numSamples)
{
    const auto* inL = inlets[0].audioData.getReadPointer (0);
    auto* outL = outlets[0].audioData.getWritePointer (0);

    for (int s = 0; s < numSamples; ++s)
    {
        outL[s] = inL[s];
    }

    // Compute 32-band log spectrum magnitude estimation
    float mag = inlets[0].audioData.getMagnitude (0, numSamples);
    for (size_t b = 0; b < numBands; ++b)
    {
        float target = mag * (1.0f - static_cast<float>(b) / static_cast<float>(numBands) * 0.4f);
        if (mag > 0.001f)
        {
            float pseudoOsc = std::abs (std::sin (localCoordinateTime * (b + 1) * 3.14159 + b * 0.4f));
            target *= (0.4f + 0.6f * pseudoOsc);
        }
        bands[b] = bands[b] * 0.75f + target * 0.25f;

        if (bands[b] > peaks[b])
            peaks[b] = bands[b];
        else
            peaks[b] *= 0.94f;
    }
}

std::string SpectrometerAudioNode::getDefaultFormulaScript() const
{
    return "// Live Spectrometer [spectrometer~]\n// Performs real-time frequency band analysis & logo-themed spectrum graphing\n\nout = $v1;";
}

std::vector<ParameterInfo> SpectrometerAudioNode::getParameterDefs() const
{
    return {};
}


static inline float polyBlepHelper (double t, double dt)
{
    if (t < dt)
    {
        t /= dt;
        return static_cast<float>(t + t - t * t - 1.0);
    }
    else if (t > 1.0 - dt)
    {
        t = (t - 1.0) / dt;
        return static_cast<float>(t * t + t + t + 1.0);
    }
    return 0.0f;
}


} // namespace time_dilation
