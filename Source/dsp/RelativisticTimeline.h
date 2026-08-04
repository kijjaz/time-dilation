#pragma once

#include "RelativisticNodeGraph.h"
#include "AssetPoolManager.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <string>
#include <memory>
#include <cmath>

namespace time_dilation
{

enum class TrackType
{
    Audio,
    Midi,
    TimeDilation,
    ControlAutomation
};

enum class RecordInputSource
{
    InternalPatch,     // Direct patch cord input from graph
    PhysicalHardware   // Physical audio interface input channel
};

struct AudioClip
{
    int poolAssetId = 0;
    double startBeat = 0.0;
    double durationBeats = 4.0;
    juce::AudioBuffer<float> buffer;
    std::string clipName = "Audio Clip";
};

struct MidiNoteEvent
{
    double startBeat = 0.0;
    double durationBeats = 1.0;
    int noteNumber = 60; // Middle C
    float velocity = 0.8f;
};

struct TimeDilationPoint
{
    double beat = 0.0;
    float gamma = 1.0f;
};

struct ControlAutomationPoint
{
    double beat = 0.0;
    float value = 0.0f;
};

class TimelineTrack
{
public:
    TimelineTrack (const std::string& name, TrackType type);

    std::string getName() const { return trackName; }
    void setName (const std::string& n) { trackName = n; }

    TrackType getType() const { return trackType; }

    bool isArmed() const { return armed; }
    void setArmed (bool a) { armed = a; }

    bool isMuted() const { return muted; }
    void setMuted (bool m) { muted = m; }

    bool isSoloed() const { return soloed; }
    void setSoloed (bool s) { soloed = s; }

    float getVolume() const { return volume; }
    void setVolume (float v) { volume = v; }

    RecordInputSource getInputSource() const { return inputSource; }
    void setInputSource (RecordInputSource src) { inputSource = src; }

    int getInputChannelIndex() const { return inputChannelIndex; }
    void setInputChannelIndex (int ch) { inputChannelIndex = ch; }

    int getNumChannels() const { return numChannels; }
    void setNumChannels (int n) { numChannels = n; }

    BitDepthFormat getRecordingBitDepth() const { return recordingBitDepth; }
    void setRecordingBitDepth (BitDepthFormat fmt) { recordingBitDepth = fmt; }

    // Audio Clips
    std::vector<AudioClip>& getAudioClips() { return audioClips; }
    const std::vector<AudioClip>& getAudioClips() const { return audioClips; }
    void addAudioClip (const AudioClip& clip) { audioClips.push_back (clip); }

    // MIDI Notes
    std::vector<MidiNoteEvent>& getMidiNotes() { return midiNotes; }
    const std::vector<MidiNoteEvent>& getMidiNotes() const { return midiNotes; }
    void addMidiNote (const MidiNoteEvent& note) { midiNotes.push_back (note); }

    // Time Dilation Automation Points
    std::vector<TimeDilationPoint>& getDilationPoints() { return dilationPoints; }
    const std::vector<TimeDilationPoint>& getDilationPoints() const { return dilationPoints; }
    void addDilationPoint (double beat, float gamma);
    float getDilationAtBeat (double beat) const;

    // Control Value Automation Points
    std::vector<ControlAutomationPoint>& getControlPoints() { return controlPoints; }
    const std::vector<ControlAutomationPoint>& getControlPoints() const { return controlPoints; }
    void addControlPoint (double beat, float val);
    float getControlAtBeat (double beat) const;

    // Recording buffer
    juce::AudioBuffer<float>& getRecordingBuffer() { return recordingBuffer; }
    double getRecordStartBeat() const { return recordStartBeat; }
    void startRecording (double currentBeat);
    void stopRecording (double currentBeat, double sampleRate = 44100.0, double bpm = 120.0);

private:
    std::string trackName;
    TrackType trackType = TrackType::Audio;

    bool armed = false;
    bool muted = false;
    bool soloed = false;
    float volume = 0.8f;

    RecordInputSource inputSource = RecordInputSource::InternalPatch;
    int inputChannelIndex = 0;
    int numChannels = 2; // Stereo by default
    BitDepthFormat recordingBitDepth = BitDepthFormat::Int24;

    std::vector<AudioClip> audioClips;
    std::vector<MidiNoteEvent> midiNotes;
    std::vector<TimeDilationPoint> dilationPoints;
    std::vector<ControlAutomationPoint> controlPoints;

    juce::AudioBuffer<float> recordingBuffer;
    double recordStartBeat = 0.0;
    bool isRecording = false;
};

// ----------------------------------------------------
// TimelineNode ([timeline]) Multi-Instance Node
// ----------------------------------------------------
class TimelineNode : public RelativisticNode
{
public:
    TimelineNode (int id);

    void process (int numSamples) override;
    std::string getDefaultFormulaScript() const override;
    std::vector<ParameterInfo> getParameterDefs() const override;
    std::vector<std::string> getExposedMethods() const override;
    void invokeMethod (const std::string& methodName) override;
    void receiveMessage (const std::string& msg, float val = 1.0f) override;

    // Track Management
    int addTrack (const std::string& name, TrackType type);
    void removeTrack (int index);
    std::vector<TimelineTrack>& getTracks() { return tracks; }
    const std::vector<TimelineTrack>& getTracks() const { return tracks; }

    double getCurrentPlayheadBeat() const { return currentPlayheadBeat; }
    void setPlayheadBeat (double beat) { currentPlayheadBeat = beat; }

    bool getIsRecording() const { return isRecording; }
    bool getIsPlaying() const { return isPlaying; }

    juce::ValueTree saveToValueTree() const override;
    void loadFromValueTree (const juce::ValueTree& v, bool preserveExistingId = false) override;

private:
    std::vector<TimelineTrack> tracks;
    double currentPlayheadBeat = 0.0;
    bool isPlaying = false;
    bool isRecording = false;
    float bpm = 120.0f;
};

} // namespace time_dilation
