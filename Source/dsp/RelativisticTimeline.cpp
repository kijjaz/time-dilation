#include "RelativisticTimeline.h"
#include <algorithm>

namespace time_dilation
{

TimelineTrack::TimelineTrack (const std::string& name, TrackType type)
    : trackName (name), trackType (type)
{
    recordingBuffer.setSize (2, 44100 * 30); // 30 sec record capacity
    recordingBuffer.clear();
}

void TimelineTrack::addDilationPoint (double beat, float gamma)
{
    TimeDilationPoint pt { beat, gamma };
    dilationPoints.push_back (pt);
    std::sort (dilationPoints.begin(), dilationPoints.end(),
        [] (const TimeDilationPoint& a, const TimeDilationPoint& b) { return a.beat < b.beat; });
}

float TimelineTrack::getDilationAtBeat (double beat) const
{
    if (dilationPoints.empty()) return 1.0f;
    if (beat <= dilationPoints.front().beat) return dilationPoints.front().gamma;
    if (beat >= dilationPoints.back().beat) return dilationPoints.back().gamma;

    for (size_t i = 0; i < dilationPoints.size() - 1; ++i)
    {
        if (beat >= dilationPoints[i].beat && beat <= dilationPoints[i + 1].beat)
        {
            double t = (beat - dilationPoints[i].beat) / (dilationPoints[i + 1].beat - dilationPoints[i].beat);
            return dilationPoints[i].gamma + static_cast<float>(t) * (dilationPoints[i + 1].gamma - dilationPoints[i].gamma);
        }
    }
    return 1.0f;
}

void TimelineTrack::addControlPoint (double beat, float val)
{
    ControlAutomationPoint pt { beat, val };
    controlPoints.push_back (pt);
    std::sort (controlPoints.begin(), controlPoints.end(),
        [] (const ControlAutomationPoint& a, const ControlAutomationPoint& b) { return a.beat < b.beat; });
}

float TimelineTrack::getControlAtBeat (double beat) const
{
    if (controlPoints.empty()) return 0.0f;
    if (beat <= controlPoints.front().beat) return controlPoints.front().value;
    if (beat >= controlPoints.back().beat) return controlPoints.back().value;

    for (size_t i = 0; i < controlPoints.size() - 1; ++i)
    {
        if (beat >= controlPoints[i].beat && beat <= controlPoints[i + 1].beat)
        {
            double t = (beat - controlPoints[i].beat) / (controlPoints[i + 1].beat - controlPoints[i].beat);
            return controlPoints[i].value + static_cast<float>(t) * (controlPoints[i + 1].value - controlPoints[i].value);
        }
    }
    return 0.0f;
}

void TimelineTrack::startRecording (double currentBeat)
{
    isRecording = true;
    recordStartBeat = currentBeat;
    recordingBuffer.clear();
}

void TimelineTrack::stopRecording (double currentBeat)
{
    if (!isRecording) return;
    isRecording = false;

    if (trackType == TrackType::Audio)
    {
        AudioClip clip;
        clip.startBeat = recordStartBeat;
        clip.durationBeats = std::max (0.25, currentBeat - recordStartBeat);
        clip.clipName = trackName + " Clip";
        clip.buffer.makeCopyOf (recordingBuffer);
        audioClips.push_back (clip);
    }
}

// ----------------------------------------------------
// TimelineNode Implementation
// ----------------------------------------------------
TimelineNode::TimelineNode (int id)
    : RelativisticNode (id, "timeline", "timeline (Arrangement View)")
{
    addInlet ("timeIn", NodePortType::Time);       // Inlet 0: Dilated coordinate time
    addInlet ("transport", NodePortType::Control); // Inlet 1: Transport beat / sync
    addInlet ("record", NodePortType::Control);    // Inlet 2: Master Record arm
    addInlet ("inL~", NodePortType::Audio);        // Inlet 3: Audio In Left
    addInlet ("inR~", NodePortType::Audio);        // Inlet 4: Audio In Right
    addInlet ("midiIn", NodePortType::Control);    // Inlet 5: MIDI / Note In

    addOutlet ("outL~", NodePortType::Audio);       // Outlet 0: Audio Out Left
    addOutlet ("outR~", NodePortType::Audio);       // Outlet 1: Audio Out Right
    addOutlet ("midiOut", NodePortType::Control);   // Outlet 2: MIDI / Note Out
    addOutlet ("dilationOut", NodePortType::Time);  // Outlet 3: Automated Time Dilation Gamma

    setParameter ("bpm", 120.0f);
    setParameter ("playhead", 0.0f);
    setParameter ("isArmed", 0.0f);

    // Create 3 default arrangement tracks
    addTrack ("Audio Track 1", TrackType::Audio);
    addTrack ("MIDI Track 1", TrackType::Midi);
    addTrack ("Time Dilation Track", TrackType::TimeDilation);

    // Seed default automation curve on time dilation track
    if (tracks.size() >= 3)
    {
        tracks[2].addDilationPoint (0.0, 1.0f);
        tracks[2].addDilationPoint (8.0, 2.0f);
        tracks[2].addDilationPoint (16.0, 0.5f);
    }
}

int TimelineNode::addTrack (const std::string& name, TrackType type)
{
    tracks.emplace_back (name, type);
    return static_cast<int>(tracks.size()) - 1;
}

void TimelineNode::removeTrack (int index)
{
    if (index >= 0 && index < static_cast<int>(tracks.size()))
    {
        tracks.erase (tracks.begin() + index);
    }
}

void TimelineNode::process (int numSamples)
{
    double gamma = std::abs (inlets[0].timeGamma);
    if (gamma < 0.001) gamma = 1.0;

    float transSync = inlets[1].controlValue;
    float recCtrl   = inlets[2].controlValue;

    if (transSync > 0.0f)
    {
        currentPlayheadBeat = transSync;
        isPlaying = true;
    }
    else
    {
        bpm = getParameter ("bpm", 120.0f);
        double beatsPerSample = (bpm / 60.0) / currentSampleRate * gamma;
        currentPlayheadBeat += beatsPerSample * numSamples;
    }

    setParameter ("playhead", static_cast<float>(currentPlayheadBeat));

    bool masterArmed = (recCtrl > 0.5f) || (getParameter ("isArmed", 0.0f) > 0.5f);

    // Handle recording logic
    if (masterArmed && !isRecording)
    {
        isRecording = true;
        for (auto& trk : tracks)
        {
            if (trk.isArmed()) trk.startRecording (currentPlayheadBeat);
        }
    }
    else if (!masterArmed && isRecording)
    {
        isRecording = false;
        for (auto& trk : tracks)
        {
            trk.stopRecording (currentPlayheadBeat);
        }
    }

    // Process outlets
    outlets[0].audioData.clear();
    outlets[1].audioData.clear();

    const auto* inL = inlets[3].audioData.getReadPointer (0);
    const auto* inR = inlets[4].audioData.getReadPointer (0);

    // Record incoming audio into armed tracks
    if (isRecording)
    {
        for (auto& trk : tracks)
        {
            if (trk.isArmed() && trk.getType() == TrackType::Audio)
            {
                auto& recBuf = trk.getRecordingBuffer();
                if (recBuf.getNumSamples() >= numSamples)
                {
                    recBuf.addFrom (0, 0, inL, numSamples);
                    recBuf.addFrom (1, 0, inR, numSamples);
                }
            }
        }
    }

    // Render playback audio from clips
    for (const auto& trk : tracks)
    {
        if (trk.isMuted() || trk.getType() != TrackType::Audio) continue;

        float vol = trk.getVolume();
        for (const auto& clip : trk.getAudioClips())
        {
            if (currentPlayheadBeat >= clip.startBeat && currentPlayheadBeat < (clip.startBeat + clip.durationBeats))
            {
                double clipOffsetBeats = currentPlayheadBeat - clip.startBeat;
                int clipSampleIdx = static_cast<int>((clipOffsetBeats * (60.0 / bpm)) * currentSampleRate);

                if (clipSampleIdx >= 0 && clipSampleIdx + numSamples < clip.buffer.getNumSamples())
                {
                    outlets[0].audioData.addFrom (0, 0, clip.buffer, 0, clipSampleIdx, numSamples, vol);
                    outlets[1].audioData.addFrom (0, 0, clip.buffer, 1, clipSampleIdx, numSamples, vol);
                }
            }
        }
    }

    // Render Time Dilation Track output
    float dilGamma = 1.0f;
    for (const auto& trk : tracks)
    {
        if (trk.getType() == TrackType::TimeDilation)
        {
            dilGamma = trk.getDilationAtBeat (currentPlayheadBeat);
            break;
        }
    }
    outlets[3].timeGamma = static_cast<double>(dilGamma);
}

std::string TimelineNode::getDefaultFormulaScript() const
{
    return "// Multi-Track Relativistic Timeline [timeline]\n// Output: outL~, outR~, midiOut, dilationOut\n\noutL = audio_tracks_sum_L;\noutR = audio_tracks_sum_R;\ndilationOut = dilation_automation(beat);";
}

std::vector<ParameterInfo> TimelineNode::getParameterDefs() const
{
    std::vector<ParameterInfo> defs;
    defs.push_back ({ "bpm", "TIMELINE BPM", getParameter ("bpm", 120.0f), 20.0f, 300.0f, getParamExpression ("bpm"), -1 });
    defs.push_back ({ "playhead", "PLAYHEAD BEAT", getParameter ("playhead", 0.0f), 0.0f, 999.0f, getParamExpression ("playhead"), -1 });
    defs.push_back ({ "isArmed", "MASTER RECORD ARM", getParameter ("isArmed", 0.0f), 0.0f, 1.0f, getParamExpression ("isArmed"), -1 });
    return defs;
}

std::vector<std::string> TimelineNode::getExposedMethods() const
{
    return { "Add Audio Track", "Add MIDI Track", "Add Dilation Track", "Add Control Automation Track", "Arm All Tracks", "Clear Tracks" };
}

void TimelineNode::invokeMethod (const std::string& methodName)
{
    if (methodName == "Add Audio Track")
    {
        addTrack ("Audio Track " + std::to_string (tracks.size() + 1), TrackType::Audio);
    }
    else if (methodName == "Add MIDI Track")
    {
        addTrack ("MIDI Track " + std::to_string (tracks.size() + 1), TrackType::Midi);
    }
    else if (methodName == "Add Dilation Track")
    {
        addTrack ("Time Dilation Track", TrackType::TimeDilation);
    }
    else if (methodName == "Add Control Automation Track")
    {
        addTrack ("Control Automation Track", TrackType::ControlAutomation);
    }
    else if (methodName == "Arm All Tracks")
    {
        for (auto& trk : tracks) trk.setArmed (true);
    }
    else if (methodName == "Clear Tracks")
    {
        tracks.clear();
    }
}

juce::ValueTree TimelineNode::saveToValueTree() const
{
    auto v = RelativisticNode::saveToValueTree();
    juce::ValueTree tracksTree ("Tracks");
    for (const auto& trk : tracks)
    {
        juce::ValueTree t ("Track");
        t.setProperty ("name", juce::String (trk.getName()), nullptr);
        t.setProperty ("type", static_cast<int>(trk.getType()), nullptr);
        t.setProperty ("volume", trk.getVolume(), nullptr);
        t.setProperty ("armed", trk.isArmed(), nullptr);
        t.setProperty ("muted", trk.isMuted(), nullptr);
        tracksTree.addChild (t, -1, nullptr);
    }
    v.addChild (tracksTree, -1, nullptr);
    return v;
}

void TimelineNode::loadFromValueTree (const juce::ValueTree& v, bool preserveExistingId)
{
    RelativisticNode::loadFromValueTree (v, preserveExistingId);
    auto tracksTree = v.getChildWithName ("Tracks");
    if (tracksTree.isValid())
    {
        tracks.clear();
        for (int i = 0; i < tracksTree.getNumChildren(); ++i)
        {
            auto t = tracksTree.getChild (i);
            std::string name = t.getProperty ("name").toString().toStdString();
            TrackType type = static_cast<TrackType>(static_cast<int>(t.getProperty ("type")));
            int idx = addTrack (name, type);
            tracks[idx].setVolume (t.getProperty ("volume", 0.8f));
            tracks[idx].setArmed (t.getProperty ("armed", false));
            tracks[idx].setMuted (t.getProperty ("muted", false));
        }
    }
}

} // namespace time_dilation
