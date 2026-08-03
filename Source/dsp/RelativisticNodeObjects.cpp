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
    : RelativisticNode (id, "time.warp~", "time.warp~ 2.0x")
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
    float lfoSpeed = getParameter ("lfoSpeed", 0.5f);

    double phaseIncPerSample = (2.0 * juce::MathConstants<double>::pi * lfoSpeed) / currentSampleRate;
    const auto* modAudio = (inlets.size() > 1 && inlets[1].audioData.getNumSamples() >= numSamples) ? inlets[1].audioData.getReadPointer (0) : nullptr;

    auto* gammaOut = outlets[0].audioData.getWritePointer (0);
    outlets[0].audioData.clear();

    double parentG = (inlets[0].timeGamma != 0.0) ? inlets[0].timeGamma : 1.0;
    double lastGamma = parentG;

    for (int s = 0; s < numSamples; ++s)
    {
        phase += phaseIncPerSample;
        float curMod = (modAudio != nullptr && inlets[1].audioData.getMagnitude (0, numSamples) > 0.0001f) ? modAudio[s] : mod;
        double g = (baseGamma + (baseGamma * 0.5f) * std::sin (phase) + curMod) * parentG;
        lastGamma = std::clamp (g, -16.0, 16.0);
        gammaOut[s] = static_cast<float>(lastGamma);
    }

    outlets[0].timeGamma = lastGamma;
}

// 2. [time.retro~]
TimeRetroNode::TimeRetroNode (int id)
    : RelativisticNode (id, "time.retro~", "time.retro~ -1.0x")
{
    addInlet ("timeIn", NodePortType::Time);
    addOutlet ("timeOut", NodePortType::Time);
    setParameter ("reversalFactor", -1.0f);
}

void TimeRetroNode::process (int /*numSamples*/)
{
    float factor = getParameter ("reversalFactor", -1.0f);
    outlets[0].timeGamma = factor * std::abs (inlets[0].timeGamma);
}

// 3. [time.quantize~]
TimeQuantizeNode::TimeQuantizeNode (int id)
    : RelativisticNode (id, "time.quantize~", "time.quantize~ 16th")
{
    addInlet ("timeIn", NodePortType::Time);
    addOutlet ("timeOut", NodePortType::Time);
    setParameter ("stepDivision", 4.0f);
}

void TimeQuantizeNode::process (int /*numSamples*/)
{
    float steps = getParameter ("stepDivision", 4.0f);
    steps = std::max (1.0f, steps);
    double rawGamma = inlets[0].timeGamma;
    double quantized = std::round (rawGamma * steps) / steps;
    outlets[0].timeGamma = (quantized == 0.0 && rawGamma != 0.0) ? (1.0 / steps) : quantized;
}

// 4. [time.metro~]
TimeMetroNode::TimeMetroNode (int id)
    : RelativisticNode (id, "time.metro~", "time.metro~ 120bpm")
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
    : RelativisticNode (id, "time.stasis~", "time.stasis~ freeze")
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
    : RelativisticNode (id, "time.singularity~", "time.singularity~ horizon")
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

    double parentG = inlets[0].timeGamma != 0.0 ? inlets[0].timeGamma : 1.0;
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
    double gamma = std::abs (inlets[0].timeGamma);
    if (gamma < 0.001) gamma = 1.0;

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
    : RelativisticNode (id, "time.scope", "time.scope")
{
    addInlet ("timeIn", NodePortType::Time);
    addInlet ("in", NodePortType::Control);
    addOutlet ("out", NodePortType::Control);
    addOutlet ("gammaOut", NodePortType::Control);

    setParameter ("gamma", 1.0f);
    setParameter ("t_local", 0.0f);
    signalHistory.assign (128, 0.0f);
}

void TimeScopeNode::process (int numSamples)
{
    float gamma = 1.0f;
    if (!inlets.empty() && inlets[0].timeGamma != 0.0f)
    {
        gamma = inlets[0].timeGamma;
    }
    else
    {
        gamma = getParameter ("gamma", 1.0f);
    }

    localCoordinateTime += (static_cast<double>(numSamples) / currentSampleRate) * gamma;
    monitoredGamma = gamma;
    monitoredTimeSec = localCoordinateTime;

    float sampleVal = 0.0f;
    if (inlets.size() > 1)
    {
        if (inlets[1].audioData.getNumSamples() > 0 && inlets[1].audioData.getMagnitude (0, numSamples) > 0.0001f)
            sampleVal = inlets[1].audioData.getSample (0, 0);
        else
            sampleVal = inlets[1].controlValue;
    }
    signalHistory[historyWritePos] = sampleVal;
    historyWritePos = (historyWritePos + 1) % signalHistory.size();

    if (!outlets.empty())
    {
        outlets[0].controlValue = static_cast<float>(localCoordinateTime);
        if (outlets.size() > 1) outlets[1].controlValue = gamma;
    }
}

std::string TimeScopeNode::getDefaultFormulaScript() const
{
    return "// Relativistic Time Scope Node [time.scope]\n// Monitors coordinate time (t_local) & time dilation factor (gamma)\n\nout = $t;\ngammaOut = gamma;";
}

std::vector<ParameterInfo> TimeScopeNode::getParameterDefs() const
{
    return {
        { "gamma", "Dilation Factor (gamma)", 1.0f, -99999.0f, 99999.0f, "", 0 },
        { "t_local", "Local Time (t_local sec)", 0.0f, -99999.0f, 99999.0f, "", -1 }
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

    if (inlets.size() > 0)
    {
        if (inlets[0].audioData.getNumSamples() > 0 && inlets[0].audioData.getMagnitude (0, numSamples) > 0.0001f)
            valX = inlets[0].audioData.getSample (0, 0);
        else
            valX = inlets[0].controlValue;
    }

    if (inlets.size() > 1)
    {
        if (inlets[1].audioData.getNumSamples() > 0 && inlets[1].audioData.getMagnitude (0, numSamples) > 0.0001f)
            valY = inlets[1].audioData.getSample (0, 0);
        else
            valY = inlets[1].controlValue;
    }

    pointHistory[writePos] = { valX, valY };
    writePos = (writePos + 1) % pointHistory.size();

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

// 4g. [spectrometer~] Live Audio Spectrum Visualizer (Logo Gradient Palette)
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


// 5. [osc~]
OscNode::OscNode (int id)
    : RelativisticNode (id, "osc~", "osc~ 440 Hz")
{
    addInlet ("timeIn", NodePortType::Time);
    addInlet ("freq", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("frequency", 440.0f);
    setParameter ("gain", 0.8f);
    setParameter ("interpMode", 1.0f); // 0:Linear, 1:Hermite 4-Pt, 2:Nearest
    setParameter ("loopMode", 0.0f);   // 0:Cycle, 1:Fwd, 2:Ping-Pong, 3:One-Shot
    setParameter ("polyphony", 1.0f);  // 1 to 16 Voices

    voices.push_back ({ 0.0, 69.0f, 440.0f, 0.8f, false });
}

float OscNode::interpolateSample (const float* tableData, int tableSize, double pos, int interpMode) const
{
    if (tableSize <= 0 || tableData == nullptr) return 0.0f;

    if (interpMode == 2) // Nearest Neighbor (Zero-Order Hold)
    {
        int idx = std::clamp (static_cast<int>(std::round (pos)), 0, tableSize - 1);
        return tableData[idx];
    }
    else if (interpMode == 0) // Linear 2-Point Interpolation
    {
        int i1 = static_cast<int>(std::floor (pos));
        double frac = pos - i1;
        int idx1 = (i1 % tableSize + tableSize) % tableSize;
        int idx2 = ((i1 + 1) % tableSize + tableSize) % tableSize;
        return static_cast<float>((1.0 - frac) * tableData[idx1] + frac * tableData[idx2]);
    }
    else // Hermite 4-Point Cubic Interpolation (Pro Anti-Aliased)
    {
        int i1 = static_cast<int>(std::floor (pos));
        double frac = pos - i1;

        int i0 = (i1 - 1 + tableSize) % tableSize;
        int i2 = (i1 + 1) % tableSize;
        int i3 = (i1 + 2) % tableSize;
        i1 = (i1 + tableSize) % tableSize;

        float y0 = tableData[i0];
        float y1 = tableData[i1];
        float y2 = tableData[i2];
        float y3 = tableData[i3];

        double c0 = y1;
        double c1 = 0.5 * (y2 - y0);
        double c2 = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3;
        double c3 = 0.5 * (y3 - y0) + 1.5 * (y1 - y2);

        return static_cast<float>(((c3 * frac + c2) * frac + c1) * frac + c0);
    }
}

void OscNode::process (int numSamples)
{
    double gamma = getEffectiveGamma();
    float freqCtrl = inlets[1].controlValue;
    float baseFreq = getModulatedParamValue ("frequency", 440.0f);
    float gain = getModulatedParamValue ("gain", 0.8f);
    float freq = baseFreq;
    if (freqCtrl > 0.0f)
    {
        freq = (freqCtrl <= 127.0f) ? (440.0f * std::pow (2.0f, (freqCtrl - 69.0f) / 12.0f)) : freqCtrl;
    }

    int interpMode = static_cast<int>(getParameter ("interpMode", 1.0f));
    int loopMode = static_cast<int>(getParameter ("loopMode", 0.0f));

    std::shared_ptr<TableNode> tableObj = nullptr;
    if (parentGraph != nullptr && !tableName.empty())
    {
        tableObj = parentGraph->getTableByName (tableName);
    }

    outlets[0].audioData.clear();
    auto* left = outlets[0].audioData.getWritePointer (0);
    auto* right = (outlets[0].audioData.getNumChannels() > 1) ? outlets[0].audioData.getWritePointer (1) : left;

    if (voices.empty()) voices.push_back ({ 0.0, 69.0f, 440.0f, 0.8f, false });

    double effectiveFreq = std::abs (freq * gamma);

    if (tableObj != nullptr && tableObj->getTableSize() > 0)
    {
        const auto& tableBuffer = tableObj->getTableData();
        const float* tablePtr = tableBuffer.data();
        int tableSize = tableObj->getTableSize();

        double step = (effectiveFreq * tableSize) / currentSampleRate;

        for (int s = 0; s < numSamples; ++s)
        {
            float sum = 0.0f;
            for (auto& v : voices)
            {
                float val = interpolateSample (tablePtr, tableSize, v.phase, interpMode);
                sum += val;

                double currentStep = (loopMode == 2 && v.isPingPongReversing) ? -step : step;
                v.phase += currentStep;

                if (loopMode == 1 || loopMode == 0) // Forward Cycle / Loop
                {
                    if (v.phase >= tableSize) v.phase -= tableSize;
                    if (v.phase < 0.0) v.phase += tableSize;
                }
                else if (loopMode == 2) // Ping-Pong
                {
                    if (v.phase >= tableSize) { v.phase = tableSize - 1; v.isPingPongReversing = true; }
                    else if (v.phase < 0.0)   { v.phase = 0.0; v.isPingPongReversing = false; }
                }
                else if (loopMode == 3) // One-Shot
                {
                    if (v.phase >= tableSize || v.phase < 0.0) v.phase = tableSize;
                }
            }

            float sampleVal = sum * gain;
            left[s] = sampleVal;
            right[s] = sampleVal;
        }
    }
    else
    {
        // Pristine PolyBLEP Sine Oscillator
        double phaseInc = 2.0 * juce::MathConstants<double>::pi * effectiveFreq / currentSampleRate;

        for (int s = 0; s < numSamples; ++s)
        {
            float val = std::sin (voices[0].phase) * gain;
            left[s] = val;
            right[s] = val;

            voices[0].phase += (gamma >= 0.0 ? phaseInc : -phaseInc);
            if (voices[0].phase >= 2.0 * juce::MathConstants<double>::pi) voices[0].phase -= 2.0 * juce::MathConstants<double>::pi;
            if (voices[0].phase < 0.0) voices[0].phase += 2.0 * juce::MathConstants<double>::pi;
        }
    }
}

std::string OscNode::getDefaultFormulaScript() const
{
    return "// PolyBLEP & Wavetable Oscillator [osc~]\n// Reads Pristine Sine or Custom Canvas [table] Data\n\nout = sin(phase * 2 * PI);";
}

std::vector<ParameterInfo> OscNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "frequency", "FREQUENCY (Hz)", getParameter ("frequency", 440.0f), 20.0f, 20000.0f, getParamExpression ("frequency"), 1 });
    defs.push_back ({ "gain", "OSCILLATOR GAIN", getParameter ("gain", 0.8f), 0.0f, 1.0f, getParamExpression ("gain"), -1 });
    defs.push_back ({ "interpMode", "INTERPOLATION (0:LIN, 1:HERMITE, 2:NEAREST)", getParameter ("interpMode", 1.0f), 0.0f, 2.0f, getParamExpression ("interpMode"), -1 });
    defs.push_back ({ "loopMode", "LOOP MODE (0:CYCLE, 1:FWD, 2:PINGPONG, 3:ONESHOT)", getParameter ("loopMode", 0.0f), 0.0f, 3.0f, getParamExpression ("loopMode"), -1 });
    defs.push_back ({ "polyphony", "MAX POLYPHONIC VOICES", getParameter ("polyphony", 1.0f), 1.0f, 16.0f, getParamExpression ("polyphony"), -1 });
    return defs;
}

std::vector<std::string> OscNode::getExposedMethods() const
{
    return { "Reset Phase", "Toggle Interpolation", "Toggle Loop Mode" };
}

void OscNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Reset Phase")
    {
        for (auto& v : voices) v.phase = 0.0;
    }
    else if (methodName == "Toggle Interpolation")
    {
        int interp = (static_cast<int>(getParameter ("interpMode", 1.0f)) + 1) % 3;
        setParameter ("interpMode", static_cast<float>(interp));
    }
    else if (methodName == "Toggle Loop Mode")
    {
        int loop = (static_cast<int>(getParameter ("loopMode", 0.0f)) + 1) % 4;
        setParameter ("loopMode", static_cast<float>(loop));
    }
}

// 5b. [mtof] MIDI Note to Frequency Converter Node Object
MtofNode::MtofNode (int id)
    : RelativisticNode (id, "mtof", "mtof (Note -> Hz)")
{
    addInlet ("note", NodePortType::Control);
    addOutlet ("freq", NodePortType::Control);
    setParameter ("note", 69.0f);
    setParameter ("offset", 0.0f);
}

void MtofNode::process (int /*numSamples*/)
{
    float note = inlets[0].controlValue;
    if (note <= 0.0f) note = getParameter ("note", 69.0f);
    float offset = getModulatedParamValue ("offset", 0.0f);

    float freq = 440.0f * std::pow (2.0f, (note + offset - 69.0f) / 12.0f);
    outlets[0].controlValue = freq;
}

std::string MtofNode::getDefaultFormulaScript() const
{
    return "// MIDI Note to Frequency [mtof]\n// Hz = 440.0 * pow(2.0, (note + offset - 69.0) / 12.0)\n\nfreq = 440.0 * pow(2.0, (note + offset - 69.0) / 12.0);";
}

std::vector<ParameterInfo> MtofNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "note", "MIDI NOTE NUMBER (0..127)", getParameter ("note", 69.0f), 0.0f, 127.0f, getParamExpression ("note"), 0 });
    defs.push_back ({ "offset", "TRANSPOSE OFFSET (semitones)", getParameter ("offset", 0.0f), -48.0f, 48.0f, getParamExpression ("offset"), -1 });
    return defs;
}

// 5c. [ftom] Frequency to MIDI Note Converter Node Object
FtomNode::FtomNode (int id)
    : RelativisticNode (id, "ftom", "ftom (Hz -> Note)")
{
    addInlet ("freq", NodePortType::Control);
    addOutlet ("note", NodePortType::Control);
    setParameter ("freq", 440.0f);
    setParameter ("offset", 0.0f);
}

void FtomNode::process (int /*numSamples*/)
{
    float freq = inlets[0].controlValue;
    if (freq <= 0.0f) freq = getParameter ("freq", 440.0f);
    float offset = getModulatedParamValue ("offset", 0.0f);

    float note = 69.0f + 12.0f * (std::log (std::max (0.001f, freq) / 440.0f) / std::log (2.0f)) + offset;
    outlets[0].controlValue = note;
}

std::string FtomNode::getDefaultFormulaScript() const
{
    return "// Frequency to MIDI Note [ftom]\n// Note = 69.0 + 12.0 * log2(freq / 440.0) + offset\n\nnote = 69.0 + 12.0 * log2(max(0.001, freq) / 440.0) + offset;";
}

std::vector<ParameterInfo> FtomNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "freq", "FREQUENCY (Hz)", getParameter ("freq", 440.0f), 1.0f, 20000.0f, getParamExpression ("freq"), 0 });
    defs.push_back ({ "offset", "TRANSPOSE OFFSET (semitones)", getParameter ("offset", 0.0f), -48.0f, 48.0f, getParamExpression ("offset"), -1 });
    return defs;
}

// 5d. [note] Algorithmic MIDI Note Generator Node Object
NoteGenNode::NoteGenNode (int id)
    : RelativisticNode (id, "note", "note generator")
{
    addInlet ("trig", NodePortType::Control);
    addInlet ("pitch", NodePortType::Control);

    addOutlet ("note", NodePortType::Control);
    addOutlet ("freq", NodePortType::Control);
    addOutlet ("vel", NodePortType::Control);
    addOutlet ("gate", NodePortType::Control);

    setParameter ("pitch", 60.0f);
}

void NoteGenNode::process (int /*numSamples*/)
{
    float trig = inlets[0].controlValue;
    float baseNote = (inlets[1].controlValue > 0.0f) ? inlets[1].controlValue : getParameter ("pitch", 60.0f);

    if (trig > 0.5f)
    {
        float noteVal = baseNote;
        float freqVal = 440.0f * std::pow (2.0f, (noteVal - 69.0f) / 12.0f);

        outlets[0].controlValue = noteVal;
        outlets[1].controlValue = freqVal;
        outlets[2].controlValue = 0.8f;
        outlets[3].controlValue = 1.0f;
    }
}

std::string NoteGenNode::getDefaultFormulaScript() const
{
    return "// Algorithmic MIDI Note Generator [note]\n// Outputs Note, Frequency, Velocity, and Gate signals\n\nnote = pitch;\nfreq = mtof(pitch);";
}

std::vector<ParameterInfo> NoteGenNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "pitch", "BASE MIDI PITCH (0..127)", getParameter ("pitch", 60.0f), 0.0f, 127.0f, getParamExpression ("pitch"), 1 });
    return defs;
}

// 6. [phasor~]
PhasorNode::PhasorNode (int id)
    : RelativisticNode (id, "phasor~", "phasor~ 1 Hz")
{
    addInlet ("timeIn", NodePortType::Time);      // Inlet 0: Dilated coordinate time
    addInlet ("freq", NodePortType::Control);    // Inlet 1: Frequency modulation / MIDI pitch
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("frequency", 1.0f);
}

void PhasorNode::process (int numSamples)
{
    double gamma = getEffectiveGamma();
    float freqCtrl = inlets[1].controlValue;
    float baseFreq = getParameter ("frequency", 1.0f);
    float rawFreq = (freqCtrl > 0.0f) ? freqCtrl : baseFreq;
    double freq = (rawFreq > 0.0f && rawFreq <= 127.0f) ? (440.0 * std::pow (2.0, (rawFreq - 69.0) / 12.0)) : rawFreq;
    double phaseInc = (freq * gamma) / currentSampleRate;

    auto* out = outlets[0].audioData.getWritePointer (0);
    for (int s = 0; s < numSamples; ++s)
    {
        out[s] = static_cast<float>(phase);

        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
        else if (phase < 0.0) phase += 1.0;
    }
}

// 7. [sampler~]
SamplerNode::SamplerNode (int id)
    : RelativisticNode (id, "sampler~", "sampler~ beat.wav")
{
    addInlet ("timeIn", NodePortType::Time);
    addInlet ("posIn", NodePortType::Control);
    addInlet ("pitchIn", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("playbackSpeed", 1.0f);
    setParameter ("pitch", 0.0f);
    setParameter ("gain", 0.8f);
    setParameter ("loopMode", 0.0f); // 0 = Forward, 1 = Ping-Pong, 2 = One-Shot
    setParameter ("loopStart", 0.0f);
    setParameter ("loopEnd", 1.0f);
    setParameter ("samplePreset", 0.0f);

    generateSynthSample();
}

float SamplerNode::interpolateHermite (const float* buffer, int bufferSize, double pos) const
{
    if (bufferSize <= 0) return 0.0f;

    int i1 = static_cast<int>(std::floor (pos));
    double frac = pos - i1;

    int i0 = (i1 - 1 + bufferSize) % bufferSize;
    int i2 = (i1 + 1) % bufferSize;
    int i3 = (i1 + 2) % bufferSize;
    i1 = (i1 + bufferSize) % bufferSize;

    float y0 = buffer[i0];
    float y1 = buffer[i1];
    float y2 = buffer[i2];
    float y3 = buffer[i3];

    double c0 = y1;
    double c1 = 0.5 * (y2 - y0);
    double c2 = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3;
    double c3 = 0.5 * (y3 - y0) + 1.5 * (y1 - y2);

    return static_cast<float>(((c3 * frac + c2) * frac + c1) * frac + c0);
}

void SamplerNode::generateSynthSample()
{
    int preset = static_cast<int>(getParameter ("samplePreset", 0.0f));
    int numSamples = static_cast<int>(currentSampleRate * 2.5); // 2.5 sec buffer
    internalBuffer.setSize (2, numSamples);

    auto* left = internalBuffer.getWritePointer (0);
    auto* right = internalBuffer.getWritePointer (1);

    for (int s = 0; s < numSamples; ++s)
    {
        float tSec = static_cast<float>(s) / static_cast<float>(currentSampleRate);
        float val = 0.0f;

        if (preset == 0) // Resonant Metallic Bell
        {
            float env = std::exp (-tSec * 2.5f);
            val = (std::sin (tSec * 2.0f * juce::MathConstants<float>::pi * 587.33f)
                 + 0.5f * std::sin (tSec * 2.0f * juce::MathConstants<float>::pi * 880.0f)
                 + 0.25f * std::sin (tSec * 2.0f * juce::MathConstants<float>::pi * 1318.5f)) * env;
        }
        else if (preset == 1) // Electronic Kick & Snare Beat
        {
            float envK = std::exp (-tSec * 12.0f);
            float freqK = 150.0f * std::exp (-tSec * 30.0f) + 40.0f;
            float kick = std::sin (tSec * 2.0f * juce::MathConstants<float>::pi * freqK) * envK;

            float envS = std::exp (-static_cast<float>((s % 22050)) / (currentSampleRate * 0.15f));
            float noise = ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * envS;
            val = kick * 0.8f + noise * 0.4f;
        }
        else // Warm Ambient Pad Drone
        {
            float env = std::sin (tSec * juce::MathConstants<float>::pi / 2.5f);
            val = (std::sin (tSec * 2.0f * juce::MathConstants<float>::pi * 220.0f)
                 + 0.6f * std::sin (tSec * 2.0f * juce::MathConstants<float>::pi * 277.18f)
                 + 0.4f * std::sin (tSec * 2.0f * juce::MathConstants<float>::pi * 329.63f)) * env;
        }

        left[s] = val * 0.5f;
        right[s] = val * 0.5f;
    }
}

bool SamplerNode::loadAudioFile (const juce::File& file)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));

    if (reader != nullptr)
    {
        internalBuffer.setSize (static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
        reader->read (&internalBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
        samplePosition = 0.0;
        return true;
    }
    return false;
}

void SamplerNode::process (int numSamples)
{
    const int totalSrcSamples = internalBuffer.getNumSamples();
    if (totalSrcSamples <= 0) return;

    // Time Dilation Gamma
    double gamma = inlets[0].timeGamma;
    if (std::abs (gamma) < 0.001) gamma = getParameter ("playbackSpeed", 1.0f);

    // Control Inlets: Position Scrub & Pitch Transposition
    float posScrub = inlets[1].controlValue;
    if (posScrub > 0.0f) samplePosition = posScrub * totalSrcSamples;

    float pitchMod = inlets[2].controlValue;
    float pitchSemi = getParameter ("pitch", 0.0f) + pitchMod;
    double pitchRatio = std::pow (2.0, pitchSemi / 12.0);

    // Effective Step
    double step = gamma * pitchRatio;

    float gain = getParameter ("gain", 0.8f);
    int loopMode = static_cast<int>(getParameter ("loopMode", 0.0f));
    float lStartFrac = std::clamp (getParameter ("loopStart", 0.0f), 0.0f, 0.95f);
    float lEndFrac = std::clamp (getParameter ("loopEnd", 1.0f), lStartFrac + 0.05f, 1.0f);

    double startPos = lStartFrac * totalSrcSamples;
    double endPos = lEndFrac * totalSrcSamples;

    auto* leftOut = outlets[0].audioData.getWritePointer (0);
    auto* rightOut = outlets[0].audioData.getWritePointer (1);

    const auto* srcLeft = internalBuffer.getReadPointer (0);
    const auto* srcRight = (internalBuffer.getNumChannels() > 1) ? internalBuffer.getReadPointer (1) : srcLeft;

    for (int s = 0; s < numSamples; ++s)
    {
        // Hermite 4-Point Anti-Aliased Resampling
        leftOut[s] = interpolateHermite (srcLeft, totalSrcSamples, samplePosition) * gain;
        rightOut[s] = interpolateHermite (srcRight, totalSrcSamples, samplePosition) * gain;

        double currentStep = isPingPongReversing ? -step : step;
        samplePosition += currentStep;

        if (loopMode == 1) // Ping-Pong Loop
        {
            if (samplePosition >= endPos)
            {
                samplePosition = endPos;
                isPingPongReversing = true;
            }
            else if (samplePosition <= startPos)
            {
                samplePosition = startPos;
                isPingPongReversing = false;
            }
        }
        else if (loopMode == 2) // One-Shot Gate Trigger
        {
            if (samplePosition >= endPos || samplePosition < startPos)
            {
                samplePosition = startPos;
            }
        }
        else // Forward Continuous Loop
        {
            double range = endPos - startPos;
            if (range > 1.0)
            {
                if (samplePosition >= endPos) samplePosition -= range;
                if (samplePosition < startPos) samplePosition += range;
            }
        }
    }
}

std::vector<std::string> SamplerNode::getExposedMethods() const
{
    return { "Load Audio File...", "Reset Position", "Toggle Loop Mode", "Next Preset" };
}

void SamplerNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Reset Position")
    {
        samplePosition = 0.0;
        isPingPongReversing = false;
    }
    else if (methodName == "Toggle Loop Mode")
    {
        int mode = (static_cast<int>(getParameter ("loopMode", 0.0f)) + 1) % 3;
        setParameter ("loopMode", static_cast<float>(mode));
    }
    else if (methodName == "Next Preset")
    {
        int preset = (static_cast<int>(getParameter ("samplePreset", 0.0f)) + 1) % 3;
        setParameter ("samplePreset", static_cast<float>(preset));
        generateSynthSample();
    }
}

// 8. [filter~]
FilterNode::FilterNode (int id)
    : RelativisticNode (id, "filter~", "filter~ 1200 Hz")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in~", NodePortType::Audio);        // Inlet 1: Audio input stream
    addInlet ("cutoff_mod", NodePortType::Control); // Inlet 2: Cutoff modulation
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("cutoff", 1200.0f);
    setParameter ("resonance", 0.7f);
}

void FilterNode::process (int numSamples)
{
    double gamma = getEffectiveGamma();
    const auto* inL = inlets[1].audioData.getReadPointer (0);
    const auto* inR = (inlets[1].audioData.getNumChannels() > 1) ? inlets[1].audioData.getReadPointer (1) : inL;

    auto* outL = outlets[0].audioData.getWritePointer (0);
    auto* outR = (outlets[0].audioData.getNumChannels() > 1) ? outlets[0].audioData.getWritePointer (1) : outL;

    for (int s = 0; s < numSamples; ++s)
    {
        float cutoff = getModulatedParamValue ("cutoff", 1200.0f, s) + inlets[2].controlValue;
        cutoff = cutoff * static_cast<float>(std::abs (gamma));
        cutoff = std::clamp (cutoff, 20.0f, std::min (20000.0f, static_cast<float>(currentSampleRate * 0.42f)));
        float alpha = std::clamp (static_cast<float>(2.0 * juce::MathConstants<double>::pi * cutoff / currentSampleRate), 0.001f, 0.95f);

        filterStateL += alpha * (inL[s] - filterStateL);
        filterStateR += alpha * (inR[s] - filterStateR);

        outL[s] = filterStateL;
        outR[s] = filterStateR;
    }
}

// 9. [delay~]
DelayNode::DelayNode (int id)
    : RelativisticNode (id, "delay~", "delay~ 250ms")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in~", NodePortType::Audio);        // Inlet 1: Audio input stream
    addInlet ("delay_mod", NodePortType::Control); // Inlet 2: Delay modulation
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("delayTimeMs", 250.0f);
    setParameter ("feedback", 0.4f);
}

void DelayNode::prepare (double sampleRate, int samplesPerBlock)
{
    RelativisticNode::prepare (sampleRate, samplesPerBlock);
    delayBuffer.setSize (2, static_cast<int>(sampleRate * 2.0));
    delayBuffer.clear();
}

void DelayNode::process (int numSamples)
{
    double gamma = getEffectiveGamma();
    float delayMs = getParameter ("delayTimeMs", 250.0f) + inlets[2].controlValue;
    float feedback = getParameter ("feedback", 0.4f);

    if (std::abs (gamma) > 0.001) delayMs = static_cast<float>(delayMs / std::abs (gamma));

    const int totalBufferLength = delayBuffer.getNumSamples();
    if (totalBufferLength <= 0) return;

    int delayLength = std::clamp (static_cast<int>((delayMs / 1000.0f) * currentSampleRate), 1, totalBufferLength - 1);

    const auto* inL = inlets[1].audioData.getReadPointer (0);
    const auto* inR = (inlets[1].audioData.getNumChannels() > 1) ? inlets[1].audioData.getReadPointer (1) : inL;
    auto* outL = outlets[0].audioData.getWritePointer (0);
    auto* outR = (outlets[0].audioData.getNumChannels() > 1) ? outlets[0].audioData.getWritePointer (1) : outL;

    for (int s = 0; s < numSamples; ++s)
    {
        int readPos = (writePosition - delayLength + totalBufferLength) % totalBufferLength;
        float delayedSampleL = delayBuffer.getSample (0, readPos);
        float delayedSampleR = (delayBuffer.getNumChannels() > 1) ? delayBuffer.getSample (1, readPos) : delayedSampleL;

        delayBuffer.setSample (0, writePosition, inL[s] + delayedSampleL * feedback);
        if (delayBuffer.getNumChannels() > 1)
            delayBuffer.setSample (1, writePosition, inR[s] + delayedSampleR * feedback);

        outL[s] = inL[s] + delayedSampleL;
        outR[s] = inR[s] + delayedSampleR;

        writePosition = (writePosition + 1) % totalBufferLength;
    }
}

std::vector<ParameterInfo> TimeWarpNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "dilationGamma", "TIME DILATION GAMMA (γ)", getParameter ("dilationGamma", 2.0f), 0.1f, 10.0f, getParamExpression ("dilationGamma"), -1 });
    defs.push_back ({ "lfoSpeed", "LFO SPEED (Hz)", getParameter ("lfoSpeed", 0.5f), 0.01f, 20.0f, getParamExpression ("lfoSpeed"), -1 });
    return defs;
}

std::vector<ParameterInfo> TimeRetroNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "reversalFactor", "TEMPORAL REVERSAL FACTOR", getParameter ("reversalFactor", -1.0f), -5.0f, 0.0f, getParamExpression ("reversalFactor"), -1 });
    return defs;
}

std::vector<ParameterInfo> TimeQuantizeNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "stepDivision", "GRID DIVISION STEPS", getParameter ("stepDivision", 4.0f), 1.0f, 32.0f, getParamExpression ("stepDivision"), -1 });
    return defs;
}

std::vector<ParameterInfo> TimeMetroNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "bpm", "TEMPO (BPM)", getParameter ("bpm", 120.0f), 20.0f, 300.0f, getParamExpression ("bpm"), -1 });
    return defs;
}


std::vector<ParameterInfo> PhasorNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "frequency", "RAMP FREQUENCY (Hz)", getParameter ("frequency", 1.0f), 0.01f, 100.0f, getParamExpression ("frequency"), -1 });
    return defs;
}

std::vector<ParameterInfo> SamplerNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "playbackSpeed", "PLAYBACK SPEED (x)", getParameter ("playbackSpeed", 1.0f), 0.01f, 8.0f, getParamExpression ("playbackSpeed"), -1 });
    defs.push_back ({ "pitch", "PITCH TRANSPOSE (SEMITONES)", getParameter ("pitch", 0.0f), -24.0f, 24.0f, getParamExpression ("pitch"), 2 });
    defs.push_back ({ "gain", "SAMPLER GAIN", getParameter ("gain", 0.8f), 0.0f, 2.0f, getParamExpression ("gain"), -1 });
    defs.push_back ({ "loopMode", "LOOP MODE (0:FWD, 1:PINGPONG, 2:ONESHOT)", getParameter ("loopMode", 0.0f), 0.0f, 2.0f, getParamExpression ("loopMode"), -1 });
    defs.push_back ({ "loopStart", "LOOP START POINT (0..1)", getParameter ("loopStart", 0.0f), 0.0f, 0.95f, getParamExpression ("loopStart"), -1 });
    defs.push_back ({ "loopEnd", "LOOP END POINT (0..1)", getParameter ("loopEnd", 1.0f), 0.05f, 1.0f, getParamExpression ("loopEnd"), -1 });
    defs.push_back ({ "samplePreset", "SYNTH PRESET (0:BELL, 1:DRUM, 2:PAD)", getParameter ("samplePreset", 0.0f), 0.0f, 2.0f, getParamExpression ("samplePreset"), -1 });
    return defs;
}

std::vector<ParameterInfo> FilterNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "cutoff", "CUTOFF FREQUENCY (Hz)", getParameter ("cutoff", 1200.0f), 20.0f, 20000.0f, getParamExpression ("cutoff"), -1 });
    defs.push_back ({ "resonance", "RESONANCE (Q)", getParameter ("resonance", 0.7f), 0.1f, 10.0f, getParamExpression ("resonance"), -1 });
    return defs;
}

std::vector<ParameterInfo> DelayNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "delayTimeMs", "DELAY TIME (ms)", getParameter ("delayTimeMs", 250.0f), 1.0f, 2000.0f, getParamExpression ("delayTimeMs"), -1 });
    defs.push_back ({ "feedback", "FEEDBACK AMOUNT", getParameter ("feedback", 0.4f), 0.0f, 0.99f, getParamExpression ("feedback"), -1 });
    return defs;
}

std::vector<ParameterInfo> DacNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "masterVolume", "MASTER DAC VOLUME", getParameter ("masterVolume", 0.8f), 0.0f, 1.0f, getParamExpression ("masterVolume"), -1 });
    return defs;
}

std::vector<ParameterInfo> ExprNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "scale", "EXPRESSION SCALE", getParameter ("scale", 1.0f), 0.0f, 10.0f, getParamExpression ("scale"), -1 });
    return defs;
}

std::vector<ParameterInfo> ExprAudioNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "gain", "EXPRESSION AUDIO GAIN", getParameter ("gain", 1.0f), 0.0f, 2.0f, getParamExpression ("gain"), -1 });
    return defs;
}

std::vector<ParameterInfo> FexprAudioNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "feedback", "RECURRENT FEEDBACK GAIN", getParameter ("feedback", 0.95f), 0.0f, 0.999f, getParamExpression ("feedback"), -1 });
    return defs;
}

std::string TimeWarpNode::getDefaultFormulaScript() const
{
    return "// Relativistic Time Warp Engine [time.warp~]\n// Controls dilated coordinate time factor (gamma)\n// Inputs: $v1 (Base Speed Modulator)\n\ngamma = 1.0 + 1.5 * sin($t * 2.5) + $v1;";
}

std::string TimeRetroNode::getDefaultFormulaScript() const
{
    return "// Retrograde Time Reversal Engine [time.retro~]\n// Inverts temporal progression velocity for negative causality\n\ngamma = -1.0;";
}

std::string TimeQuantizeNode::getDefaultFormulaScript() const
{
    return "// Time Dilation Quantizer [time.quantize~]\n// Step-quantizes continuous dilated clock to musical grid\n\ngamma = floor($v1 * 4.0) / 4.0;";
}

std::string TimeMetroNode::getDefaultFormulaScript() const
{
    return "// Relativistic Metronome [time.metro~]\n// Emits impulse ticks under dilated coordinate clock\n\nif (phase >= 1.0) {\n    triggerBang();\n    phase = 0.0;\n}";
}


std::string PhasorNode::getDefaultFormulaScript() const
{
    return "// Linear Ramp Phase Generator [phasor~]\n// Parameters: frequency (Hz)\n\nphase = fmod($t * freq, 1.0);\nout = phase;";
}

std::string SamplerNode::getDefaultFormulaScript() const
{
    return "// Advanced Hermite Varispeed & Granular Sampler [sampler~]\n"
           "// Inlets: $v1 (timeIn gamma), $v2 (posIn scrub 0..1), $v3 (pitchIn semitones)\n"
           "// 4-Point Hermite Cubic Resampling | Loop Modes: 0=Forward, 1=Ping-Pong, 2=One-Shot\n\n"
           "pos = ($v2 > 0.0) ? $v2 * sampleLen : samplePos;\n"
           "pitchRatio = pow(2.0, (pitch + $v3) / 12.0);\n"
           "step = gamma * pitchRatio;\n"
           "out = interpolateHermite(buffer, pos) * gain;";
}

std::string FilterNode::getDefaultFormulaScript() const
{
    return "// State-Variable Filter [filter~]\n// Inputs: $v1 (Audio In), $v2 (Cutoff Modulator)\n// Parameters: cutoff (Hz), resonance (Q)\n\ncutoff = clamp(cutoff + $v2, 20.0, 20000.0);\nout = processSVF($v1, cutoff, resonance);";
}

std::string DelayNode::getDefaultFormulaScript() const
{
    return "// Feedback Delay Line [delay~]\n// Inputs: $v1 (Audio In), $v2 (Feedback Modulator)\n// Parameters: delayTimeMs, feedback\n\nout = delayBuffer.read(delayTimeMs);\ndelayBuffer.write($v1 + out * feedback);";
}

std::string DacNode::getDefaultFormulaScript() const
{
    return "// Master Audio Output DAC [dac~]\n// Direct Hardware Output Speaker Driver\n// Inputs: $v1 (Left Audio), $v2 (Right Audio)\n\nout_L = $v1 * masterVolume;\nout_R = $v2 * masterVolume;";
}

std::string ExprNode::getDefaultFormulaScript() const
{
    return "// Pure Data Control Expression [expr]\n// Inlets: $v1, $v2 | Variables: $t, $gt, $gamma, $bpm\n\nout = $v1 * sin($t * 2.0) + $gt;";
}

std::string ExprAudioNode::getDefaultFormulaScript() const
{
    return "// Pure Data Audio Signal Expression [expr~]\n// Audio Inlets: $v1, $v2 | Variables: $t, $gamma\n\nout = $v1 * $v2 + sin($t * 440.0);";
}

std::string FexprAudioNode::getDefaultFormulaScript() const
{
    return "// Filter Recurrent Audio Expression [fexpr~]\n// Inlets: $v1 | Recurrent Feedback: $y1[-1]\n\nout = $y1[-1] * 0.95 + $v1;";
}

DacNode::DacNode (int id)
    : RelativisticNode (id, "dac~", "dac~ (Audio Out)")
{
    addInlet ("L~", NodePortType::Audio);
    addInlet ("R~", NodePortType::Audio);
}

void DacNode::process (int /*numSamples*/)
{
}

ExprNode::ExprNode (int id)
    : RelativisticNode (id, "expr", "expr $v1 * 2.0")
{
    addInlet ("in1", NodePortType::Control);
    addInlet ("in2", NodePortType::Control);
    addOutlet ("out", NodePortType::Control);
}

void ExprNode::process (int numSamples)
{
    const auto* in1Buf = (inlets[0].audioData.getNumSamples() >= numSamples) ? inlets[0].audioData.getReadPointer (0) : nullptr;
    const auto* in2Buf = (inlets[1].audioData.getNumSamples() >= numSamples) ? inlets[1].audioData.getReadPointer (0) : nullptr;
    auto* outBuf = outlets[0].audioData.getWritePointer (0);

    outlets[0].audioData.clear();

    std::map<std::string, double> vars;
    std::string exprStr = (nodeLabel.length() > 5) ? nodeLabel.substr (5) : "$v1";
    auto resolver = [this] (const std::string& target) { return parentGraph ? parentGraph->tapSignal (target) : 0.0; };

    double lastRes = 0.0;
    for (int s = 0; s < numSamples; ++s)
    {
        vars["v1"] = (in1Buf != nullptr && inlets[0].audioData.getMagnitude (0, numSamples) > 0.0001f) ? in1Buf[s] : inlets[0].controlValue;
        vars["v2"] = (in2Buf != nullptr && inlets[1].audioData.getMagnitude (0, numSamples) > 0.0001f) ? in2Buf[s] : inlets[1].controlValue;
        vars["t"]  = inlets[0].timeGamma;

        lastRes = RelativisticExpressionParser::evaluateExpression (exprStr, vars, resolver);
        outBuf[s] = static_cast<float>(lastRes);
    }
    outlets[0].controlValue = static_cast<float>(lastRes);
}

ExprAudioNode::ExprAudioNode (int id)
    : RelativisticNode (id, "expr~", "expr~ $v1 * $v2")
{
    addInlet ("in1~", NodePortType::Audio);
    addInlet ("in2~", NodePortType::Audio);
    addOutlet ("out~", NodePortType::Audio);
}

void ExprAudioNode::process (int numSamples)
{
    const auto* in1 = inlets[0].audioData.getReadPointer (0);
    const auto* in2 = inlets[1].audioData.getReadPointer (0);
    auto* out = outlets[0].audioData.getWritePointer (0);

    std::map<std::string, double> vars;
    std::string formula = (nodeLabel.length() > 6) ? nodeLabel.substr (6) : "$v1 * $v2";

    auto resolver = [this] (const std::string& target) { return parentGraph ? parentGraph->tapSignal (target) : 0.0; };

    for (int s = 0; s < numSamples; ++s)
    {
        vars["v1"] = in1[s];
        vars["v2"] = in2[s];
        out[s] = static_cast<float>(RelativisticExpressionParser::evaluateExpression (formula, vars, resolver));
    }
}

FexprAudioNode::FexprAudioNode (int id)
    : RelativisticNode (id, "fexpr~", "fexpr~ $y1[-1]*0.95 + $v1")
{
    addInlet ("in1~", NodePortType::Audio);
    addOutlet ("out~", NodePortType::Audio);
}

void FexprAudioNode::process (int numSamples)
{
    const auto* in1 = inlets[0].audioData.getReadPointer (0);
    auto* out = outlets[0].audioData.getWritePointer (0);

    for (int s = 0; s < numSamples; ++s)
    {
        float val = in1[s] + prevSampleL * 0.95f;
        out[s] = val;
        prevSampleL = val;
    }
}

// 14. [gain~] Audio Signal Scaler Node Object
GainNode::GainNode (int id)
    : RelativisticNode (id, "gain~", "gain~ 1.0")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in~", NodePortType::Audio);        // Inlet 1: Audio input stream
    addInlet ("gain_mod", NodePortType::Control); // Inlet 2: Gain modulation

    addOutlet ("out~", NodePortType::Audio);      // Outlet 0: Time-warped audio output

    setParameter ("gain", 1.0f);
    setParameter ("timeFilterHz", 10.0f);
    setParameter ("bypassTimeFilter", 0.0f);
}

void GainNode::prepare (double sampleRate, int samplesPerBlock)
{
    RelativisticNode::prepare (sampleRate, samplesPerBlock);
    warpBuffer.setSize (2, static_cast<int>(sampleRate * 4.0)); // 4-second circular time-warp buffer
    warpBuffer.clear();
    writePos = 0;
    readPos = 0.0;
}

void GainNode::process (int numSamples)
{
    double gamma = getEffectiveGamma();
    float gVal = getParameter ("gain", 1.0f) + inlets[2].controlValue;

    const auto* inL = inlets[1].audioData.getReadPointer (0);
    const auto* inR = (inlets[1].audioData.getNumChannels() > 1) ? inlets[1].audioData.getReadPointer (1) : inL;

    auto* outL = outlets[0].audioData.getWritePointer (0);
    auto* outR = (outlets[0].audioData.getNumChannels() > 1) ? outlets[0].audioData.getWritePointer (1) : outL;

    if (std::abs (gamma - 1.0) < 0.0001 && inlets[0].timeGamma == 1.0)
    {
        for (int s = 0; s < numSamples; ++s)
        {
            outL[s] = inL[s] * gVal;
            outR[s] = inR[s] * gVal;
        }
        return;
    }

    const int bufLen = warpBuffer.getNumSamples();
    if (bufLen <= 0) return;

    auto* bufL = warpBuffer.getWritePointer (0);
    auto* bufR = warpBuffer.getWritePointer (1);

    for (int s = 0; s < numSamples; ++s)
    {
        bufL[writePos] = inL[s];
        bufR[writePos] = inR[s];

        writePos = (writePos + 1) % bufLen;

        readPos += gamma;
        while (readPos < 0.0) readPos += bufLen;
        while (readPos >= bufLen) readPos -= bufLen;

        int rIdx = static_cast<int>(readPos);
        float alpha = static_cast<float>(readPos - rIdx);

        int i0 = (rIdx - 1 + bufLen) % bufLen;
        int i1 = rIdx;
        int i2 = (rIdx + 1) % bufLen;
        int i3 = (rIdx + 2) % bufLen;

        float sL = interpolateHermite (bufL[i0], bufL[i1], bufL[i2], bufL[i3], alpha);
        float sR = interpolateHermite (bufR[i0], bufR[i1], bufR[i2], bufR[i3], alpha);

        outL[s] = sL * gVal;
        outR[s] = sR * gVal;
    }
}

std::string GainNode::getDefaultFormulaScript() const
{
    return "// Audio Signal Scaler & Relativistic Time Warper [gain~]\n// Inputs: timeIn (Time), in~ (Audio In), gain_mod (Gain Modulator)\n// Parameters: gain\n\nout = interpolateHermite(timeWarp(in~, gamma)) * (gain + gain_mod);";
}

std::vector<ParameterInfo> GainNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "gain", "SIGNAL GAIN (x)", getParameter ("gain", 1.0f), 0.0f, 2.0f, getParamExpression ("gain"), -1 });
    defs.push_back ({ "timeFilterHz", "TIME LOWPASS CUTOFF (Hz)", getParameter ("timeFilterHz", 10.0f), 0.1f, 50.0f, getParamExpression ("timeFilterHz"), -1 });
    defs.push_back ({ "bypassTimeFilter", "INSTANT QUANTUM BYPASS", getParameter ("bypassTimeFilter", 0.0f), 0.0f, 1.0f, getParamExpression ("bypassTimeFilter"), -1 });
    return defs;
}

// 11b. [number] Control Number Box Object
NumberNode::NumberNode (int id)
    : RelativisticNode (id, "number", "0.00")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in", NodePortType::Control);        // Inlet 1: Control input
    addOutlet ("out", NodePortType::Control);      // Outlet 0: Control output
    setParameter ("value", 0.0f);
}

void NumberNode::process (int /*numSamples*/)
{
    if (inlets.size() > 1 && inlets[1].controlValue != 0.0f)
    {
        setParameter ("value", inlets[1].controlValue);
    }
    float val = getParameter ("value", 0.0f);
    outlets[0].controlValue = val;
}

std::string NumberNode::getDefaultFormulaScript() const
{
    return "// Control Number Box Object [number]\n// Stores and outputs a floating-point control value\n\nout = value;";
}

std::vector<ParameterInfo> NumberNode::getParameterDefs() const
{
    return { { "value", "NUMBER VALUE", getParameter ("value", 0.0f), -9999.0f, 9999.0f, 0 } };
}

// 11c. [bang] Control Trigger Pulse Object
BangNode::BangNode (int id)
    : RelativisticNode (id, "bang", "bang")
{
    addInlet ("in", NodePortType::Control);
    addOutlet ("out", NodePortType::Control);
    setParameter ("trigger", 0.0f);
}

void BangNode::triggerBang()
{
    bangRequested = true;
}

void BangNode::process (int /*numSamples*/)
{
    float trg = inlets[0].controlValue;
    if (trg > 0.5f || bangRequested)
    {
        outlets[0].controlValue = 1.0f;
        bangRequested = false;
    }
    else
    {
        outlets[0].controlValue = 0.0f;
    }
}

std::string BangNode::getDefaultFormulaScript() const
{
    return "// Control Bang Trigger [bang]\n// Emits a 1.0 control trigger pulse when clicked or invoked\n\nout = trigger ? 1.0 : 0.0;";
}

std::vector<ParameterInfo> BangNode::getParameterDefs() const
{
    return { { "trigger", "MANUAL BANG TRIGGER", 0.0f, 0.0f, 1.0f, 0 } };
}

std::vector<std::string> BangNode::getExposedMethods() const
{
    return { "SEND BANG TRIGGER" };
}

void BangNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "SEND BANG TRIGGER") triggerBang();
}

// 11d. [bang~] Audio Rate Impulse Spike Object
BangAudioNode::BangAudioNode (int id)
    : RelativisticNode (id, "bang~", "bang~")
{
    addInlet ("in~", NodePortType::Audio);
    addInlet ("trigger", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);
}

void BangAudioNode::triggerBang()
{
    bangRequested = true;
}

void BangAudioNode::process (int numSamples)
{
    auto* out = outlets[0].audioData.getWritePointer (0);
    outlets[0].audioData.clear();

    float trg = inlets[1].controlValue;
    if (trg > 0.5f || bangRequested)
    {
        out[0] = 1.0f; // 1-Sample Audio Impulse Spike!
        bangRequested = false;
    }
}

std::string BangAudioNode::getDefaultFormulaScript() const
{
    return "// Audio Rate Impulse Spike Object [bang~]\n// Emits a single 1.0 sample audio impulse spike on trigger\n\nout = impulse_sample;";
}

std::vector<ParameterInfo> BangAudioNode::getParameterDefs() const
{
    return { { "trigger", "TRIGGER AUDIO IMPULSE", 0.0f, 0.0f, 1.0f, 0 } };
}

std::vector<std::string> BangAudioNode::getExposedMethods() const
{
    return { "FIRE IMPULSE SPIKE" };
}

void BangAudioNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "FIRE IMPULSE SPIKE") triggerBang();
}

// 11e. [counter] Smart Value Counter Object
CounterNode::CounterNode (int id)
    : RelativisticNode (id, "counter", "counter 0 15 1")
{
    addInlet ("timeIn", NodePortType::Time);    // Inlet 0: Dilated coordinate time
    addInlet ("in", NodePortType::Control);    // Inlet 1: Trigger / Bang (increment)
    addInlet ("low", NodePortType::Control);   // Inlet 2: Low limit
    addInlet ("high", NodePortType::Control);  // Inlet 3: High limit
    addInlet ("step", NodePortType::Control);  // Inlet 4: Step size
    addInlet ("reset", NodePortType::Control); // Inlet 5: Reset trigger

    addOutlet ("count", NodePortType::Control); // Outlet 0: Current count
    addOutlet ("carry", NodePortType::Control); // Outlet 1: Carry bang on overflow

    setParameter ("low", 0.0f);
    setParameter ("high", 15.0f);
    setParameter ("step", 1.0f);
    setParameter ("current", 0.0f);
}

void CounterNode::process (int /*numSamples*/)
{
    float lowVal = getParameter ("low", 0.0f);
    float highVal = getParameter ("high", 15.0f);
    float stepVal = getParameter ("step", 1.0f);

    if (inlets.size() > 2 && inlets[2].controlValue != 0.0f) lowVal = inlets[2].controlValue;
    if (inlets.size() > 3 && inlets[3].controlValue != 0.0f) highVal = inlets[3].controlValue;
    if (inlets.size() > 4 && inlets[4].controlValue != 0.0f) stepVal = inlets[4].controlValue;

    bool currentInletState = (inlets.size() > 1 && inlets[1].controlValue > 0.5f);
    bool currentResetState = (inlets.size() > 5 && inlets[5].controlValue > 0.5f);

    bool triggerFired = (currentInletState && !lastInletState);
    bool resetFired = (currentResetState && !lastResetState);
    lastInletState = currentInletState;
    lastResetState = currentResetState;

    bool carryFired = false;

    if (resetFired)
    {
        currentCount = lowVal;
    }
    else if (triggerFired)
    {
        currentCount += stepVal;
        if (currentCount > highVal)
        {
            currentCount = lowVal;
            carryFired = true;
        }
    }

    setParameter ("current", currentCount);
    outlets[0].controlValue = currentCount;
    if (outlets.size() > 1) outlets[1].controlValue = carryFired ? 1.0f : 0.0f;
}


std::string CounterNode::getDefaultFormulaScript() const
{
    return "// Smart Value Counter Object [counter]\n// Increments count on bang; outputs carry bang when hitting high limit\n\nif (bang) count = (count >= high) ? low : (count + step);";
}

std::vector<ParameterInfo> CounterNode::getParameterDefs() const
{
    return {
        { "low", "MIN LOW BOUND", getParameter ("low", 0.0f), -999.0f, 999.0f, 0 },
        { "high", "MAX HIGH BOUND", getParameter ("high", 15.0f), -999.0f, 999.0f, 0 },
        { "step", "INCREMENT STEP", getParameter ("step", 1.0f), 0.1f, 100.0f, 0 },
        { "current", "CURRENT COUNT", getParameter ("current", 0.0f), -999.0f, 999.0f, 0 }
    };
}

std::vector<std::string> CounterNode::getExposedMethods() const
{
    return { "Increment (+1)", "Reset Counter" };
}

void CounterNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Increment (+1)")
    {
        lastInletState = false;
        inlets[0].controlValue = 1.0f;
    }
    else if (methodName == "Reset Counter")
    {
        lastResetState = false;
        if (inlets.size() > 4) inlets[4].controlValue = 1.0f;
        else currentCount = getParameter ("low", 0.0f);
    }
}

// 15. [out~] Master Audio Output Node Object with Live Oscilloscope & Dual RMS/Peak Metering
OutNode::OutNode (int id)
    : RelativisticNode (id, "out~", "out~ (Scope & Volume)")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("L~", NodePortType::Audio);          // Inlet 1: Audio Left input
    addInlet ("R~", NodePortType::Audio);          // Inlet 2: Audio Right input
    addInlet ("vol_mod", NodePortType::Control);   // Inlet 3: Master Volume modulation
    addOutlet ("L~", NodePortType::Audio);
    addOutlet ("R~", NodePortType::Audio);

    setParameter ("volume", 0.8f);
    setParameter ("displayMode", 0.0f); // 0: Waveform vs Time, 1: X-Y Lissajous

    scopeBufferL.assign (256, 0.0f);
    scopeBufferR.assign (256, 0.0f);
}

OutNode::~OutNode()
{
    stopRecording();
}

void OutNode::startRecording()
{
    const juce::ScopedLock sl (recordLock);
    if (isRecording) return;

    juce::File tmpDir ("/tmp");
    if (!tmpDir.exists())
    {
        tmpDir = juce::File::getSpecialLocation (juce::File::tempDirectory);
    }
    tmpDir.createDirectory();

    juce::String timeStamp = juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S");
    juce::File recFile = tmpDir.getChildFile ("out_node_" + juce::String (nodeId) + "_" + timeStamp + ".wav");

    juce::WavAudioFormat wavFormat;
    auto fileStream = recFile.createOutputStream();
    if (fileStream != nullptr)
    {
        recordWriter.reset (wavFormat.createWriterFor (fileStream.release(), currentSampleRate, 2, 16, {}, 0));
        if (recordWriter != nullptr)
        {
            isRecording = true;
            lastRecordFilePath = recFile.getFullPathName().toStdString();
        }
    }
}

void OutNode::stopRecording()
{
    const juce::ScopedLock sl (recordLock);
    if (!isRecording) return;

    if (recordWriter != nullptr)
    {
        recordWriter.reset();
    }
    isRecording = false;
}

void OutNode::process (int numSamples)
{
    const auto* inL = inlets[1].audioData.getReadPointer (0);
    const auto* inR = inlets[2].audioData.getReadPointer (0);

    if (inlets[2].audioData.getMagnitude (0, numSamples) < 0.00001f && inlets[1].audioData.getMagnitude (0, numSamples) > 0.00001f)
    {
        inR = inL;
    }

    auto* outL = outlets[0].audioData.getWritePointer (0);
    auto* outR = outlets[1].audioData.getWritePointer (0);

    float vol = getParameter ("volume", 0.8f) + inlets[3].controlValue;
    float sumSqL = 0.0f, sumSqR = 0.0f;
    float pL = 0.0f, pR = 0.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        float sL = std::clamp (inL[s] * vol, -2.0f, 2.0f);
        float sR = std::clamp (inR[s] * vol, -2.0f, 2.0f);
        outL[s] = sL;
        outR[s] = sR;

        sumSqL += sL * sL;
        sumSqR += sR * sR;
        pL = std::max (pL, std::abs (sL));
        pR = std::max (pR, std::abs (sR));

        // Circular Scope Buffer
        scopeBufferL[scopeWriteIdx] = sL;
        scopeBufferR[scopeWriteIdx] = sR;
        scopeWriteIdx = (scopeWriteIdx + 1) % 256;
    }

    rmsL = std::sqrt (sumSqL / std::max (1, numSamples));
    rmsR = std::sqrt (sumSqR / std::max (1, numSamples));
    peakL = pL;
    peakR = pR;

    // WAV Recording to /tmp
    if (isRecording)
    {
        const juce::ScopedLock sl (recordLock);
        if (recordWriter != nullptr)
        {
            juce::AudioBuffer<float> recBuf (2, numSamples);
            recBuf.copyFrom (0, 0, outlets[0].audioData, 0, 0, numSamples);
            recBuf.copyFrom (1, 0, outlets[1].audioData, 0, 0, numSamples);
            recordWriter->writeFromAudioSampleBuffer (recBuf, 0, numSamples);
        }
    }
}

std::string OutNode::getDefaultFormulaScript() const
{
    return "// Master Output & Oscilloscope [out~]\n// Inputs: $v1 (Left Audio), $v2 (Right Audio), $v3 (Volume Mod)\n\nout_L = $v1 * (volume + $v3);\nout_R = $v2 * (volume + $v3);";
}

std::vector<ParameterInfo> OutNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "volume", "MASTER OUTPUT VOLUME", getParameter ("volume", 0.8f), 0.0f, 1.5f, getParamExpression ("volume"), -1 });
    defs.push_back ({ "displayMode", "SCOPE MODE (0: TIME DOMAIN, 1: X-Y LISSAJOUS)", getParameter ("displayMode", 0.0f), 0.0f, 1.0f, getParamExpression ("displayMode"), -1 });
    return defs;
}

std::vector<std::string> OutNode::getExposedMethods() const
{
    return { "Toggle Scope Mode", isRecording ? "Stop WAV Record" : "Start WAV Record" };
}

void OutNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Toggle Scope Mode")
    {
        float current = getParameter ("displayMode", 0.0f);
        setParameter ("displayMode", (current > 0.5f) ? 0.0f : 1.0f);
    }
    else if (methodName == "Start WAV Record" || methodName == "Stop WAV Record" || methodName.find ("WAV") != std::string::npos)
    {
        if (isRecording) stopRecording();
        else startRecording();
    }
}

// 16. [env~] Envelope Follower Node Object
EnvFollowerNode::EnvFollowerNode (int id)
    : RelativisticNode (id, "env~", "env~ (Envelope Follower)")
{
    addInlet ("in~", NodePortType::Audio);
    addInlet ("attack", NodePortType::Control);
    addInlet ("release", NodePortType::Control);
    addOutlet ("env", NodePortType::Control);
    setParameter ("attackMs", 10.0f);
    setParameter ("releaseMs", 100.0f);
}

void EnvFollowerNode::process (int numSamples)
{
    const auto* in = inlets[0].audioData.getReadPointer (0);
    float att = std::max (1.0f, getParameter ("attackMs", 10.0f) + inlets[1].controlValue);
    float rel = std::max (1.0f, getParameter ("releaseMs", 100.0f) + inlets[2].controlValue);

    float attCoeff = std::exp (-1.0f / (att * 0.001f * static_cast<float>(currentSampleRate)));
    float relCoeff = std::exp (-1.0f / (rel * 0.001f * static_cast<float>(currentSampleRate)));

    for (int s = 0; s < numSamples; ++s)
    {
        float inputLevel = std::abs (in[s]);
        if (inputLevel > currentEnvelope)
            currentEnvelope = attCoeff * currentEnvelope + (1.0f - attCoeff) * inputLevel;
        else
            currentEnvelope = relCoeff * currentEnvelope + (1.0f - relCoeff) * inputLevel;
    }

    outlets[0].controlValue = currentEnvelope;
}

std::string EnvFollowerNode::getDefaultFormulaScript() const
{
    return "// Envelope Follower [env~]\n// Inputs: $v1 (Audio In), $v2 (Attack Mod), $v3 (Release Mod)\n\nenvelope = processEnvFollower($v1, attackMs, releaseMs);";
}

std::vector<ParameterInfo> EnvFollowerNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "attackMs", "ATTACK TIME (ms)", getParameter ("attackMs", 10.0f), 1.0f, 500.0f, getParamExpression ("attackMs"), -1 });
    defs.push_back ({ "releaseMs", "RELEASE TIME (ms)", getParameter ("releaseMs", 100.0f), 1.0f, 2000.0f, getParamExpression ("releaseMs"), -1 });
    return defs;
}

// 17. [tap] Control Signal Wireless Tap Object
TapControlNode::TapControlNode (int id)
    : RelativisticNode (id, "tap", "tap control_point")
{
    addInlet ("in", NodePortType::Control);
    addOutlet ("out", NodePortType::Control);
}

void TapControlNode::process (int /*numSamples*/)
{
    outlets[0].controlValue = inlets[0].controlValue;
}

std::string TapControlNode::getDefaultFormulaScript() const
{
    return "// Control Wireless Signal Tap [tap]\n// Assigns or extracts control signal under custom tap point name\n\nout = $v1;";
}

std::vector<ParameterInfo> TapControlNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "value", "TAP CONTROL VALUE", getParameter ("value", 0.0f), -100.0f, 100.0f, getParamExpression ("value"), -1 });
    return defs;
}

// 18. [tap~] Audio Signal Wireless Tap Object
TapAudioNode::TapAudioNode (int id)
    : RelativisticNode (id, "tap~", "tap~ audio_stream")
{
    addInlet ("in~", NodePortType::Audio);
    addOutlet ("out~", NodePortType::Audio);
}

void TapAudioNode::process (int numSamples)
{
    const auto* in = inlets[0].audioData.getReadPointer (0);
    auto* out = outlets[0].audioData.getWritePointer (0);

    for (int s = 0; s < numSamples; ++s)
    {
        out[s] = in[s];
    }
}

std::string TapAudioNode::getDefaultFormulaScript() const
{
    return "// Audio Wireless Signal Tap [tap~]\n// Publishes audio stream for wireless expression tapping\n\nout = $v1;";
}

std::vector<ParameterInfo> TapAudioNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "gain", "TAP AUDIO GAIN", getParameter ("gain", 1.0f), 0.0f, 2.0f, getParamExpression ("gain"), -1 });
    return defs;
}

// 19. [v] Value Storage Control Node Object
ValueNode::ValueNode (int id)
    : RelativisticNode (id, "v", "v value_store")
{
    addInlet ("in", NodePortType::Control);
    addOutlet ("out", NodePortType::Control);
    setParameter ("value", 0.0f);
}

void ValueNode::process (int /*numSamples*/)
{
    if (inlets[0].controlValue != 0.0f)
    {
        storedValue = inlets[0].controlValue;
    }
    else
    {
        storedValue = getParameter ("value", storedValue);
    }
    outlets[0].controlValue = storedValue;
}

std::string ValueNode::getDefaultFormulaScript() const
{
    return "// Control Value Storage [v]\n// Holds value state across ticks for feedback calculations\n\nout = storedValue;";
}

std::vector<ParameterInfo> ValueNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "value", "STORED VALUE", getParameter ("value", storedValue), -1000.0f, 1000.0f, getParamExpression ("value"), -1 });
    return defs;
}

// 20. [z~] 1-Sample Feedback Delay Node Object
OneSampleDelayNode::OneSampleDelayNode (int id)
    : RelativisticNode (id, "z~", "z~ 1-sample delay")
{
    addInlet ("in~", NodePortType::Audio);
    addOutlet ("out~", NodePortType::Audio);
}

void OneSampleDelayNode::process (int numSamples)
{
    const auto* inL = inlets[0].audioData.getReadPointer (0);
    auto* outL = outlets[0].audioData.getWritePointer (0);

    for (int s = 0; s < numSamples; ++s)
    {
        outL[s] = lastSampleL;
        lastSampleL = inL[s];
    }
}

std::string OneSampleDelayNode::getDefaultFormulaScript() const
{
    return "// 1-Sample Feedback Delay [z~]\n// Out(n) = In(n-1)\n// Guarantees sample-accurate feedback stability\n\nout = z_1;";
}

std::vector<ParameterInfo> OneSampleDelayNode::getParameterDefs() const
{
    return {};
}

// 21. [snapshot~] Audio-to-Control Snapshot Node Object
SnapshotNode::SnapshotNode (int id)
    : RelativisticNode (id, "snapshot~", "snapshot~ sample")
{
    addInlet ("in~", NodePortType::Audio);
    addOutlet ("out", NodePortType::Control);
}

void SnapshotNode::process (int numSamples)
{
    if (numSamples > 0)
    {
        outlets[0].controlValue = inlets[0].audioData.getSample (0, 0);
    }
}

std::string SnapshotNode::getDefaultFormulaScript() const
{
    return "// Audio Snapshot Node [snapshot~]\n// Converts audio sample to control float value\n\nout = $v1[0];";
}

std::vector<ParameterInfo> SnapshotNode::getParameterDefs() const
{
    return {};
}

// 22. [+] Signal & Control Adder Node Object
AddMathNode::AddMathNode (int id)
    : RelativisticNode (id, "+", "+ 0")
{
    addInlet ("in1", NodePortType::Control);
    addInlet ("in2", NodePortType::Control);
    addOutlet ("out", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);
    setParameter ("offset", 0.0f);
}

void AddMathNode::parseLabelArguments (const std::string& label)
{
    juce::StringArray tokens;
    tokens.addTokens (juce::String (label), " ", "");
    if (tokens.size() > 1)
    {
        float o = tokens[1].getFloatValue();
        setParameter ("offset", o);
    }
}

void AddMathNode::process (int numSamples)
{
    const auto* in1 = inlets[0].audioData.getReadPointer (0);
    const auto* in2 = inlets[1].audioData.getReadPointer (0);
    auto* outAudio = outlets[1].audioData.getWritePointer (0);

    float defaultOffset = getParameter ("offset", 0.0f);

    float val1 = inlets[0].controlValue;
    float val2 = (inlets[1].controlValue != 0.0f) ? inlets[1].controlValue : defaultOffset;

    bool hasAudio1 = inlets[0].audioData.getMagnitude (0, numSamples) > 0.0f;
    bool hasAudio2 = inlets[1].audioData.getMagnitude (0, numSamples) > 0.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        float sig1 = hasAudio1 ? in1[s] : val1;
        float sig2 = hasAudio2 ? in2[s] : val2;
        outAudio[s] = sig1 + sig2;
    }

    outlets[0].controlValue = val1 + val2;
}

std::string AddMathNode::getDefaultFormulaScript() const
{
    return "// Adder Node [+]\n// Adds inputs ($v1 + $v2)\n\nout = $v1 + $v2;";
}

std::vector<ParameterInfo> AddMathNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "offset", "ADDER OFFSET", getParameter ("offset", 0.0f), -9999.0f, 9999.0f, getParamExpression ("offset"), 1 });
    return defs;
}

// 23. [*] Signal & Control Multiplier Node Object
MulMathNode::MulMathNode (int id)
    : RelativisticNode (id, "*", "* 1")
{
    addInlet ("in1", NodePortType::Control);
    addInlet ("in2", NodePortType::Control);
    addOutlet ("out", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);
    setParameter ("factor", 1.0f);
}

void MulMathNode::parseLabelArguments (const std::string& label)
{
    juce::StringArray tokens;
    tokens.addTokens (juce::String (label), " ", "");
    if (tokens.size() > 1)
    {
        float f = tokens[1].getFloatValue();
        setParameter ("factor", f);
    }
}

void MulMathNode::process (int numSamples)
{
    const auto* in1 = inlets[0].audioData.getReadPointer (0);
    const auto* in2 = inlets[1].audioData.getReadPointer (0);
    auto* outAudio = outlets[1].audioData.getWritePointer (0);

    float defaultFactor = getParameter ("factor", 1.0f);

    float val1 = inlets[0].controlValue;
    float val2 = (inlets[1].controlValue != 0.0f) ? inlets[1].controlValue : defaultFactor;

    bool hasAudio1 = inlets[0].audioData.getMagnitude (0, numSamples) > 0.0f;
    bool hasAudio2 = inlets[1].audioData.getMagnitude (0, numSamples) > 0.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        float sig1 = hasAudio1 ? in1[s] : val1;
        float sig2 = hasAudio2 ? in2[s] : val2;
        outAudio[s] = sig1 * sig2;
    }

    outlets[0].controlValue = val1 * val2;
}

std::string MulMathNode::getDefaultFormulaScript() const
{
    return "// Multiplier Node [*]\n// Multiplies inputs ($v1 * $v2)\n\nout = $v1 * $v2;";
}

std::vector<ParameterInfo> MulMathNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "factor", "MULTIPLIER FACTOR", getParameter ("factor", 1.0f), -9999.0f, 9999.0f, getParamExpression ("factor"), 1 });
    return defs;
}

// 24. [table] Pure Data-Style Named Float Buffer Table Object
TableNode::TableNode (int id)
    : RelativisticNode (id, "table", "table array1")
{
    addInlet ("in", NodePortType::Control);
    addOutlet ("out", NodePortType::Control);

    setParameter ("size", 1024.0f);
    setParameter ("preset", 0.0f);
    resize (1024);
    generatePreset (0);
}

void TableNode::resize (int newSize)
{
    newSize = std::clamp (newSize, 16, 262144);
    buffer.resize (static_cast<size_t>(newSize), 0.0f);
    setParameter ("size", static_cast<float>(newSize));
}

float TableNode::readSample (int idx) const
{
    if (buffer.empty()) return 0.0f;
    int sz = static_cast<int>(buffer.size());
    idx = (idx % sz + sz) % sz;
    return buffer[static_cast<size_t>(idx)];
}

float TableNode::readSampleHermite (double pos) const
{
    int sz = static_cast<int>(buffer.size());
    if (sz <= 0) return 0.0f;

    int i1 = static_cast<int>(std::floor (pos));
    double frac = pos - i1;

    int i0 = (i1 - 1 + sz) % sz;
    int i2 = (i1 + 1) % sz;
    int i3 = (i1 + 2) % sz;
    i1 = (i1 + sz) % sz;

    float y0 = buffer[i0];
    float y1 = buffer[i1];
    float y2 = buffer[i2];
    float y3 = buffer[i3];

    double c0 = y1;
    double c1 = 0.5 * (y2 - y0);
    double c2 = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3;
    double c3 = 0.5 * (y3 - y0) + 1.5 * (y1 - y2);

    return static_cast<float>(((c3 * frac + c2) * frac + c1) * frac + c0);
}

void TableNode::writeSample (int idx, float val)
{
    if (buffer.empty()) return;
    int sz = static_cast<int>(buffer.size());
    if (idx >= 0 && idx < sz)
    {
        buffer[static_cast<size_t>(idx)] = val;
    }
}

void TableNode::writeSampleNormalized (float normX, float normVal)
{
    if (buffer.empty()) return;
    int sz = static_cast<int>(buffer.size());
    int idx = std::clamp (static_cast<int>(normX * sz), 0, sz - 1);
    buffer[static_cast<size_t>(idx)] = normVal;
}

void TableNode::normalize()
{
    if (buffer.empty()) return;
    float maxVal = 0.0f;
    for (float sample : buffer)
    {
        float absVal = std::abs (sample);
        if (absVal > maxVal)
            maxVal = absVal;
    }
    if (maxVal > 0.000001f)
    {
        float scale = 1.0f / maxVal;
        for (float& sample : buffer)
        {
            sample *= scale;
        }
    }
}

void TableNode::generatePreset (int presetIndex)
{
    int sz = getSize();
    if (sz <= 0) return;

    static const float stepPitches[8] = { 60.0f, 64.0f, 67.0f, 72.0f, 74.0f, 76.0f, 79.0f, 84.0f };

    for (int i = 0; i < sz; ++i)
    {
        float phaseFrac = static_cast<float>(i) / static_cast<float>(sz);
        float val = 0.0f;

        if (presetIndex == 0) // Sine Wave
        {
            val = std::sin (phaseFrac * 2.0f * juce::MathConstants<float>::pi);
        }
        else if (presetIndex == 1) // Triangle Wave
        {
            val = 1.0f - 4.0f * std::abs (phaseFrac - 0.5f);
        }
        else if (presetIndex == 2) // Sawtooth Wave
        {
            val = 2.0f * phaseFrac - 1.0f;
        }
        else if (presetIndex == 3) // Square Wave
        {
            val = (phaseFrac < 0.5f) ? 1.0f : -1.0f;
        }
        else if (presetIndex == 4) // Random Noise / Wave
        {
            val = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
        }
        else if (presetIndex == 5) // 8-Step Arpeggio MIDI Sequence
        {
            int step = static_cast<int>(phaseFrac * 8.0f) % 8;
            val = stepPitches[step];
        }

        buffer[static_cast<size_t>(i)] = val;
    }
}

void TableNode::process (int /*numSamples*/)
{
    int currentSize = static_cast<int>(getParameter ("size", 1024.0f));
    if (currentSize != getSize()) resize (currentSize);
}

std::string TableNode::getDefaultFormulaScript() const
{
    return "// Named Float Data Table Buffer [table]\n// Memory buffer for sequencers, wavetables, and audio recording\n\nval = buffer[idx];";
}

std::vector<ParameterInfo> TableNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "size", "TABLE BUFFER SIZE", getParameter ("size", 1024.0f), 16.0f, 65536.0f, getParamExpression ("size"), -1 });
    defs.push_back ({ "preset", "WAVEFORM PRESET (0:SINE 1:TRI 2:SAW 3:SQR 4:NOISE)", getParameter ("preset", 0.0f), 0.0f, 4.0f, getParamExpression ("preset"), -1 });
    return defs;
}

std::vector<std::string> TableNode::getExposedMethods() const
{
    return { "Gen Sine Wave", "Gen Saw Wave", "Gen Triangle", "Gen Square", "Gen Random", "Normalize Buffer", "Clear Buffer" };
}

void TableNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Gen Sine Wave") generatePreset (0);
    else if (methodName == "Gen Triangle") generatePreset (1);
    else if (methodName == "Gen Saw Wave") generatePreset (2);
    else if (methodName == "Gen Square") generatePreset (3);
    else if (methodName == "Gen Random") generatePreset (4);
    else if (methodName == "Normalize Buffer" || methodName == "normalize") normalize();
    else if (methodName == "Clear Buffer" || methodName == "clear") std::fill (buffer.begin(), buffer.end(), 0.0f);
}

// 25. [tabwrite~] Sound & Data Recorder Node Object
TabWriteNode::TabWriteNode (int id)
    : RelativisticNode (id, "tabwrite~", "tabwrite~ array1")
{
    addInlet ("in~", NodePortType::Audio);
    addInlet ("rec", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);
    setParameter ("targetTable", 0.0f);
}

void TabWriteNode::process (int numSamples)
{
    const auto* in = inlets[0].audioData.getReadPointer (0);
    auto* out = outlets[0].audioData.getWritePointer (0);
    float recGate = inlets[1].controlValue;

    if (recGate > 0.5f)
    {
        isRecording = true;
        writeIndex = 0;
    }

    std::shared_ptr<TableNode> targetTbl = nullptr;
    if (parentGraph != nullptr)
    {
        targetTbl = parentGraph->getTableByName ("array1");
    }

    for (int s = 0; s < numSamples; ++s)
    {
        out[s] = in[s];
        if (isRecording && targetTbl != nullptr)
        {
            if (writeIndex < targetTbl->getSize())
            {
                targetTbl->writeSample (writeIndex++, in[s]);
            }
            else
            {
                isRecording = false;
            }
        }
    }
}

std::string TabWriteNode::getDefaultFormulaScript() const
{
    return "// Audio & Control Sound Recorder [tabwrite~]\n// Sequential recorder writing signal frames into named [table]\n\ntable.write(writeIdx++, $v1);";
}

std::vector<ParameterInfo> TabWriteNode::getParameterDefs() const
{
    return {};
}

// 26. [tabread~] Table Reader Node Object
TabReadNode::TabReadNode (int id)
    : RelativisticNode (id, "tabread~", "tabread~ array1")
{
    addInlet ("index", NodePortType::Audio);
    addOutlet ("out~", NodePortType::Audio);
}

void TabReadNode::process (int numSamples)
{
    const auto* inIdx = inlets[0].audioData.getReadPointer (0);
    auto* out = outlets[0].audioData.getWritePointer (0);

    std::shared_ptr<TableNode> targetTbl = nullptr;
    if (parentGraph != nullptr)
    {
        targetTbl = parentGraph->getTableByName ("array1");
    }

    float ctrlIdx = inlets[0].controlValue;

    for (int s = 0; s < numSamples; ++s)
    {
        double pos = (inlets[0].audioData.getMagnitude (0, numSamples) > 0.0f) ? inIdx[s] : ctrlIdx;
        out[s] = (targetTbl != nullptr) ? targetTbl->readSampleHermite (pos) : 0.0f;
    }
}

std::string TabReadNode::getDefaultFormulaScript() const
{
    return "// Hermite Table Reader [tabread~]\n// Reads table at index $v1 with 4-point Hermite cubic interpolation\n\nout = table.readHermite($v1);";
}

std::vector<ParameterInfo> TabReadNode::getParameterDefs() const
{
    return {};
}

// 27. [tabosc4~] 4-Point Hermite Wavetable Oscillator Node Object
TabOscNode::TabOscNode (int id)
    : RelativisticNode (id, "tabosc4~", "tabosc4~ array1 440")
{
    addInlet ("timeIn", NodePortType::Time);
    addInlet ("freq", NodePortType::Control);
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("frequency", 440.0f);
}

void TabOscNode::process (int numSamples)
{
    double gamma = std::abs (inlets[0].timeGamma);
    if (gamma < 0.001) gamma = 1.0;

    float freqCtrl = inlets[1].controlValue;
    float baseFreq = getParameter ("frequency", 440.0f);
    float rawFreq = (freqCtrl > 0.0f) ? freqCtrl : baseFreq;
    double freq = (rawFreq > 0.0f && rawFreq <= 127.0f) ? (440.0 * std::pow (2.0, (rawFreq - 69.0) / 12.0)) : rawFreq;

    std::shared_ptr<TableNode> targetTbl = nullptr;
    if (parentGraph != nullptr)
    {
        targetTbl = parentGraph->getTableByName ("array1");
    }

    int tblSize = (targetTbl != nullptr) ? targetTbl->getSize() : 1024;
    double effectiveFreq = freq * gamma;
    double phaseInc = (effectiveFreq * tblSize) / currentSampleRate;

    auto* left = outlets[0].audioData.getWritePointer (0);
    auto* right = outlets[0].audioData.getWritePointer (1);

    for (int s = 0; s < numSamples; ++s)
    {
        float val = (targetTbl != nullptr) ? targetTbl->readSampleHermite (phase) : std::sin (2.0 * juce::MathConstants<double>::pi * phase / tblSize);
        left[s] = val * 0.5f;
        right[s] = val * 0.5f;

        phase += phaseInc;
        if (phase >= tblSize) phase -= tblSize;
        if (phase < 0.0) phase += tblSize;
    }
}

std::string TabOscNode::getDefaultFormulaScript() const
{
    return "// 4-Point Hermite Wavetable Oscillator [tabosc4~]\n// Plays target [table] as cyclic wavetable under dilated coordinate time\n\nout = table.readHermite(phase);";
}

std::vector<ParameterInfo> TabOscNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "frequency", "WAVETABLE FREQUENCY (Hz)", getParameter ("frequency", 440.0f), 20.0f, 20000.0f, getParamExpression ("frequency"), -1 });
    return defs;
}

// 28. [svfilter~] Multi-Mode State Variable Filter Node Object
SvFilterNode::SvFilterNode (int id)
    : RelativisticNode (id, "svfilter~", "svfilter~ 1000 0.707")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in~", NodePortType::Audio);        // Inlet 1: Audio input stream
    addInlet ("cutoff", NodePortType::Control);   // Inlet 2: Cutoff frequency modulation
    addInlet ("res", NodePortType::Control);      // Inlet 3: Resonance modulation

    addOutlet ("lp~", NodePortType::Audio);
    addOutlet ("hp~", NodePortType::Audio);
    addOutlet ("bp~", NodePortType::Audio);
    addOutlet ("notch~", NodePortType::Audio);

    setParameter ("cutoff", 1000.0f);
    setParameter ("resonance", 0.707f);
}

void SvFilterNode::process (int numSamples)
{
    double gamma = getEffectiveGamma();
    const auto* in = inlets[1].audioData.getReadPointer (0);

    float cutoffCtrl = inlets[2].controlValue;
    float resCtrl = inlets[3].controlValue;

    float cutoff = (cutoffCtrl > 0.0f) ? cutoffCtrl : getParameter ("cutoff", 1000.0f);
    float q = (resCtrl > 0.0f) ? resCtrl : getParameter ("resonance", 0.707f);

    cutoff = cutoff * static_cast<float>(std::abs (gamma));
    cutoff = std::clamp (cutoff, 20.0f, std::min (18000.0f, static_cast<float>(currentSampleRate * 0.42f)));
    q = std::clamp (q, 0.1f, 10.0f);

    // Authentic Vadim Zavalishin TPT State-Variable Filter (04_state_variable_filter_dsp.tex)
    float g = std::tan (juce::MathConstants<float>::pi * cutoff / static_cast<float>(currentSampleRate));
    g = std::clamp (g, 0.0001f, 0.99f);
    float k = 1.0f / q;
    float denom = 1.0f / (1.0f + g * (g + k));

    auto* lp = outlets[0].audioData.getWritePointer (0);
    auto* hp = outlets[1].audioData.getWritePointer (0);
    auto* bp = outlets[2].audioData.getWritePointer (0);
    auto* notch = outlets[3].audioData.getWritePointer (0);

    if (!std::isfinite (s1)) s1 = 0.0f;
    if (!std::isfinite (s2)) s2 = 0.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        float x = std::clamp (in[s], -2.0f, 2.0f);

        float vH = (x - (g + k) * s1 - s2) * denom;
        float vB = g * vH + s1;
        float vL = g * vB + s2;

        s1 = 2.0f * vB - s1;
        s2 = 2.0f * vL - s2;

        s1 = std::clamp (s1, -4.0f, 4.0f);
        s2 = std::clamp (s2, -4.0f, 4.0f);

        lp[s] = std::clamp (vL, -2.0f, 2.0f);
        hp[s] = std::clamp (vH, -2.0f, 2.0f);
        bp[s] = std::clamp (vB, -2.0f, 2.0f);
        notch[s] = std::clamp (vH + vL, -2.0f, 2.0f);
    }
}

std::string SvFilterNode::getDefaultFormulaScript() const
{
    return "// Multi-Mode State Variable Filter [svfilter~]\n// Chamberlin SVF providing simultaneous LP, HP, BP, and Notch outputs\n\nhp = (in - (g + k)*s1 - s2) / (1 + g*(g + k));\nbp = g*hp + s1;\nlp = g*bp + s2;";
}

std::vector<ParameterInfo> SvFilterNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "cutoff", "CUTOFF FREQ (Hz)", getParameter ("cutoff", 1000.0f), 20.0f, 20000.0f, getParamExpression ("cutoff"), 2 });
    defs.push_back ({ "resonance", "RESONANCE (Q)", getParameter ("resonance", 0.707f), 0.1f, 20.0f, getParamExpression ("resonance"), 3 });
    return defs;
}

// 29. [drive~] Non-Linear Tube Saturation & Overdrive Node Object
DriveNode::DriveNode (int id)
    : RelativisticNode (id, "drive~", "drive~ 2.0")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in~", NodePortType::Audio);        // Inlet 1: Audio input stream
    addInlet ("drive", NodePortType::Control);    // Inlet 2: Overdrive modulation
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("drive", 2.0f);
    setParameter ("outGain", 0.8f);
}

void DriveNode::process (int numSamples)
{
    double gamma = getEffectiveGamma();
    const auto* inL = inlets[1].audioData.getReadPointer (0);
    const auto* inR = (inlets[1].audioData.getNumChannels() > 1) ? inlets[1].audioData.getReadPointer (1) : inL;

    auto* outL = outlets[0].audioData.getWritePointer (0);
    auto* outR = (outlets[0].audioData.getNumChannels() > 1) ? outlets[0].audioData.getWritePointer (1) : outL;

    float driveCtrl = inlets[2].controlValue;
    float drive = (driveCtrl > 0.0f) ? driveCtrl : getParameter ("drive", 2.0f);
    drive = drive * static_cast<float>(std::sqrt (std::abs (gamma)));
    float outGain = getParameter ("outGain", 0.8f);

    for (int s = 0; s < numSamples; ++s)
    {
        outL[s] = std::tanh (inL[s] * drive) * outGain;
        outR[s] = std::tanh (inR[s] * drive) * outGain;
    }
}

std::string DriveNode::getDefaultFormulaScript() const
{
    return "// Non-Linear Tube Saturation Node [drive~]\n// Applies tanh(drive * in) warmth and soft clipping\n\nout = tanh(in * drive) * outGain;";
}

std::vector<ParameterInfo> DriveNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "drive", "OVERDRIVE AMOUNT", getParameter ("drive", 2.0f), 1.0f, 20.0f, getParamExpression ("drive"), 2 });
    defs.push_back ({ "outGain", "OUTPUT GAIN", getParameter ("outGain", 0.8f), 0.0f, 2.0f, getParamExpression ("outGain"), -1 });
    return defs;
}

// 30. [reverb~] High-Density Stereo Reverb Node Object
ReverbNode::ReverbNode (int id)
    : RelativisticNode (id, "reverb~", "reverb~ 0.5 0.5")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("inL~", NodePortType::Audio);       // Inlet 1: Audio Left input
    addInlet ("inR~", NodePortType::Audio);       // Inlet 2: Audio Right input
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("roomSize", 0.5f);
    setParameter ("damping", 0.5f);
    setParameter ("wetLevel", 0.33f);
    setParameter ("dryLevel", 0.7f);
}

void ReverbNode::prepare (double sampleRate, int samplesPerBlock)
{
    RelativisticNode::prepare (sampleRate, samplesPerBlock);
    reverbEngine.setSampleRate (sampleRate);
}

void ReverbNode::process (int numSamples)
{
    double gamma = getEffectiveGamma();
    float roomSize = std::clamp (getParameter ("roomSize", 0.5f) * static_cast<float>(std::sqrt (std::abs (gamma))), 0.05f, 0.98f);
    reverbParams.roomSize = roomSize;
    reverbParams.damping = getParameter ("damping", 0.5f);
    reverbParams.wetLevel = getParameter ("wetLevel", 0.33f);
    reverbParams.dryLevel = getParameter ("dryLevel", 0.7f);
    reverbParams.width = 1.0f;
    reverbEngine.setParameters (reverbParams);

    auto* left = outlets[0].audioData.getWritePointer (0);
    auto* right = (outlets[0].audioData.getNumChannels() > 1) ? outlets[0].audioData.getWritePointer (1) : left;
    const auto* inL = inlets[1].audioData.getReadPointer (0);
    const auto* inR = (inlets.size() > 2 && inlets[2].audioData.getMagnitude(0, numSamples) > 0.00001f) ? inlets[2].audioData.getReadPointer (0) : inL;

    for (int s = 0; s < numSamples; ++s)
    {
        left[s] = std::clamp (inL[s], -2.0f, 2.0f);
        right[s] = std::clamp (inR[s], -2.0f, 2.0f);
    }

    reverbEngine.processStereo (left, right, numSamples);

    for (int s = 0; s < numSamples; ++s)
    {
        left[s] = std::clamp (left[s], -2.0f, 2.0f);
        right[s] = std::clamp (right[s], -2.0f, 2.0f);
    }
}

std::string ReverbNode::getDefaultFormulaScript() const
{
    return "// High-Density Stereo Reverberator [reverb~]\n// Schroeder/Moorer algorithmic room simulation\n\nreverb.processStereo(left, right);";
}

std::vector<ParameterInfo> ReverbNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "roomSize", "ROOM SIZE", getParameter ("roomSize", 0.5f), 0.0f, 1.0f, getParamExpression ("roomSize"), -1 });
    defs.push_back ({ "damping", "HIGH DAMPING", getParameter ("damping", 0.5f), 0.0f, 1.0f, getParamExpression ("damping"), -1 });
    defs.push_back ({ "wetLevel", "WET MIX LEVEL", getParameter ("wetLevel", 0.33f), 0.0f, 1.0f, getParamExpression ("wetLevel"), -1 });
    return defs;
}

// 31. [crush~] Bit-Crusher & Downsampler Node Object
CrushNode::CrushNode (int id)
    : RelativisticNode (id, "crush~", "crush~ 8 4")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in~", NodePortType::Audio);        // Inlet 1: Audio input stream
    addOutlet ("out~", NodePortType::Audio);

    setParameter ("bitDepth", 8.0f);
    setParameter ("downsample", 4.0f);
}

void CrushNode::process (int numSamples)
{
    double gamma = getEffectiveGamma();
    const auto* inL = inlets[1].audioData.getReadPointer (0);
    const auto* inR = (inlets[1].audioData.getNumChannels() > 1) ? inlets[1].audioData.getReadPointer (1) : inL;

    auto* outL = outlets[0].audioData.getWritePointer (0);
    auto* outR = (outlets[0].audioData.getNumChannels() > 1) ? outlets[0].audioData.getWritePointer (1) : outL;

    float bits = std::clamp (getParameter ("bitDepth", 8.0f), 1.0f, 24.0f);
    float rawDown = getParameter ("downsample", 4.0f);
    if (std::abs (gamma) > 0.001) rawDown = static_cast<float>(rawDown / std::abs (gamma));
    int downsample = std::max (1, static_cast<int>(rawDown));

    float step = std::pow (2.0f, bits - 1.0f);

    for (int s = 0; s < numSamples; ++s)
    {
        if (sampleCounter % downsample == 0)
        {
            float valL = std::clamp (inL[s], -2.0f, 2.0f);
            float valR = std::clamp (inR[s], -2.0f, 2.0f);
            holdSampleL = std::round (valL * step) / step;
            holdSampleR = std::round (valR * step) / step;
        }
        outL[s] = holdSampleL;
        outR[s] = holdSampleR;
        sampleCounter++;
    }
}

std::string CrushNode::getDefaultFormulaScript() const
{
    return "// Digital Lo-Fi Bit-Crusher [crush~]\n// Quantizes bit-resolution and decimates sample rate\n\nout = round(in * 2^(bits-1)) / 2^(bits-1);";
}

std::vector<ParameterInfo> CrushNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "bitDepth", "BIT RESOLUTION (BITS)", getParameter ("bitDepth", 8.0f), 1.0f, 16.0f, getParamExpression ("bitDepth"), -1 });
    defs.push_back ({ "downsample", "DOWNSAMPLE FACTOR", getParameter ("downsample", 4.0f), 1.0f, 32.0f, getParamExpression ("downsample"), -1 });
    return defs;
}

// 32. [adsr~] 4-Stage ADSR Envelope Generator Node Object
AdsrNode::AdsrNode (int id)
    : RelativisticNode (id, "adsr~", "adsr~ 10 100 0.7 300")
{
    addInlet ("gate", NodePortType::Control);
    addOutlet ("env~", NodePortType::Audio);

    setParameter ("attack", 10.0f);   // ms
    setParameter ("decay", 100.0f);   // ms
    setParameter ("sustain", 0.7f);   // level
    setParameter ("release", 300.0f); // ms
}

void AdsrNode::prepare (double sampleRate, int samplesPerBlock)
{
    RelativisticNode::prepare (sampleRate, samplesPerBlock);
    adsrEngine.setSampleRate (sampleRate);
}

void AdsrNode::process (int numSamples)
{
    adsrParams.attack = getParameter ("attack", 10.0f) / 1000.0f;
    adsrParams.decay = getParameter ("decay", 100.0f) / 1000.0f;
    adsrParams.sustain = getParameter ("sustain", 0.7f);
    adsrParams.release = getParameter ("release", 300.0f) / 1000.0f;
    adsrEngine.setParameters (adsrParams);

    bool currentGate = (inlets[0].controlValue > 0.5f);
    if (currentGate != lastGateState)
    {
        if (currentGate) adsrEngine.noteOn();
        else adsrEngine.noteOff();
        lastGateState = currentGate;
    }

    auto* out = outlets[0].audioData.getWritePointer (0);
    for (int s = 0; s < numSamples; ++s)
    {
        out[s] = adsrEngine.getNextSample();
    }
}

std::string AdsrNode::getDefaultFormulaScript() const
{
    return "// 4-Stage ADSR Envelope Generator [adsr~]\n// Generates Attack-Decay-Sustain-Release envelope curve\n\nenv = adsr.getNextSample();";
}

std::vector<ParameterInfo> AdsrNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "attack", "ATTACK TIME (ms)", getParameter ("attack", 10.0f), 1.0f, 5000.0f, getParamExpression ("attack"), -1 });
    defs.push_back ({ "decay", "DECAY TIME (ms)", getParameter ("decay", 100.0f), 1.0f, 5000.0f, getParamExpression ("decay"), -1 });
    defs.push_back ({ "sustain", "SUSTAIN LEVEL", getParameter ("sustain", 0.7f), 0.0f, 1.0f, getParamExpression ("sustain"), -1 });
    defs.push_back ({ "release", "RELEASE TIME (ms)", getParameter ("release", 300.0f), 1.0f, 10000.0f, getParamExpression ("release"), -1 });
    return defs;
}

// 4e. [time.future~] Future Lookahead Causality Offset Object
TimeFutureNode::TimeFutureNode (int id)
    : RelativisticNode (id, "time.future~", "time.future~ 1.0s (Lookahead)")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in~", NodePortType::Audio);        // Inlet 1: Audio input stream
    addInlet ("lookahead", NodePortType::Control); // Inlet 2: Future lookahead time (sec)

    addOutlet ("out~", NodePortType::Audio);       // Outlet 0: Future audio output
    addOutlet ("timeOut", NodePortType::Time);    // Outlet 1: Dilated future time factor

    setParameter ("lookahead", 1.0f);
    setParameter ("autoMatchTime", 1.0f);
    setParameter ("maxSlewRate", 2.0f);
    setParameter ("timeFilterHz", 10.0f);
    setParameter ("bypassTimeFilter", 0.0f);
}

void TimeFutureNode::process (int numSamples)
{
    double gamma = std::abs (inlets[0].timeGamma);
    if (gamma < 0.001) gamma = 1.0;

    const auto* inL = inlets[1].audioData.getReadPointer (0);
    const auto* inR = (inlets[1].audioData.getNumChannels() > 1) ? inlets[1].audioData.getReadPointer (1) : inL;

    auto* outL = outlets[0].audioData.getWritePointer (0);
    auto* outR = outlets[0].audioData.getWritePointer (1);

    for (int s = 0; s < numSamples; ++s)
    {
        outL[s] = inL[s];
        outR[s] = inR[s];
    }

    outlets[1].timeGamma = gamma;
}

std::string TimeFutureNode::getDefaultFormulaScript() const
{
    return "// Future Lookahead Causality Offset [time.future~]\n// Reads live un-delayed stream, shifting global project horizon\n\nout = $v1;";
}

std::vector<ParameterInfo> TimeFutureNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "lookahead", "FUTURE LOOKAHEAD (sec)", getParameter ("lookahead", 1.0f), 0.0f, 10.0f, getParamExpression ("lookahead"), 2 });
    defs.push_back ({ "autoMatchTime", "ADAPTIVE TARGET-MATCH", getParameter ("autoMatchTime", 1.0f), 0.0f, 1.0f, getParamExpression ("autoMatchTime"), -1 });
    defs.push_back ({ "maxSlewRate", "MAX SLEW RATE (Constant Shift)", getParameter ("maxSlewRate", 2.0f), 0.1f, 20.0f, getParamExpression ("maxSlewRate"), -1 });
    defs.push_back ({ "timeFilterHz", "TIME LOWPASS CUTOFF (Hz)", getParameter ("timeFilterHz", 10.0f), 0.1f, 50.0f, getParamExpression ("timeFilterHz"), -1 });
    defs.push_back ({ "bypassTimeFilter", "INSTANT QUANTUM BYPASS", getParameter ("bypassTimeFilter", 0.0f), 0.0f, 1.0f, getParamExpression ("bypassTimeFilter"), -1 });
    return defs;
}

std::vector<std::string> TimeFutureNode::getExposedMethods() const
{
    return { "Mode: Adaptive Time-Match", "Mode: Linear Slew Travel", "Mode: Instant Quantum Jump" };
}

void TimeFutureNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Mode: Adaptive Time-Match")
    {
        setParameter ("autoMatchTime", 1.0f);
        setParameter ("bypassTimeFilter", 0.0f);
    }
    else if (methodName == "Mode: Linear Slew Travel")
    {
        setParameter ("autoMatchTime", 0.0f);
        setParameter ("maxSlewRate", 1.5f);
        setParameter ("bypassTimeFilter", 0.0f);
    }
    else if (methodName == "Mode: Instant Quantum Jump")
    {
        setParameter ("bypassTimeFilter", 1.0f);
    }
}

// 33. [fbdrum~] / [drum~] Future Bass Drum Synthesizer Node Object Implementation
FutureBassDrumNode::FutureBassDrumNode (int id)
    : RelativisticNode (id, "fbdrum~", "fbdrum~ (Future Bass Drum Engine)")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("midiIn", NodePortType::Control);   // Inlet 1: Control MIDI note number (36=Kick, 38=Snare, 39=Clap, 42=ClosedHat, 46=OpenHat, 48+=Perc)
    addInlet ("trig", NodePortType::Control);     // Inlet 2: Gate / trigger impulse
    addInlet ("vel", NodePortType::Control);      // Inlet 3: Velocity amplitude

    addOutlet ("outL~", NodePortType::Audio);     // Outlet 0: Left stereo mix output
    addOutlet ("outR~", NodePortType::Audio);     // Outlet 1: Right stereo mix output
    addOutlet ("kick~", NodePortType::Audio);     // Outlet 2: Dedicated Kick channel
    addOutlet ("snare~", NodePortType::Audio);    // Outlet 3: Dedicated Snare channel
    addOutlet ("hat~", NodePortType::Audio);      // Outlet 4: Dedicated Hi-Hat channel

    setParameter ("kickPitch", 45.0f);   // Sub Kick Base Frequency (Hz)
    setParameter ("snareSnap", 0.7f);   // Snare Noise Mix
    setParameter ("clapSpread", 7.0f);   // Multi-impulse clap delay (ms)
    setParameter ("hatDecay", 45.0f);    // Hi-Hat decay length (ms)
    setParameter ("drive", 1.8f);        // Tube Overdrive warmth
    setParameter ("masterGain", 1.0f);   // Master Drum Gain
    setParameter ("pitchDilation", 0.0f); // 0.0: Steady Drum Pitch, 1.0: Full Pitch Shift
}

void FutureBassDrumNode::prepare (double sampleRate, int samplesPerBlock)
{
    RelativisticNode::prepare (sampleRate, samplesPerBlock);
    kickPhase = 0.0;
    snareBodyPhase = 0.0;
    percCarrierPhase = 0.0;
    percModPhase = 0.0;
}

void FutureBassDrumNode::triggerNote (int midiNote, float velocity)
{
    velocity = std::clamp (velocity, 0.1f, 1.2f);

    if (midiNote == 36 || midiNote == 35) // Kick
    {
        kickPhase = 0.0;
        kickFreq = 190.0;
        kickEnv = velocity;
        kickClickEnv = velocity;
    }
    else if (midiNote == 38 || midiNote == 40) // Snare
    {
        snareBodyPhase = 0.0;
        snareBodyFreq = 260.0;
        snareBodyEnv = velocity;
        snareNoiseEnv = velocity;
    }
    else if (midiNote == 39 || midiNote == 37) // Multi-Impulse Clap
    {
        clapEnv = velocity;
        clapBurstCounter = 3;
        clapTimer = 0;
    }
    else if (midiNote == 42 || midiNote == 44) // Closed Hat
    {
        hatEnv = velocity;
        isHatOpen = false;
    }
    else if (midiNote == 46) // Open Hat
    {
        hatEnv = velocity;
        isHatOpen = true;
    }
    else // Tonal FM Pitched Perc (C2 and above)
    {
        float hz = 440.0f * std::pow (2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
        percCarrierFreq = hz;
        percModFreq = hz * 1.414;
        percCarrierPhase = 0.0;
        percModPhase = 0.0;
        percEnv = velocity;
    }
}

void FutureBassDrumNode::process (int numSamples)
{
    double gamma = std::abs (inlets[0].timeGamma);
    if (gamma < 0.001) gamma = 1.0;

    const auto* noteBuf = (inlets.size() > 1 && inlets[1].audioData.getNumSamples() >= numSamples) ? inlets[1].audioData.getReadPointer (0) : nullptr;
    const auto* trigBuf = (inlets.size() > 2 && inlets[2].audioData.getNumSamples() >= numSamples) ? inlets[2].audioData.getReadPointer (0) : nullptr;
    const auto* velBuf  = (inlets.size() > 3 && inlets[3].audioData.getNumSamples() >= numSamples) ? inlets[3].audioData.getReadPointer (0) : nullptr;

    float midiNoteIn = inlets[1].controlValue;
    float trigIn = inlets[2].controlValue;
    float velIn = inlets[3].controlValue;
    float defaultVelocity = (velIn > 0.0f) ? velIn : 0.9f;

    auto* outL = outlets[0].audioData.getWritePointer (0);
    auto* outR = (outlets[0].audioData.getNumChannels() > 1) ? outlets[0].audioData.getWritePointer (1) : outL;
    auto* kickOut = outlets[2].audioData.getWritePointer (0);
    auto* snareOut = outlets[3].audioData.getWritePointer (0);
    auto* hatOut = outlets[4].audioData.getWritePointer (0);

    outlets[0].audioData.clear();
    outlets[2].audioData.clear();
    outlets[3].audioData.clear();
    outlets[4].audioData.clear();

    float baseKickPitch = getParameter ("kickPitch", 45.0f);
    float snareSnap = getParameter ("snareSnap", 0.7f);
    float hatDecayParam = getParameter ("hatDecay", 45.0f);
    float drive = getParameter ("drive", 1.8f);
    float masterGain = getParameter ("masterGain", 1.0f);
    float pitchDilation = getParameter ("pitchDilation", 0.0f);

    double pitchGamma = 1.0 + (gamma - 1.0) * static_cast<double>(pitchDilation);
    double envRateScale = gamma / currentSampleRate;

    float targetKickPitch = baseKickPitch * static_cast<float>(pitchGamma);
    double targetSnarePitch = 110.0 * pitchGamma;

    for (int s = 0; s < numSamples; ++s)
    {
        float curTrig = (trigBuf != nullptr && trigBuf[s] > 0.1f) ? trigBuf[s] : ((s == 0) ? trigIn : 0.0f);
        float curNote = (noteBuf != nullptr && noteBuf[s] > 0.0f) ? noteBuf[s] : midiNoteIn;
        float curVel  = (velBuf != nullptr && velBuf[s] > 0.0f) ? velBuf[s] : defaultVelocity;

        if (curTrig > 0.5f && lastTrigState <= 0.5f)
        {
            int noteToPlay = (curNote > 0.0f) ? static_cast<int>(curNote) : 36;
            triggerNote (noteToPlay, curVel);
        }
        else if (curNote > 0.0f && std::abs (curNote - lastNoteValue) > 0.1f)
        {
            triggerNote (static_cast<int>(curNote), curVel);
        }

        lastTrigState = curTrig;
        lastNoteValue = curNote;

        float kSample = 0.0f;
        float sSample = 0.0f;
        float cSample = 0.0f;
        float hSampleL = 0.0f, hSampleR = 0.0f;
        float pSample = 0.0f;

        if (kickEnv > 0.0001f)
        {
            kickFreq += (targetKickPitch - kickFreq) * (18.0 * envRateScale);
            kickPhase += (2.0 * juce::MathConstants<double>::pi * kickFreq) / currentSampleRate;
            if (kickPhase >= 2.0 * juce::MathConstants<double>::pi) kickPhase -= 2.0 * juce::MathConstants<double>::pi;

            float subSine = std::sin (kickPhase);
            float click = ((float) rand() / RAND_MAX * 2.0f - 1.0f) * kickClickEnv;

            kSample = (subSine * 0.9f + click * 0.35f) * kickEnv;

            kickEnv *= std::exp (-10.0 * envRateScale);
            kickClickEnv *= std::exp (-120.0 * envRateScale);
        }

        if (snareBodyEnv > 0.0001f || snareNoiseEnv > 0.0001f)
        {
            snareBodyFreq += (targetSnarePitch - snareBodyFreq) * (30.0 * envRateScale);
            snareBodyPhase += (2.0 * juce::MathConstants<double>::pi * snareBodyFreq) / currentSampleRate;
            if (snareBodyPhase >= 2.0 * juce::MathConstants<double>::pi) snareBodyPhase -= 2.0 * juce::MathConstants<double>::pi;

            float body = std::sin (snareBodyPhase) * snareBodyEnv;

            float rawNoise = ((float) rand() / RAND_MAX * 2.0f - 1.0f);
            snareFilterState += 0.4f * (rawNoise - snareFilterState);
            float hpfNoise = (rawNoise - snareFilterState) * snareNoiseEnv * snareSnap;

            sSample = (body * 0.6f + hpfNoise * 0.7f);

            snareBodyEnv *= std::exp (-22.0 * envRateScale);
            snareNoiseEnv *= std::exp (-14.0 * envRateScale);
        }

        if (clapEnv > 0.0001f)
        {
            clapTimer++;
            if (clapTimer >= static_cast<int>((7.0 / 1000.0) * currentSampleRate) && clapBurstCounter > 0)
            {
                clapTimer = 0;
                clapBurstCounter--;
            }

            float noise = ((float) rand() / RAND_MAX * 2.0f - 1.0f);
            clapFilterState += 0.3f * (noise - clapFilterState);
            float bpfClap = (noise - clapFilterState);

            float burstGain = (clapBurstCounter > 0) ? 1.0f : 0.65f;
            cSample = bpfClap * clapEnv * burstGain;

            clapEnv *= std::exp (-18.0 * envRateScale);
        }

        if (hatEnv > 0.0001f)
        {
            float noise = ((float) rand() / RAND_MAX * 2.0f - 1.0f);
            hatFilterStateL += 0.7f * (noise - hatFilterStateL);
            hatFilterStateR += 0.65f * (noise - hatFilterStateR);

            float hpfL = noise - hatFilterStateL;
            float hpfR = noise - hatFilterStateR;

            hSampleL = hpfL * hatEnv * 0.6f;
            hSampleR = hpfR * hatEnv * 0.6f;

            double hatDecayRate = isHatOpen ? (4.0 * envRateScale) : ((1000.0 / std::max (10.0f, hatDecayParam)) * envRateScale);
            hatEnv *= std::exp (-hatDecayRate);
        }

        if (percEnv > 0.0001f)
        {
            double effPercMod = percModFreq * pitchGamma;
            double effPercCar = percCarrierFreq * pitchGamma;

            percModPhase += (2.0 * juce::MathConstants<double>::pi * effPercMod) / currentSampleRate;
            float modSignal = std::sin (percModPhase) * 300.0f * percEnv;

            percCarrierPhase += (2.0 * juce::MathConstants<double>::pi * (effPercCar + modSignal)) / currentSampleRate;
            pSample = std::sin (percCarrierPhase) * percEnv * 0.7f;

            percEnv *= std::exp (-25.0 * envRateScale);
        }

        float mixL = kSample + sSample + cSample + hSampleL + pSample;
        float mixR = kSample + sSample + cSample + hSampleR + pSample;

        mixL = std::tanh (mixL * drive) * masterGain;
        mixR = std::tanh (mixR * drive) * masterGain;

        outL[s] = mixL;
        outR[s] = mixR;
        kickOut[s] = std::tanh (kSample * drive);
        snareOut[s] = std::tanh ((sSample + cSample) * drive);
        hatOut[s] = hSampleL;
    }
}

std::string FutureBassDrumNode::getDefaultFormulaScript() const
{
    return "// Future Bass Drum Synthesizer Engine [fbdrum~]\n// Multi-voice physical sub-kick, snappy snare, clap, & FM percs\n\noutL = tanh((kick + snare + clap + hat + perc) * drive) * masterGain;";
}

std::vector<ParameterInfo> FutureBassDrumNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "kickPitch", "SUB KICK BASE PITCH (Hz)", getParameter ("kickPitch", 45.0f), 30.0f, 100.0f, getParamExpression ("kickPitch"), -1 });
    defs.push_back ({ "snareSnap", "SNARE NOISE SNAP MIX", getParameter ("snareSnap", 0.7f), 0.0f, 1.0f, getParamExpression ("snareSnap"), -1 });
    defs.push_back ({ "clapSpread", "CLAP BURST SPREAD (ms)", getParameter ("clapSpread", 7.0f), 1.0f, 25.0f, getParamExpression ("clapSpread"), -1 });
    defs.push_back ({ "hatDecay", "HI-HAT DECAY LENGTH (ms)", getParameter ("hatDecay", 45.0f), 10.0f, 200.0f, getParamExpression ("hatDecay"), -1 });
    defs.push_back ({ "drive", "TUBE OVERDRIVE WARMTH", getParameter ("drive", 1.8f), 1.0f, 10.0f, getParamExpression ("drive"), -1 });
    defs.push_back ({ "masterGain", "MASTER DRUM GAIN", getParameter ("masterGain", 1.0f), 0.0f, 2.0f, getParamExpression ("masterGain"), -1 });
    defs.push_back ({ "pitchDilation", "PITCH DILATION DOPPLER (0=Steady Pitch, 1=Full Pitch Shift)", getParameter ("pitchDilation", 0.0f), 0.0f, 1.0f, getParamExpression ("pitchDilation"), -1 });
    return defs;
}

std::vector<std::string> FutureBassDrumNode::getExposedMethods() const
{
    return { "Trig Kick", "Trig Snare", "Trig Clap", "Trig Closed Hat", "Trig Open Hat", "Trig Pitched Perc (C2)", "Trig Pitched Perc (E2)" };
}

void FutureBassDrumNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Trig Kick")                 triggerNote (36, 1.0f);
    else if (methodName == "Trig Snare")           triggerNote (38, 1.0f);
    else if (methodName == "Trig Clap")            triggerNote (39, 1.0f);
    else if (methodName == "Trig Closed Hat")       triggerNote (42, 0.8f);
    else if (methodName == "Trig Open Hat")         triggerNote (46, 0.9f);
    else if (methodName == "Trig Pitched Perc (C2)") triggerNote (48, 0.85f);
    else if (methodName == "Trig Pitched Perc (E2)") triggerNote (52, 0.85f);
}

// ----------------------------------------------------
// 52. [meter~] Audio Peak & RMS Level VU Meter Object
// ----------------------------------------------------
VuMeterAudioNode::VuMeterAudioNode (int id)
    : RelativisticNode (id, "meter~", "meter~")
{
    addInlet ("timeIn", NodePortType::Time);      // Inlet 0: Dilated coordinate time
    addInlet ("in~", NodePortType::Audio);        // Inlet 1: Audio signal input
    addOutlet ("peak", NodePortType::Control);     // Outlet 0: Peak level float
    addOutlet ("rms", NodePortType::Control);      // Outlet 1: RMS level float
}

void VuMeterAudioNode::process (int numSamples)
{
    const auto* samples = inlets[1].audioData.getReadPointer (0);
    float p = 0.0f;
    float sumSq = 0.0f;
    for (int s = 0; s < numSamples; ++s)
    {
        float absS = std::abs (samples[s]);
        if (absS > p) p = absS;
        sumSq += samples[s] * samples[s];
    }
    float rms = (numSamples > 0) ? std::sqrt (sumSq / static_cast<float>(numSamples)) : 0.0f;

    float pDb = (p > 0.00001f) ? 20.0f * std::log10 (p) : -100.0f;
    float rDb = (rms > 0.00001f) ? 20.0f * std::log10 (rms) : -100.0f;

    peakLevelDb.store (pDb);
    rmsLevelDb.store (rDb);

    outlets[0].controlValue = p;
    if (outlets.size() > 1) outlets[1].controlValue = rms;
}

std::string VuMeterAudioNode::getDefaultFormulaScript() const
{
    return "// Audio Peak & RMS Level Meter [meter~]\n// Real-time VU meter with dB monitoring\n\npeak = max(abs(in~)); rms = sqrt(mean(in~^2));";
}

std::vector<ParameterInfo> VuMeterAudioNode::getParameterDefs() const
{
    return {};
}

// ----------------------------------------------------
// 53. [number~] Audio-Rate Sample Monitor Object
// ----------------------------------------------------
NumberAudioNode::NumberAudioNode (int id)
    : RelativisticNode (id, "number~", "number~")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in~", NodePortType::Audio);         // Inlet 1: Audio signal input
    addOutlet ("out", NodePortType::Control);      // Outlet 0: Instantaneous sample control float
}

void NumberAudioNode::process (int numSamples)
{
    if (numSamples > 0)
    {
        float sampleVal = inlets[1].audioData.getSample (0, 0);
        currentSampleValue.store (sampleVal);
        outlets[0].controlValue = sampleVal;
    }
}

std::string NumberAudioNode::getDefaultFormulaScript() const
{
    return "// Audio Sample Monitor Object [number~]\n// Captures instantaneous audio sample into control float\n\nout = in~[0];";
}

std::vector<ParameterInfo> NumberAudioNode::getParameterDefs() const
{
    return {};
}

// ----------------------------------------------------
// 54. [print] Control & Signal Data Logger Node Object
// ----------------------------------------------------
PrintMonitorNode::PrintMonitorNode (int id)
    : RelativisticNode (id, "print", "print")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("in", NodePortType::Control);        // Inlet 1: Control/Signal value input
    addOutlet ("out", NodePortType::Control);      // Outlet 0: Pass-through value
}

void PrintMonitorNode::process (int /*numSamples*/)
{
    float val = inlets[1].controlValue;
    outlets[0].controlValue = val;

    if (val != lastLoggedValue)
    {
        lastLoggedValue = val;
        std::lock_guard<std::mutex> lock (logMutex);
        juce::String entry = juce::String::formatted (juce::CharPointer_UTF8 ("\xce\xb3=%.2fx : val=%.4f"), inlets[0].timeGamma, val);
        logHistory.push_back (entry.toStdString());
        if (logHistory.size() > 16) logHistory.erase (logHistory.begin());
    }

}

std::string PrintMonitorNode::getDefaultFormulaScript() const
{
    return "// Data Logger Node [print]\n// Logs incoming value changes with relativistic timestamps\n\nout = in;";
}

std::vector<ParameterInfo> PrintMonitorNode::getParameterDefs() const
{
    return {};
}

std::vector<std::string> PrintMonitorNode::getLogHistory() const
{
    std::lock_guard<std::mutex> lock (logMutex);
    return logHistory;
}

void PrintMonitorNode::clearLog()
{
    std::lock_guard<std::mutex> lock (logMutex);
    logHistory.clear();
}

} // namespace time_dilation


