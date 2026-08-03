#include "RelativisticSequencers.h"
#include <sstream>
#include <cmath>
#include <algorithm>
#include <regex>

namespace time_dilation
{

// Helper to parse MIDI note names ("C4", "E4", "G4", "60", "64")
static float parseNoteOrFloat (const std::string& token)
{
    if (token.empty()) return 60.0f;
    try {
        if (std::isdigit (token[0]) || token[0] == '-' || token[0] == '+')
            return std::stof (token);
    } catch (...) {}

    // Parse note string e.g. C4, D#4, Eb4
    static const std::string names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    std::string upper = token;
    for (auto& c : upper) c = (char) std::toupper (c);

    int octave = 4;
    int noteIdx = 0;
    for (int i = 0; i < 12; ++i)
    {
        if (upper.rfind (names[i], 0) == 0)
        {
            noteIdx = i;
            std::string octStr = upper.substr (names[i].length());
            if (!octStr.empty())
            {
                try { octave = std::stoi (octStr); } catch (...) {}
            }
            return static_cast<float>((octave + 1) * 12 + noteIdx);
        }
    }
    return 60.0f;
}

// ----------------------------------------------------
// StepSequencerNode Implementation
// ----------------------------------------------------
StepSequencerNode::StepSequencerNode (int id)
    : RelativisticNode (id, "seq", "seq (Pattern Sequencer)")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("trig", NodePortType::Control);     // Inlet 1: External clock / step trigger
    addInlet ("bpm", NodePortType::Control);      // Inlet 2: Tempo modulation

    addOutlet ("pitch", NodePortType::Control);   // Outlet 0: Output MIDI pitch / value
    addOutlet ("gate~", NodePortType::Audio);     // Outlet 1: Audio gate pulse
    addOutlet ("gate", NodePortType::Control);    // Outlet 2: Control gate pulse
    addOutlet ("step", NodePortType::Control);    // Outlet 3: Current step index
    addOutlet ("velocity", NodePortType::Control); // Outlet 4: Control velocity amplitude

    setParameter ("bpm", 120.0f);
    setParameter ("steps", 8.0f);
    setParameter ("gateLength", 0.5f);
    setParameter ("playMode", 0.0f);   // 0: Fwd, 1: Rev, 2: PingPong, 3: Random, 4: BrownWalk
    setParameter ("scaleMode", 0.0f);  // 0: Off, 1: Major, 2: Minor, 3: Pentatonic
    setParameter ("rootNote", 0.0f);   // 0: C, 1: C# ...

    setPatternString (patternString);
}

void StepSequencerNode::setLabel (const std::string& l)
{
    RelativisticNode::setLabel (l);
    size_t spacePos = l.find (' ');
    if (spacePos != std::string::npos)
    {
        setPatternString (l.substr (spacePos + 1));
    }
}

void StepSequencerNode::setPatternString (const std::string& patStr)
{
    patternString = patStr;
    stepValues.clear();
    std::stringstream ss (patStr);
    std::string token;
    while (ss >> token)
    {
        stepValues.push_back (parseNoteOrFloat (token));
    }
    if (stepValues.empty()) stepValues = { 60.0f, 62.0f, 64.0f, 65.0f, 67.0f, 69.0f, 71.0f, 72.0f };
}

void StepSequencerNode::process (int numSamples)
{
    double gamma = std::abs (inlets[0].timeGamma);
    if (gamma < 0.001) gamma = 1.0;

    float trigIn = inlets[1].controlValue;
    float bpmMod = inlets[2].controlValue;

    float baseBpm = getModulatedParamValue ("bpm", 120.0f) + bpmMod;
    double effectiveBpm = baseBpm * gamma;
    double beatsPerSample = (effectiveBpm / 60.0) / currentSampleRate;

    auto* gateAudio = outlets[1].audioData.getWritePointer (0);
    outlets[1].audioData.clear();

    int totalSteps = static_cast<int>(std::max (1.0f, getParameter ("steps", 8.0f)));
    int pMode = static_cast<int>(getParameter ("playMode", 0.0f));

    if (stepValues.empty()) setPatternString (patternString);

    auto stepAdvance = [&] {
        if (totalSteps <= 1) { currentStep = 0; return; }
        if (pMode == 1) // Reverse
        {
            currentStep = (currentStep - 1 + totalSteps) % totalSteps;
        }
        else if (pMode == 2) // Ping-Pong
        {
            if (isPingPongReversing)
            {
                currentStep--;
                if (currentStep <= 0) { currentStep = 0; isPingPongReversing = false; }
            }
            else
            {
                currentStep++;
                if (currentStep >= totalSteps - 1) { currentStep = totalSteps - 1; isPingPongReversing = true; }
            }
        }
        else if (pMode == 3) // Random
        {
            currentStep = rand() % totalSteps;
        }
        else if (pMode == 4) // Brown Walk
        {
            int delta = (rand() % 3) - 1;
            currentStep = (currentStep + delta + totalSteps) % totalSteps;
        }
        else // Forward
        {
            currentStep = (currentStep + 1) % totalSteps;
        }
    };

    if (trigIn > 0.5f)
    {
        stepAdvance();
        stepProgress = 0.0;
    }
    else
    {
        for (int s = 0; s < numSamples; ++s)
        {
            stepProgress += beatsPerSample * 4.0; // 16th note steps
            if (stepProgress >= 1.0)
            {
                stepProgress -= 1.0;
                stepAdvance();
                gateAudio[s] = 0.9f;
                flashCounter = 8;
            }
        }
    }

    int safeStepIdx = currentStep % static_cast<int>(stepValues.size());
    float currentPitch = stepValues[safeStepIdx];

    outlets[0].controlValue = currentPitch;
    outlets[2].controlValue = (stepProgress < getParameter ("gateLength", 0.5f)) ? 1.0f : 0.0f;
    outlets[3].controlValue = static_cast<float>(currentStep);
    if (outlets.size() > 4) outlets[4].controlValue = 0.85f;
}

std::string StepSequencerNode::getDefaultFormulaScript() const
{
    return "// Relativistic Step Sequencer [seq]\n// Outputs pitch, audio gate pulse, and step index\n\npitch = step_pattern[step];";
}

std::vector<ParameterInfo> StepSequencerNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "bpm", "SEQUENCER BPM", getParameter ("bpm", 120.0f), 20.0f, 300.0f, getParamExpression ("bpm"), 2 });
    defs.push_back ({ "steps", "PATTERN LENGTH", getParameter ("steps", 8.0f), 1.0f, 32.0f, getParamExpression ("steps"), -1 });
    defs.push_back ({ "gateLength", "GATE LENGTH DURATION", getParameter ("gateLength", 0.5f), 0.05f, 1.0f, getParamExpression ("gateLength"), -1 });
    defs.push_back ({ "playMode", "PLAYHEAD DIRECTION", getParameter ("playMode", 0.0f), 0.0f, 4.0f, getParamExpression ("playMode"), -1 });
    defs.push_back ({ "scaleMode", "SCALE QUANTIZER", getParameter ("scaleMode", 0.0f), 0.0f, 10.0f, getParamExpression ("scaleMode"), -1 });
    return defs;
}

std::vector<std::string> StepSequencerNode::getExposedMethods() const
{
    return { "Reset Step", "Set Major Scale", "Set Pentatonic Scale", "Randomize Pattern", "Invert Pattern", "Reverse Pattern" };
}

void StepSequencerNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Reset Step")
    {
        currentStep = 0;
        stepProgress = 0.0;
        isPingPongReversing = false;
    }
    else if (methodName == "Set Major Scale")
    {
        setPatternString ("60 62 64 65 67 69 71 72");
    }
    else if (methodName == "Set Pentatonic Scale")
    {
        setPatternString ("60 63 65 67 70 72 75 77");
    }
    else if (methodName == "Randomize Pattern")
    {
        std::string rndStr;
        for (int i = 0; i < 8; ++i)
        {
            int note = 60 + rand() % 24;
            rndStr += std::to_string (note) + " ";
        }
        setPatternString (rndStr);
    }
    else if (methodName == "Invert Pattern")
    {
        if (!stepValues.empty())
        {
            float center = 66.0f;
            for (auto& val : stepValues) val = std::clamp (2.0f * center - val, 36.0f, 96.0f);
        }
    }
    else if (methodName == "Reverse Pattern")
    {
        std::reverse (stepValues.begin(), stepValues.end());
    }
}

// ----------------------------------------------------
// EuclideanSequencerNode Implementation
// ----------------------------------------------------
EuclideanSequencerNode::EuclideanSequencerNode (int id)
    : RelativisticNode (id, "euclid", "euclid 5 8 (Euclidean Rhythm)")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("pulses", NodePortType::Control);   // Inlet 1: Active pulses (k)
    addInlet ("steps", NodePortType::Control);    // Inlet 2: Total steps (n)

    addOutlet ("pulse~", NodePortType::Audio);    // Outlet 0: Audio trigger pulse
    addOutlet ("gate", NodePortType::Control);    // Outlet 1: Control gate
    addOutlet ("step", NodePortType::Control);    // Outlet 2: Current step
    addOutlet ("pitch", NodePortType::Control);   // Outlet 3: Pitch output

    setParameter ("pulses", 5.0f);
    setParameter ("steps", 8.0f);
    setParameter ("rotation", 0.0f);
    setParameter ("bpm", 120.0f);

    setPitchPatternString (pitchPatternString);
    generatePattern();
}

void EuclideanSequencerNode::setLabel (const std::string& l)
{
    RelativisticNode::setLabel (l);
    std::stringstream ss (l);
    std::string name;
    int p = 5, s = 8;
    if (ss >> name >> p >> s)
    {
        setParameter ("pulses", static_cast<float>(p));
        setParameter ("steps", static_cast<float>(s));
        generatePattern();
    }
}

void EuclideanSequencerNode::setPitchPatternString (const std::string& patStr)
{
    pitchPatternString = patStr;
    pitchValues.clear();
    std::stringstream ss (patStr);
    std::string token;
    while (ss >> token)
    {
        pitchValues.push_back (parseNoteOrFloat (token));
    }
    if (pitchValues.empty()) pitchValues = { 60.0f, 63.0f, 65.0f, 67.0f, 70.0f };
}

void EuclideanSequencerNode::generatePattern()
{
    pulses = std::clamp (static_cast<int>(getParameter ("pulses", 5.0f)), 0, 32);
    steps = std::clamp (static_cast<int>(getParameter ("steps", 8.0f)), 1, 32);
    pulses = std::min (pulses, steps);

    pattern.assign (steps, false);
    if (pulses <= 0) return;

    double bucket = 0.0;
    for (int i = 0; i < steps; ++i)
    {
        bucket += pulses;
        if (bucket >= steps)
        {
            bucket -= steps;
            pattern[i] = true;
        }
    }
}

void EuclideanSequencerNode::process (int numSamples)
{
    double gamma = std::abs (inlets[0].timeGamma);
    if (gamma < 0.001) gamma = 1.0;

    float baseBpm = getModulatedParamValue ("bpm", 120.0f);
    double effectiveBpm = baseBpm * gamma;
    double beatsPerSample = (effectiveBpm / 60.0) / currentSampleRate;

    auto* pulseOut = outlets[0].audioData.getWritePointer (0);
    outlets[0].audioData.clear();

    generatePattern();

    for (int s = 0; s < numSamples; ++s)
    {
        stepProgress += beatsPerSample * 4.0;
        if (stepProgress >= 1.0)
        {
            stepProgress -= 1.0;
            currentStep = (currentStep + 1) % std::max (1, steps);
            if (pattern[currentStep])
            {
                pulseOut[s] = 0.9f;
                if (!pitchValues.empty())
                {
                    float currentPitch = pitchValues[pitchIdx % pitchValues.size()];
                    outlets[3].controlValue = currentPitch;
                    pitchIdx++;
                }
            }
        }
    }

    outlets[1].controlValue = pattern[currentStep] ? 1.0f : 0.0f;
    outlets[2].controlValue = static_cast<float>(currentStep);
}

std::string EuclideanSequencerNode::getDefaultFormulaScript() const
{
    return "// Euclidean Rhythm Generator [euclid k n]\n// Distributes k pulses across n steps\n\ngate = euclid_pattern[step];";
}

std::vector<ParameterInfo> EuclideanSequencerNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "pulses", "EUCLIDEAN PULSES (k)", getParameter ("pulses", 5.0f), 0.0f, 32.0f, getParamExpression ("pulses"), 1 });
    defs.push_back ({ "steps", "TOTAL STEPS (n)", getParameter ("steps", 8.0f), 1.0f, 32.0f, getParamExpression ("steps"), 2 });
    defs.push_back ({ "bpm", "TEMPO BPM", getParameter ("bpm", 120.0f), 20.0f, 300.0f, getParamExpression ("bpm"), -1 });
    return defs;
}

std::vector<std::string> EuclideanSequencerNode::getExposedMethods() const
{
    return { "Rotate Left", "Rotate Right", "Reset Step", "Set Tresillo (3/8)", "Set Cumbia (5/8)", "Set Bossa Nova (5/16)" };
}

void EuclideanSequencerNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Rotate Left")
    {
        if (!pattern.empty()) std::rotate (pattern.begin(), pattern.begin() + 1, pattern.end());
    }
    else if (methodName == "Rotate Right")
    {
        if (!pattern.empty()) std::rotate (pattern.rbegin(), pattern.rbegin() + 1, pattern.rend());
    }
    else if (methodName == "Reset Step")
    {
        currentStep = 0;
        pitchIdx = 0;
        stepProgress = 0.0;
    }
    else if (methodName == "Set Tresillo (3/8)")
    {
        setParameter ("pulses", 3.0f);
        setParameter ("steps", 8.0f);
        generatePattern();
    }
    else if (methodName == "Set Cumbia (5/8)")
    {
        setParameter ("pulses", 5.0f);
        setParameter ("steps", 8.0f);
        generatePattern();
    }
    else if (methodName == "Set Bossa Nova (5/16)")
    {
        setParameter ("pulses", 5.0f);
        setParameter ("steps", 16.0f);
        generatePattern();
    }
}

// ----------------------------------------------------
// MarkovSequencerNode Implementation
// ----------------------------------------------------
MarkovSequencerNode::MarkovSequencerNode (int id)
    : RelativisticNode (id, "markov", "markov (Stochastic Generator)")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("trig", NodePortType::Control);     // Inlet 1: Step trigger

    addOutlet ("pitch", NodePortType::Control);   // Outlet 0: Generated MIDI pitch
    addOutlet ("pulse~", NodePortType::Audio);    // Outlet 1: Gate audio pulse

    setParameter ("bpm", 120.0f);
    setParameter ("randomness", 0.7f);
    setStatesString (statesString);
}

void MarkovSequencerNode::setLabel (const std::string& l)
{
    RelativisticNode::setLabel (l);
    size_t spacePos = l.find (' ');
    if (spacePos != std::string::npos)
    {
        setStatesString (l.substr (spacePos + 1));
    }
}

void MarkovSequencerNode::setStatesString (const std::string& stateStr)
{
    statesString = stateStr;
    states.clear();
    std::stringstream ss (stateStr);
    std::string token;
    while (ss >> token)
    {
        states.push_back (parseNoteOrFloat (token));
    }
    if (states.empty()) states = { 60.0f, 63.0f, 65.0f, 67.0f, 70.0f };
    currentStateIdx = std::clamp (currentStateIdx, 0, static_cast<int>(states.size()) - 1);
}

void MarkovSequencerNode::process (int numSamples)
{
    double gamma = std::abs (inlets[0].timeGamma);
    if (gamma < 0.001) gamma = 1.0;

    float baseBpm = getModulatedParamValue ("bpm", 120.0f);
    double effectiveBpm = baseBpm * gamma;
    double beatsPerSample = (effectiveBpm / 60.0) / currentSampleRate;

    auto* pulseOut = outlets[1].audioData.getWritePointer (0);
    outlets[1].audioData.clear();

    for (int s = 0; s < numSamples; ++s)
    {
        stepProgress += beatsPerSample * 4.0;
        if (stepProgress >= 1.0)
        {
            stepProgress -= 1.0;

            // Markov chain state transition
            float rnd = (float) rand() / RAND_MAX;
            if (rnd < getParameter ("randomness", 0.7f))
            {
                int step = (rand() % 3) - 1; // -1, 0, or +1 state shift
                currentStateIdx = std::clamp (currentStateIdx + step, 0, static_cast<int>(states.size()) - 1);
            }

            pulseOut[s] = 0.9f;
        }
    }

    outlets[0].controlValue = states[currentStateIdx];
}

std::string MarkovSequencerNode::getDefaultFormulaScript() const
{
    return "// Generative Markov Sequencer [markov]\n// Stochastic transition matrix for melodic generation\n\npitch = markov_state;";
}

std::vector<ParameterInfo> MarkovSequencerNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "bpm", "TEMPO BPM", getParameter ("bpm", 120.0f), 20.0f, 300.0f, getParamExpression ("bpm"), -1 });
    defs.push_back ({ "randomness", "MARKOV STOCHASTICITY", getParameter ("randomness", 0.7f), 0.0f, 1.0f, getParamExpression ("randomness"), -1 });
    return defs;
}

std::vector<std::string> MarkovSequencerNode::getExposedMethods() const
{
    return { "Reset State", "Randomize Scale" };
}

void MarkovSequencerNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Reset State")
    {
        currentStateIdx = 0;
    }
    else if (methodName == "Randomize Scale")
    {
        for (auto& st : states) st = 48.0f + (rand() % 36);
    }
}

TidalPatternSequencerNode::TidalPatternSequencerNode (int id)
    : RelativisticNode (id, "tidal", "tidal (Tidal Live-Coding Suite)")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("cps", NodePortType::Control);      // Inlet 1: Cycles per second (Tempo)
    addInlet ("patternIn", NodePortType::Control); // Inlet 2: Dynamic pattern mod

    addOutlet ("pitch", NodePortType::Control);   // Outlet 0: Generated MIDI pitch / frequency
    addOutlet ("gate~", NodePortType::Audio);     // Outlet 1: Subdivided trigger impulse pulse
    addOutlet ("gate", NodePortType::Control);    // Outlet 2: Control gate boolean
    addOutlet ("gain", NodePortType::Control);    // Outlet 3: Per-event velocity amplitude stream
    addOutlet ("pan", NodePortType::Control);     // Outlet 4: Per-event stereo panning stream
    addOutlet ("cutoff", NodePortType::Control);  // Outlet 5: Per-event filter cutoff stream (Hz)
    addOutlet ("timeOut", NodePortType::Time);    // Outlet 6: Per-step relativistic time dilation (gamma)
    addOutlet ("cyclePhase", NodePortType::Control); // Outlet 7: Current cycle phase [0, 1)

    setParameter ("cps", 0.5f); // 0.5 CPS = 120 BPM
    setParameter ("gateLength", 0.5f);
    setParameter ("transpose", 0.0f);

    setPatternString (patternString);
}

void TidalPatternSequencerNode::setLabel (const std::string& l)
{
    RelativisticNode::setLabel (l);
    size_t spacePos = l.find (' ');
    if (spacePos != std::string::npos)
    {
        setPatternString (l.substr (spacePos + 1));
    }
}

void TidalPatternSequencerNode::setPatternString (const std::string& patStr)
{
    patternString = patStr;
    parsePattern();
}

static float getScalePitch (const std::string& scaleName, int degree)
{
    static const std::vector<int> minorScale = { 0, 2, 3, 5, 7, 8, 10, 12 };
    static const std::vector<int> majorScale = { 0, 2, 4, 5, 7, 9, 11, 12 };
    static const std::vector<int> pentatonic = { 0, 2, 4, 7, 9, 12 };
    static const std::vector<int> dorian     = { 0, 2, 3, 5, 7, 9, 10, 12 };

    const std::vector<int>* sc = &minorScale;
    if (scaleName == "major") sc = &majorScale;
    else if (scaleName == "pentatonic") sc = &pentatonic;
    else if (scaleName == "dorian") sc = &dorian;

    int octave = degree / static_cast<int>(sc->size());
    int idx = degree % static_cast<int>(sc->size());
    if (idx < 0) { idx += static_cast<int>(sc->size()); octave--; }

    return 60.0f + (*sc)[idx] + (octave * 12.0f);
}

void TidalPatternSequencerNode::parsePattern()
{
    events.clear();
    if (patternString.empty()) return;

    // Check for "scale 'name' 'pattern'" wrapper
    std::string scaleName = "";
    std::string activePattern = patternString;

    if (patternString.rfind ("scale", 0) == 0)
    {
        std::smatch m;
        std::regex scaleRegex ("scale\\s+[\"'](\\w+)[\"']\\s+[\"']([^\"']+)[\"']");
        if (std::regex_search (patternString, m, scaleRegex))
        {
            scaleName = m[1].str();
            activePattern = m[2].str();
        }
    }

    // Mini-notation parser: Tokenizes top-level steps & nested [...] subgroups
    std::vector<std::string> topTokens;
    std::string currentToken;
    int nestLevel = 0;

    for (char c : activePattern)
    {
        if (c == '[') { nestLevel++; currentToken += c; }
        else if (c == ']') { nestLevel--; currentToken += c; }
        else if (std::isspace (c) && nestLevel == 0)
        {
            if (!currentToken.empty()) { topTokens.push_back (currentToken); currentToken.clear(); }
        }
        else { currentToken += c; }
    }
    if (!currentToken.empty()) topTokens.push_back (currentToken);

    int totalTopSteps = static_cast<int>(topTokens.size());
    if (totalTopSteps == 0) return;

    double stepDur = 1.0 / totalTopSteps;

    for (int i = 0; i < totalTopSteps; ++i)
    {
        double stepStart = i * stepDur;
        std::string tok = topTokens[i];

        // Check for per-step warp modifier "tok*gamma" e.g. "60*2.0" or "64*-1.0"
        double stepGamma = 1.0;
        auto starPos = tok.find ('*');
        if (starPos != std::string::npos && tok.front() != '[')
        {
            std::string multStr = tok.substr (starPos + 1);
            tok = tok.substr (0, starPos);
            stepGamma = std::atof (multStr.c_str());
        }

        // Parse nested subgroup [a b c]
        if (tok.front() == '[' && tok.back() == ']')
        {
            std::string inner = tok.substr (1, tok.length() - 2);
            std::stringstream ss (inner);
            std::vector<std::string> subToks;
            std::string subTok;
            while (ss >> subTok) subToks.push_back (subTok);

            int subCount = static_cast<int>(subToks.size());
            if (subCount > 0)
            {
                double subDur = stepDur / subCount;
                for (int s = 0; s < subCount; ++s)
                {
                    TidalEvent ev;
                    ev.startPhase = stepStart + (s * subDur);
                    ev.durationPhase = subDur;
                    ev.gamma = stepGamma;
                    if (subToks[s] == "~")
                    {
                        ev.isRest = true;
                    }
                    else
                    {
                        ev.isRest = false;
                        if (!scaleName.empty())
                        {
                            int deg = std::atoi (subToks[s].c_str());
                            ev.pitch = getScalePitch (scaleName, deg);
                        }
                        else
                        {
                            ev.pitch = parseNoteOrFloat (subToks[s]);
                        }
                    }
                    events.push_back (ev);
                }
            }
        }
        else
        {
            TidalEvent ev;
            ev.startPhase = stepStart;
            ev.durationPhase = stepDur;
            ev.gamma = stepGamma;
            if (tok == "~")
            {
                ev.isRest = true;
            }
            else
            {
                ev.isRest = false;
                if (!scaleName.empty())
                {
                    int deg = std::atoi (tok.c_str());
                    ev.pitch = getScalePitch (scaleName, deg);
                }
                else
                {
                    ev.pitch = parseNoteOrFloat (tok);
                }
            }
            events.push_back (ev);
        }
    }
}

void TidalPatternSequencerNode::process (int numSamples)
{
    double parentG = std::abs (inlets[0].timeGamma);
    if (parentG < 0.001) parentG = 1.0;

    float baseCps = getModulatedParamValue ("cps", 0.5f) + inlets[1].controlValue;
    double effectiveCps = baseCps * parentG;
    double phaseIncPerSample = effectiveCps / currentSampleRate;

    auto* gateAudio = outlets[1].audioData.getWritePointer (0);
    outlets[1].audioData.clear();

    if (events.empty()) parsePattern();

    float currentPitch = 60.0f + getParameter ("transpose", 0.0f);
    float currentGain = 1.0f;
    float currentPan = 0.5f;
    float currentCutoff = 2000.0f;
    double currentGamma = parentG;
    bool isGateOn = false;

    for (int s = 0; s < numSamples; ++s)
    {
        cyclePhase += phaseIncPerSample;

        if (cyclePhase >= 1.0)
        {
            cyclePhase -= 1.0;
            cycleCount++;
        }

        // Check active event for current cycle phase
        for (int e = 0; e < static_cast<int>(events.size()); ++e)
        {
            const auto& ev = events[e];
            if (cyclePhase >= ev.startPhase && cyclePhase < (ev.startPhase + ev.durationPhase))
            {
                if (e != currentEventIdx && !ev.isRest)
                {
                    gateAudio[s] = 0.9f; // Trigger impulse on sub-divided step start!
                    currentEventIdx = e;
                }

                if (!ev.isRest)
                {
                    currentPitch = ev.pitch + getParameter ("transpose", 0.0f);
                    currentGain = ev.gain;
                    currentPan = ev.pan;
                    currentCutoff = ev.cutoff;
                    currentGamma = ev.gamma * parentG;

                    double relPhaseInEv = (cyclePhase - ev.startPhase) / ev.durationPhase;
                    isGateOn = (relPhaseInEv < getParameter ("gateLength", 0.5f));
                }
                break;
            }
        }
    }

    outlets[0].controlValue = currentPitch;
    outlets[2].controlValue = isGateOn ? 1.0f : 0.0f;
    outlets[3].controlValue = currentGain;
    outlets[4].controlValue = currentPan;
    outlets[5].controlValue = currentCutoff;
    outlets[6].timeGamma = currentGamma;
    outlets[7].controlValue = static_cast<float>(cyclePhase);
}

std::string TidalPatternSequencerNode::getDefaultFormulaScript() const
{
    return "// Tidal Live-Coding Suite [tidal]\n// Parses complex Euclidean subdivisions, scales, per-step gamma warp, pan, cutoff\n\npitch = tidal.pitch;\ngate = tidal.gate;\ngamma = tidal.timeOut;";
}

std::vector<ParameterInfo> TidalPatternSequencerNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "cps", "CYCLES PER SECOND (CPS)", getParameter ("cps", 0.5f), 0.05f, 10.0f, getParamExpression ("cps"), 1 });
    defs.push_back ({ "gateLength", "GATE LENGTH DURATION", getParameter ("gateLength", 0.5f), 0.05f, 1.0f, getParamExpression ("gateLength"), -1 });
    defs.push_back ({ "transpose", "SEMITONE TRANSPOSE", getParameter ("transpose", 0.0f), -24.0f, 24.0f, getParamExpression ("transpose"), -1 });
    return defs;
}

std::vector<std::string> TidalPatternSequencerNode::getExposedMethods() const
{
    return { "Pattern: Scale Degree Mapping", "Pattern: Relativistic Warp Steps", "Pattern: Syncopated Rests", "Pattern: Fast Triplets", "Reset Cycle Phase" };
}

void TidalPatternSequencerNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Pattern: Scale Degree Mapping")
    {
        setPatternString ("scale 'minor' '0 [2 3] 4 [5 7]'");
    }
    else if (methodName == "Pattern: Relativistic Warp Steps")
    {
        setPatternString ("60*1.0 [62 64]*2.0 65*0.5 [67 71]*-1.0");
    }
    else if (methodName == "Pattern: Syncopated Rests")
    {
        setPatternString ("c4 ~ [e4 g4] ~ b4 ~");
    }
    else if (methodName == "Pattern: Fast Triplets")
    {
        setPatternString ("[60 63 65] [67 70 72] [75 77 79] 84");
    }
    else if (methodName == "Reset Cycle Phase")
    {
        cyclePhase = 0.0;
        currentEventIdx = -1;
    }
}

// ----------------------------------------------------
// 5. [drumseq] Multi-Track Future Bass Drum Sequencer
// ----------------------------------------------------
DrumSequencerNode::DrumSequencerNode (int id)
    : RelativisticNode (id, "drumseq", "drumseq (Future Bass Drum Sequencer)")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("trigIn", NodePortType::Control);   // Inlet 1: External clock trigger
    addInlet ("bpm", NodePortType::Control);      // Inlet 2: Tempo modulation

    addOutlet ("midiNote", NodePortType::Control); // Outlet 0: Control MIDI note number stream (36, 38, 39, 42, 46, 48)
    addOutlet ("trig", NodePortType::Control);     // Outlet 1: Control Trigger impulse boolean (1.0)
    addOutlet ("vel", NodePortType::Control);      // Outlet 2: Velocity amplitude stream (0.9)
    addOutlet ("kickTrig", NodePortType::Control); // Outlet 3: Dedicated Kick trigger
    addOutlet ("snareTrig", NodePortType::Control); // Outlet 4: Dedicated Snare trigger
    addOutlet ("hatTrig", NodePortType::Control);  // Outlet 5: Dedicated Hi-Hat trigger
    addOutlet ("step", NodePortType::Control);     // Outlet 6: Current 16th step index

    setParameter ("bpm", 120.0f);
    setParameter ("steps", 16.0f);
    setParameter ("shuffle", 0.15f);

    setPresetPattern ("Future Bass Trap Beat");
}

void DrumSequencerNode::setLabel (const std::string& l)
{
    RelativisticNode::setLabel (l);
    setPresetPattern (l);
}

void DrumSequencerNode::setPresetPattern (const std::string& presetName)
{
    gridSteps.assign (16, DrumStep());

    if (presetName.find ("Jersey") != std::string::npos || presetName.find ("Bounce") != std::string::npos)
    {
        gridSteps[0].kick = true;
        gridSteps[3].kick = true;
        gridSteps[6].kick = true;
        gridSteps[9].kick = true;
        gridSteps[12].kick = true;
        gridSteps[8].snare = true; gridSteps[8].clap = true;
        gridSteps[14].snare = true;
        for (int i = 0; i < 16; ++i) gridSteps[i].hatClosed = true;
        gridSteps[2].hatOpen = true; gridSteps[10].hatOpen = true;
        gridSteps[4].perc = true; gridSteps[4].percPitch = 50;
        gridSteps[11].perc = true; gridSteps[11].percPitch = 53;
    }
    else if (presetName.find ("Half-Time") != std::string::npos || presetName.find ("Dub") != std::string::npos)
    {
        gridSteps[0].kick = true;
        gridSteps[10].kick = true;
        gridSteps[8].snare = true; gridSteps[8].clap = true;
        for (int i = 0; i < 16; i += 2) gridSteps[i].hatClosed = true;
        gridSteps[6].hatOpen = true; gridSteps[14].hatOpen = true;
        gridSteps[12].perc = true; gridSteps[12].percPitch = 52;
    }
    else if (presetName.find ("Glitch") != std::string::npos || presetName.find ("Roll") != std::string::npos)
    {
        // Glitch Burst Roll Pattern
        gridSteps[0].kick = true;
        gridSteps[3].kick = true;
        gridSteps[6].kick = true;
        gridSteps[10].kick = true;
        gridSteps[4].snare = true; gridSteps[8].snare = true; gridSteps[12].snare = true; gridSteps[14].snare = true; gridSteps[15].snare = true;
        gridSteps[8].clap = true; gridSteps[12].clap = true;
        for (int i = 0; i < 16; ++i) { if (i % 3 != 0) gridSteps[i].hatClosed = true; }
        gridSteps[7].hatOpen = true; gridSteps[15].hatOpen = true;
        gridSteps[2].perc = true; gridSteps[2].percPitch = 50;
        gridSteps[5].perc = true; gridSteps[5].percPitch = 55;
        gridSteps[9].perc = true; gridSteps[9].percPitch = 60;
        gridSteps[13].perc = true; gridSteps[13].percPitch = 62;
    }
    else if (presetName.find ("Poly") != std::string::npos || presetName.find ("Rhythm") != std::string::npos)
    {
        // Polyrhythmic Accent Pattern (3 vs 5 vs 7)
        for (int i = 0; i < 16; i += 3) gridSteps[i].kick = true;
        for (int i = 0; i < 16; i += 5) { gridSteps[i].snare = true; gridSteps[i].clap = true; }
        for (int i = 0; i < 16; i += 2) gridSteps[i].hatClosed = true;
        for (int i = 0; i < 16; i += 7) { gridSteps[i].perc = true; gridSteps[i].percPitch = 48 + i; }
    }
    else if (presetName.find ("Random") != std::string::npos)
    {
        for (int i = 0; i < 16; ++i)
        {
            gridSteps[i].kick = (rand() % 100 < 35);
            gridSteps[i].snare = (rand() % 100 < 30);
            gridSteps[i].clap = (rand() % 100 < 20);
            gridSteps[i].hatClosed = (rand() % 100 < 70);
            gridSteps[i].hatOpen = (rand() % 100 < 25);
            gridSteps[i].perc = (rand() % 100 < 25);
            gridSteps[i].percPitch = 48 + (rand() % 16);
            gridSteps[i].velocity = 0.5f + (rand() % 50) / 100.0f;
        }
    }
    else // Default Future Bass Trap Beat
    {
        gridSteps[0].kick = true;
        gridSteps[7].kick = true;
        gridSteps[10].kick = true;
        gridSteps[8].snare = true;
        gridSteps[8].clap = true;
        for (int i = 0; i < 16; ++i) gridSteps[i].hatClosed = true;
        gridSteps[4].hatOpen = true;
        gridSteps[12].hatOpen = true;
        gridSteps[14].hatClosed = false;
        gridSteps[3].perc = true; gridSteps[3].percPitch = 48;
        gridSteps[11].perc = true; gridSteps[11].percPitch = 52;
        gridSteps[15].perc = true; gridSteps[15].percPitch = 55;
    }
}

void DrumSequencerNode::process (int numSamples)
{
    double gamma = std::abs (inlets[0].timeGamma);
    if (gamma < 0.001) gamma = 1.0;

    float trigIn = inlets[1].controlValue;
    float bpmMod = inlets[2].controlValue;

    float baseBpm = getModulatedParamValue ("bpm", 120.0f) + bpmMod;
    double effectiveBpm = baseBpm * gamma;
    double beatsPerSample = (effectiveBpm / 60.0) / currentSampleRate;

    int totalSteps = std::clamp (static_cast<int>(getParameter ("steps", 16.0f)), 1, 32);

    for (size_t o = 0; o < outlets.size(); ++o)
    {
        outlets[o].audioData.clear();
    }

    auto* noteAudio  = outlets[0].audioData.getWritePointer (0);
    auto* trigAudio  = outlets[1].audioData.getWritePointer (0);
    auto* velAudio   = outlets[2].audioData.getWritePointer (0);
    auto* kickAudio  = outlets[3].audioData.getWritePointer (0);
    auto* snareAudio = outlets[4].audioData.getWritePointer (0);
    auto* hatAudio   = outlets[5].audioData.getWritePointer (0);

    bool triggerFired = false;

    for (int s = 0; s < numSamples; ++s)
    {
        bool stepHitThisSample = false;
        if (trigIn > 0.5f)
        {
            if (s == 0)
            {
                currentStep = (currentStep + 1) % totalSteps;
                stepProgress = 0.0;
                stepHitThisSample = true;
            }
        }
        else
        {
            stepProgress += beatsPerSample * 4.0;
            if (stepProgress >= 1.0)
            {
                stepProgress -= 1.0;
                currentStep = (currentStep + 1) % totalSteps;
                stepHitThisSample = true;
            }
        }

        if (stepHitThisSample)
        {
            triggerFired = true;
            int stepIdx = currentStep % static_cast<int>(gridSteps.size());
            const auto& step = gridSteps[stepIdx];

            if (step.kick || step.snare || step.clap || step.hatClosed || step.hatOpen || step.perc)
            {
                trigAudio[s] = 0.9f;
                velAudio[s]  = step.velocity;
                if (step.kick)           { noteAudio[s] = 36.0f; kickAudio[s] = 0.9f; }
                else if (step.snare)     { noteAudio[s] = 38.0f; snareAudio[s] = 0.9f; }
                else if (step.clap)      { noteAudio[s] = 39.0f; snareAudio[s] = 0.9f; }
                else if (step.hatClosed) { noteAudio[s] = 42.0f; hatAudio[s] = 0.9f; }
                else if (step.hatOpen)   { noteAudio[s] = 46.0f; hatAudio[s] = 0.9f; }
                else if (step.perc)      { noteAudio[s] = static_cast<float>(step.percPitch); }
            }
        }
    }

    int safeStepIdx = currentStep % static_cast<int>(gridSteps.size());
    const auto& lastStep = gridSteps[safeStepIdx];

    outlets[1].controlValue = (triggerFired && (lastStep.kick || lastStep.snare || lastStep.clap || lastStep.hatClosed || lastStep.hatOpen || lastStep.perc)) ? 1.0f : 0.0f;
    outlets[2].controlValue = lastStep.velocity;
    outlets[3].controlValue = (triggerFired && lastStep.kick) ? 1.0f : 0.0f;
    outlets[4].controlValue = (triggerFired && (lastStep.snare || lastStep.clap)) ? 1.0f : 0.0f;
    outlets[5].controlValue = (triggerFired && (lastStep.hatClosed || lastStep.hatOpen)) ? 1.0f : 0.0f;
    outlets[6].controlValue = static_cast<float>(currentStep);

    if (triggerFired)
    {
        if (lastStep.kick)           outlets[0].controlValue = 36.0f;
        else if (lastStep.snare)     outlets[0].controlValue = 38.0f;
        else if (lastStep.clap)      outlets[0].controlValue = 39.0f;
        else if (lastStep.hatClosed) outlets[0].controlValue = 42.0f;
        else if (lastStep.hatOpen)   outlets[0].controlValue = 46.0f;
        else if (lastStep.perc)      outlets[0].controlValue = static_cast<float>(lastStep.percPitch);
        else                         outlets[0].controlValue = 0.0f;
    }
    else
    {
        outlets[0].controlValue = 0.0f;
    }
}

std::string DrumSequencerNode::getDefaultFormulaScript() const
{
    return "// Multi-Track Future Bass Drum Sequencer [drumseq]\n// Outputs polyphonic drum MIDI trigger stream & dedicated stem triggers\n\nmidiNote = drumseq.midiNote;\ntrig = drumseq.trig;\nstep = drumseq.step;";
}

std::vector<ParameterInfo> DrumSequencerNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "bpm", "SEQUENCER BPM", getParameter ("bpm", 120.0f), 20.0f, 300.0f, getParamExpression ("bpm"), 2 });
    defs.push_back ({ "steps", "GRID STEPS", getParameter ("steps", 16.0f), 1.0f, 32.0f, getParamExpression ("steps"), -1 });
    defs.push_back ({ "shuffle", "SWING SHUFFLE %", getParameter ("shuffle", 0.15f), 0.0f, 0.75f, getParamExpression ("shuffle"), -1 });
    return defs;
}

std::vector<std::string> DrumSequencerNode::getExposedMethods() const
{
    return { "Pattern: Future Bass Trap Beat", "Pattern: Jersey Club / Future Bounce", "Pattern: Future Bass Half-Time", "Pattern: Glitch Burst Roll", "Pattern: Polyrhythms", "Randomize Drum Pattern", "Reset Step" };
}

void DrumSequencerNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Pattern: Future Bass Trap Beat")         setPresetPattern ("Future Bass Trap Beat");
    else if (methodName == "Pattern: Jersey Club / Future Bounce") setPresetPattern ("Jersey Bounce");
    else if (methodName == "Pattern: Future Bass Half-Time")     setPresetPattern ("Half-Time");
    else if (methodName == "Pattern: Glitch Burst Roll")         setPresetPattern ("Glitch");
    else if (methodName == "Pattern: Polyrhythms")               setPresetPattern ("Poly");
    else if (methodName == "Randomize Drum Pattern")            setPresetPattern ("Random");
    else if (methodName == "Reset Step")                       { currentStep = 0; stepProgress = 0.0; }
}

} // namespace time_dilation
