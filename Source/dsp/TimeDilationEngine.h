#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <tracktion_engine/tracktion_engine.h>
#include "HermiteResampler.h"
#include "DopplerDelay.h"
#include "GammaAutomationCurve.h"
#include "GammaLFO.h"
#include "GammaScriptEngine.h"
#include "GammaTapMatrix.h"
#include "PolySynthVoice.h"
#include "Midi2Packet.h"
#include "RetrogradeAudioBuffer.h"
#include "DataflowGraph.h"
#include <vector>

namespace time_dilation
{

namespace te = tracktion::engine;

enum class WarpMode
{
    Varispeed,   // Hermite cubic resampler (Bends pitch + time together)
    Granular,    // Pitch-preserved OLA time stretcher
    DopplerDelay // Doppler delay line warp
};

struct TrackState
{
    juce::String id;
    juce::String name;
    juce::Colour color;
    WarpMode warpMode = WarpMode::Varispeed;

    float timeDilation = 1.0f; // Gamma factor (0.1x to 4.0x)
    float volume = 0.8f;
    float pan = 0.0f;
    bool mute = false;
    bool solo = false;

    int parentTrackIndex = -1; // -1 for root track, >= 0 for sub-track inheriting parent gamma
    int gammaSourceTrackIndex = -1; // -1 for local, >= 0 for gamma-stream sidechain inheritance

    double properTime = 0.0; // Accumulated Proper Time (tau)
    float timeVelocity = 1.0f; // Current d(tau)/dt
    float currentAmplitude = 0.0f; // Live RMS audio amplitude

    bool isLooping = false; // Enable timeline looping per track/sub-track
    double loopStartTau = 0.0; // Loop start boundary in proper time (tau)
    double loopEndTau = 16.0; // Loop end boundary in proper time (tau)

    juce::String gammaScriptCode = "1.0"; // Algorithmic GammaScript expression
    bool isScriptEnabled = false;

    RetrogradeAudioBuffer retrogradeBuffer;
    std::shared_ptr<DataflowGraph> dataflowGraph = std::make_shared<DataflowGraph>();

    std::vector<bool> steps; // 16-step sequencer matrix
    std::vector<int> stepNotes; // MIDI note per step
    GammaAutomationCurve automationCurve;
    GammaLFO gammaLfo;

    juce::AudioBuffer<float> importedAudioBuffer;
    bool hasAudioFile = false;
};

class TimeDilationEngine
{
public:
    TimeDilationEngine();
    ~TimeDilationEngine();

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    // Transport Controls
    void play();
    void pause();
    void stop();
    void triggerAuditionNote();
    bool isPlaying() const { return playing; }

    // BPM & Master Time Dilation Controls
    float getBpm() const { return bpm; }
    void setBpm (float newBpm);

    float getMasterDilation() const { return masterDilation; }
    void setMasterDilation (float newDilation);

    double getCoordinateTime() const { return coordinateTime; }
    int getCurrentStep() const { return currentStep; }

    // Track Management
    const std::vector<TrackState>& getTracks() const { return tracks; }
    std::vector<TrackState>& getTracksMutable() { return tracks; }
    void updateTrackGamma (int trackIndex, float gamma);
    void updateTrackWarpMode (int trackIndex, WarpMode mode);
    void setTrackParent (int trackIndex, int parentIdx);
    void setTrackGammaSource (int trackIndex, int sourceIdx);
    void updateTrackStep (int trackIndex, int stepIndex, bool active);
    void updateTrackVolume (int trackIndex, float vol);
    void updateTrackPan (int trackIndex, float pan);
    void toggleMute (int trackIndex);
    void toggleSolo (int trackIndex);
    void addTrack (const juce::String& name, juce::Colour color, int parentIndex = -1);
    void removeTrack (int trackIndex);

    // Audio File Import
    bool importAudioFile (int trackIndex, const juce::File& audioFile);

    // Polyphonic Synth & MIDI Keyboard State
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

    // Gamma Tap Matrix
    GammaTapMatrix& getTapMatrix() { return tapMatrix; }

    // Project Save & Load Persistence
    bool saveProject (const juce::File& file);
    bool loadProject (const juce::File& file);

    // Offline Disk Render (WAV / FLAC Bounce)
    bool renderToDisk (const juce::File& outputFile, double durationInSeconds = 30.0);

    // Visualizer Ring Buffer Interface
    static const int visualizerFifoSize = 1024;
    void getVisualizerData (float* bufferToFill, int numSamplesNeeded);

    // Tracktion Engine Accessors
    te::Engine& getTracktionEngine() { return engine; }
    te::Edit& getEdit() { return *edit; }

private:
    mutable juce::CriticalSection engineLock;

    te::Engine engine { "Time Dilation DAW" };
    std::unique_ptr<te::Edit> edit;

    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    bool playing = false;
    float bpm = 120.0f;
    float masterDilation = 1.0f;
    double coordinateTime = 0.0;
    int currentStep = 0;
    double stepTimer = 0.0;

    std::vector<TrackState> tracks;
    std::vector<HermiteResampler> resamplers;
    std::vector<DopplerDelay> dopplerDelays;
    GammaTapMatrix tapMatrix;

    // Calculate nested effective gamma for a track traversing parent tree & evaluating GammaScript
    float calculateEffectiveGamma (size_t trackIndex);

    // Polyphonic Synthesizer & MIDI Input
    juce::Synthesiser polySynth;
    juce::MidiKeyboardState keyboardState;
    juce::AudioFormatManager formatManager;

    // Synthetic Voice Generators for internal tracks
    void renderTrackSynth (int trackIndex, juce::AudioBuffer<float>& trackBuffer, int numSamples);

    // Thread-safe FIFO for Spacetime Visualizer
    juce::AbstractFifo fifo { visualizerFifoSize };
    std::vector<float> fifoBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeDilationEngine)
};

} // namespace time_dilation
