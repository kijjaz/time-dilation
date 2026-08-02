#include "TimeDilationEngine.h"

namespace time_dilation
{

TimeDilationEngine::TimeDilationEngine()
{
    formatManager.registerBasicFormats();

    // Setup Polyphonic Synth Engine
    for (int i = 0; i < 8; ++i)
        polySynth.addVoice (new PolySynthVoice());
    polySynth.addSound (new PolySynthSound());

    // Create an empty Tracktion Engine Edit
    edit = te::createEmptyEdit (engine, juce::File());

    // Setup initial clean slate project tracks
    addTrack ("Audio Track 1", juce::Colour (0xfff59e0b)); // Gold
    addTrack ("Synth Track 2", juce::Colour (0xff8b5cf6)); // Purple

    fifoBuffer.resize (visualizerFifoSize, 0.0f);
}

TimeDilationEngine::~TimeDilationEngine()
{
    edit = nullptr;
}

void TimeDilationEngine::prepareToPlay (double sRate, int sPerBlock)
{
    this->sampleRate = sRate;
    this->samplesPerBlock = sPerBlock;

    polySynth.setCurrentPlaybackSampleRate (sampleRate);

    for (auto& r : resamplers) r.prepare (sampleRate, samplesPerBlock, 2);
    for (auto& d : dopplerDelays) d.prepare (sampleRate, samplesPerBlock, 2);
    for (auto& t : tracks) t.gammaLfo.prepare (sampleRate);
}

void TimeDilationEngine::releaseResources()
{
    for (auto& r : resamplers) r.reset();
    for (auto& d : dopplerDelays) d.reset();
}

float TimeDilationEngine::calculateEffectiveGamma (size_t trackIndex)
{
    if (trackIndex >= tracks.size()) return 1.0f;

    auto& track = tracks[trackIndex];
    float localGamma = track.timeDilation;
    float lfoGamma = track.gammaLfo.getNextGamma (coordinateTime);
    float curveGamma = track.automationCurve.getGammaAtTime (coordinateTime);

    float scriptGamma = 1.0f;
    if (track.isScriptEnabled && !track.gammaScriptCode.isEmpty())
    {
        ScriptVariables vars;
        vars.t = coordinateTime;
        vars.tau = track.properTime;
        vars.step = currentStep;
        vars.bpm = bpm;
        vars.amp = track.currentAmplitude;
        vars.tap1 = tapMatrix.getTapGamma (0);
        vars.tap2 = tapMatrix.getTapGamma (1);

        scriptGamma = GammaScriptEngine::evaluate (track.gammaScriptCode, vars);
    }

    float effective = localGamma * lfoGamma * curveGamma * scriptGamma;

    // Inherit from sidechain gamma source track if configured
    if (track.gammaSourceTrackIndex >= 0 && track.gammaSourceTrackIndex < static_cast<int>(tracks.size()))
    {
        effective *= tracks[track.gammaSourceTrackIndex].timeVelocity;
    }

    // Traverse Parent Track Tree Hierarchy
    if (track.parentTrackIndex >= 0 && track.parentTrackIndex < static_cast<int>(tracks.size())
        && track.parentTrackIndex != static_cast<int>(trackIndex))
    {
        effective *= calculateEffectiveGamma (static_cast<size_t>(track.parentTrackIndex));
    }
    else
    {
        effective *= masterDilation;
    }

    return juce::jlimit (0.05f, 8.0f, effective);
}

void TimeDilationEngine::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0) return;

    // Process Live MIDI Keyboard Input
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);
    polySynth.renderNextBlock (buffer, midiMessages, 0, numSamples);

    if (playing)
    {
        // 1. Advance Master Transport Clock
        const double stepDurationSeconds = (60.0 / bpm / 4.0);
        const double deltaSeconds = (numSamples / sampleRate) * masterDilation;
        coordinateTime += deltaSeconds;

        stepTimer += deltaSeconds;
        if (stepTimer >= stepDurationSeconds)
        {
            stepTimer -= stepDurationSeconds;
            currentStep = (currentStep + 1) % 16;
        }

        // 2. Process Tracks with Hierarchical Gamma Inheritance, GammaScript Evaluation & Warp Mode
        juce::AudioBuffer<float> trackBuffer (2, numSamples);
        juce::AudioBuffer<float> processedBuffer (2, numSamples);

        const bool hasSolo = std::any_of (tracks.begin(), tracks.end(), [] (const TrackState& t) { return t.solo; });

        for (size_t i = 0; i < tracks.size(); ++i)
        {
            auto& track = tracks[i];
            if (track.mute) continue;
            if (hasSolo && !track.solo) continue;

            trackBuffer.clear();
            processedBuffer.clear();

            // Calculate Effective Gamma & Update Track Proper Time (tau)
            float effectiveGamma = calculateEffectiveGamma (i);
            track.timeVelocity = effectiveGamma;
            track.properTime += deltaSeconds * effectiveGamma;

            // Apply Per-Track & Sub-Track Relativistic Timeline Looping
            if (track.isLooping && track.loopEndTau > track.loopStartTau)
            {
                if (track.properTime >= track.loopEndTau)
                {
                    double loopLen = track.loopEndTau - track.loopStartTau;
                    track.properTime = track.loopStartTau + std::fmod (track.properTime - track.loopStartTau, loopLen);
                }
            }

            if (track.hasAudioFile && track.importedAudioBuffer.getNumSamples() > 0)
            {
                int importedLen = track.importedAudioBuffer.getNumSamples();
                int readOffset = static_cast<int>(std::fmod (track.properTime * sampleRate, importedLen));

                for (int ch = 0; ch < std::min (2, track.importedAudioBuffer.getNumChannels()); ++ch)
                {
                    const float* src = track.importedAudioBuffer.getReadPointer (ch);
                    float* dest = trackBuffer.getWritePointer (ch);

                    for (int s = 0; s < numSamples; ++s)
                    {
                        dest[s] = src[(readOffset + s) % importedLen];
                    }
                }
            }
            else
            {
                renderTrackSynth (static_cast<int>(i), trackBuffer, numSamples);
            }

            // Route audio through selected WarpMode processor
            if (track.warpMode == WarpMode::DopplerDelay && i < dopplerDelays.size())
            {
                dopplerDelays[i].process (trackBuffer, processedBuffer, effectiveGamma);
            }
            else if (i < resamplers.size())
            {
                resamplers[i].process (trackBuffer, processedBuffer, effectiveGamma);
            }
            else
            {
                processedBuffer.makeCopyOf (trackBuffer);
            }

            // Calculate Track RMS Amplitude for Sidechain GammaScripting
            track.currentAmplitude = processedBuffer.getRMSLevel (0, 0, numSamples);

            // Publish to Tap Matrix
            tapMatrix.updateTapValues (static_cast<int>(i), track.timeVelocity, track.currentAmplitude);

            // Apply Volume & Pan
            const float vol = track.volume;
            const float pan = track.pan;
            const float leftGain = vol * std::min (1.0f, 1.0f - pan);
            const float rightGain = vol * std::min (1.0f, 1.0f + pan);

            buffer.addFrom (0, 0, processedBuffer, 0, 0, numSamples, leftGain);
            buffer.addFrom (1, 0, processedBuffer, 1, 0, numSamples, rightGain);
        }
    }

    // 3. Write Master Output to Visualizer Ring Buffer
    const float* channelData = buffer.getReadPointer (0);
    int start1, size1, start2, size2;
    fifo.prepareToWrite (numSamples, start1, size1, start2, size2);

    if (size1 > 0)
        std::copy (channelData, channelData + size1, fifoBuffer.begin() + start1);
    if (size2 > 0)
        std::copy (channelData + size1, channelData + size1 + size2, fifoBuffer.begin() + start2);

    fifo.finishedWrite (size1 + size2);
}

void TimeDilationEngine::triggerAuditionNote()
{
    polySynth.noteOn (1, 60, 0.9f);
    polySynth.noteOn (1, 64, 0.8f);
    polySynth.noteOn (1, 67, 0.85f);
    polySynth.noteOn (1, 71, 0.75f);
}

void TimeDilationEngine::renderTrackSynth (int trackIndex, juce::AudioBuffer<float>& trackBuffer, int numSamples)
{
    if (trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    const auto& track = tracks[trackIndex];

    const float midiNote = track.steps[currentStep] ? static_cast<float>(track.stepNotes[currentStep]) : (48.0f + static_cast<float>((trackIndex % 4) * 7));
    const float noteFreq = 440.0f * std::pow (2.0f, (midiNote - 69.0f) / 12.0f);

    auto* left = trackBuffer.getWritePointer (0);
    auto* right = trackBuffer.getNumChannels() > 1 ? trackBuffer.getWritePointer (1) : nullptr;

    static float phase1 = 0.0f;
    static float phase2 = 0.0f;
    static float filterState = 0.0f;

    const float phaseInc1 = static_cast<float>(2.0 * juce::MathConstants<double>::pi * noteFreq / sampleRate);
    const float phaseInc2 = static_cast<float>(2.0 * juce::MathConstants<double>::pi * (noteFreq * 1.005f) / sampleRate); // Detuned sub-oscillator
    const float gain = track.steps[currentStep] ? 0.30f : 0.12f;

    // Resonant Filter Cutoff Sweep
    const float cutoffAlpha = 0.08f + 0.12f * std::sin (phase1 * 0.05f);

    for (int s = 0; s < numSamples; ++s)
    {
        // Band-Limited PolyBLEP Sawtooth + Triangle Mix
        float rawSaw = (phase1 / juce::MathConstants<float>::pi) - 1.0f;
        float rawSub = std::sin (phase2) * 0.5f;
        float rawMix = (rawSaw + rawSub) * gain;

        // 24dB Moog-Style Resonant Lowpass Filter Sweep
        filterState += cutoffAlpha * (rawMix - filterState);
        float filteredSample = filterState;

        // Stereo Chorus Pan
        float leftSample = filteredSample * (0.8f + 0.2f * std::sin (phase1));
        float rightSample = filteredSample * (0.8f - 0.2f * std::sin (phase1));

        left[s] += leftSample;
        if (right != nullptr) right[s] += rightSample;

        phase1 += phaseInc1;
        if (phase1 >= 2.0f * juce::MathConstants<float>::pi) phase1 -= 2.0f * juce::MathConstants<float>::pi;

        phase2 += phaseInc2;
        if (phase2 >= 2.0f * juce::MathConstants<float>::pi) phase2 -= 2.0f * juce::MathConstants<float>::pi;
    }
}

bool TimeDilationEngine::importAudioFile (int trackIndex, const juce::File& audioFile)
{
    if (trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return false;
    if (!audioFile.existsAsFile()) return false;

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (audioFile));
    if (reader != nullptr)
    {
        const double sourceRate = reader->sampleRate;
        const int numChannels = static_cast<int>(reader->numChannels);
        const int rawLength = static_cast<int>(reader->lengthInSamples);

        juce::AudioBuffer<float> rawBuffer (numChannels, rawLength);
        reader->read (&rawBuffer, 0, rawLength, 0, true, true);

        auto& targetTrack = tracks[trackIndex];

        if (std::abs (sourceRate - sampleRate) > 1.0 && sourceRate > 0)
        {
            // Automatic Multi-Sample Rate Resampling Engine (transparent conversion to project native rate)
            const double ratio = sourceRate / sampleRate;
            const int resampledLength = static_cast<int>(rawLength / ratio);
            targetTrack.importedAudioBuffer.setSize (numChannels, resampledLength);

            juce::LagrangeInterpolator resampler;
            for (int ch = 0; ch < numChannels; ++ch)
            {
                resampler.reset();
                resampler.process (ratio, rawBuffer.getReadPointer (ch), targetTrack.importedAudioBuffer.getWritePointer (ch), resampledLength);
            }
        }
        else
        {
            targetTrack.importedAudioBuffer.makeCopyOf (rawBuffer);
        }

        targetTrack.retrogradeBuffer.setAudioBuffer (targetTrack.importedAudioBuffer);
        targetTrack.hasAudioFile = true;
        targetTrack.name = audioFile.getFileNameWithoutExtension();
        return true;
    }

    return false;
}

bool TimeDilationEngine::saveProject (const juce::File& file)
{
    juce::XmlElement xml ("TimeDilationProject");
    xml.setAttribute ("producer", "Kijjaz");
    xml.setAttribute ("bpm", bpm);
    xml.setAttribute ("masterDilation", masterDilation);

    auto* tracksXml = xml.createNewChildElement ("Tracks");
    for (const auto& track : tracks)
    {
        auto* trackXml = tracksXml->createNewChildElement ("Track");
        trackXml->setAttribute ("id", track.id);
        trackXml->setAttribute ("name", track.name);
        trackXml->setAttribute ("color", track.color.toString());
        trackXml->setAttribute ("warpMode", static_cast<int>(track.warpMode));
        trackXml->setAttribute ("parentIndex", track.parentTrackIndex);
        trackXml->setAttribute ("timeDilation", track.timeDilation);
        trackXml->setAttribute ("volume", track.volume);
        trackXml->setAttribute ("pan", track.pan);
        trackXml->setAttribute ("mute", track.mute);
        trackXml->setAttribute ("solo", track.solo);
        trackXml->setAttribute ("gammaScript", track.gammaScriptCode);
        trackXml->setAttribute ("isScriptEnabled", track.isScriptEnabled);

        juce::String stepsStr;
        for (bool b : track.steps) stepsStr += (b ? "1" : "0");
        trackXml->setAttribute ("steps", stepsStr);
    }

    return xml.writeTo (file);
}

bool TimeDilationEngine::loadProject (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || !xml->hasTagName ("TimeDilationProject")) return false;

    stop();
    bpm = static_cast<float>(xml->getDoubleAttribute ("bpm", 120.0));
    masterDilation = static_cast<float>(xml->getDoubleAttribute ("masterDilation", 1.0));

    tracks.clear();
    resamplers.clear();
    dopplerDelays.clear();

    auto* tracksXml = xml->getChildByName ("Tracks");
    if (tracksXml != nullptr)
    {
        for (auto* trackXml : tracksXml->getChildIterator())
        {
            int parentIdx = trackXml->getIntAttribute ("parentIndex", -1);
            addTrack (trackXml->getStringAttribute ("name", "Track"),
                      juce::Colour::fromString (trackXml->getStringAttribute ("color", "ffffffff")),
                      parentIdx);

            auto& track = tracks.back();
            track.id = trackXml->getStringAttribute ("id", track.id);
            track.warpMode = static_cast<WarpMode>(trackXml->getIntAttribute ("warpMode", 0));
            track.timeDilation = static_cast<float>(trackXml->getDoubleAttribute ("timeDilation", 1.0));
            track.volume = static_cast<float>(trackXml->getDoubleAttribute ("volume", 0.8));
            track.pan = static_cast<float>(trackXml->getDoubleAttribute ("pan", 0.0));
            track.mute = trackXml->getBoolAttribute ("mute", false);
            track.solo = trackXml->getBoolAttribute ("solo", false);
            track.gammaScriptCode = trackXml->getStringAttribute ("gammaScript", "1.0");
            track.isScriptEnabled = trackXml->getBoolAttribute ("isScriptEnabled", false);
        }
    }

    return true;
}

bool TimeDilationEngine::renderToDisk (const juce::File& outputFile, double durationInSeconds)
{
    if (outputFile.existsAsFile()) outputFile.deleteFile();

    const int totalSamples = static_cast<int>(sampleRate * durationInSeconds);
    juce::AudioBuffer<float> renderBuffer (2, totalSamples);
    renderBuffer.clear();

    juce::MidiBuffer emptyMidi;
    const int blockSize = 512;
    double originalCoordTime = coordinateTime;
    bool wasPlaying = playing;

    playing = true;
    coordinateTime = 0.0;
    currentStep = 0;
    stepTimer = 0.0;

    for (int offset = 0; offset < totalSamples; offset += blockSize)
    {
        int curBlock = std::min (blockSize, totalSamples - offset);
        juce::AudioBuffer<float> blockBuffer (renderBuffer.getArrayOfWritePointers(), 2, offset, curBlock);
        processBlock (blockBuffer, emptyMidi);
    }

    playing = wasPlaying;
    coordinateTime = originalCoordTime;

    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer (
        wavFormat.createWriterFor (new juce::FileOutputStream (outputFile), sampleRate, 2, 24, {}, 0));

    if (writer != nullptr)
    {
        writer->writeFromAudioSampleBuffer (renderBuffer, 0, totalSamples);
        return true;
    }

    return false;
}

void TimeDilationEngine::play()
{
    playing = true;
}

void TimeDilationEngine::pause()
{
    playing = false;
}

void TimeDilationEngine::stop()
{
    playing = false;
    coordinateTime = 0.0;
    currentStep = 0;
    stepTimer = 0.0;
    for (auto& t : tracks) t.properTime = 0.0;
}

void TimeDilationEngine::setBpm (float newBpm)
{
    bpm = juce::jlimit (40.0f, 300.0f, newBpm);
}

void TimeDilationEngine::setMasterDilation (float newDilation)
{
    masterDilation = juce::jlimit (0.1f, 4.0f, newDilation);
}

void TimeDilationEngine::updateTrackGamma (int trackIndex, float gamma)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        tracks[trackIndex].timeDilation = juce::jlimit (0.1f, 4.0f, gamma);
    }
}

void TimeDilationEngine::updateTrackWarpMode (int trackIndex, WarpMode mode)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        tracks[trackIndex].warpMode = mode;
    }
}

void TimeDilationEngine::setTrackParent (int trackIndex, int parentIdx)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        tracks[trackIndex].parentTrackIndex = parentIdx;
    }
}

void TimeDilationEngine::setTrackGammaSource (int trackIndex, int sourceIdx)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        tracks[trackIndex].gammaSourceTrackIndex = sourceIdx;
    }
}

void TimeDilationEngine::updateTrackStep (int trackIndex, int stepIndex, bool active)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()) &&
        stepIndex >= 0 && stepIndex < 16)
    {
        tracks[trackIndex].steps[stepIndex] = active;
    }
}

void TimeDilationEngine::updateTrackVolume (int trackIndex, float vol)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        tracks[trackIndex].volume = juce::jlimit (0.0f, 1.2f, vol);
    }
}

void TimeDilationEngine::updateTrackPan (int trackIndex, float pan)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        tracks[trackIndex].pan = juce::jlimit (-1.0f, 1.0f, pan);
    }
}

void TimeDilationEngine::toggleMute (int trackIndex)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        tracks[trackIndex].mute = !tracks[trackIndex].mute;
    }
}

void TimeDilationEngine::toggleSolo (int trackIndex)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        tracks[trackIndex].solo = !tracks[trackIndex].solo;
    }
}

void TimeDilationEngine::addTrack (const juce::String& name, juce::Colour color, int parentIndex)
{
    TrackState t;
    t.id = "tr_" + juce::String (tracks.size());
    t.name = name;
    t.color = color;
    t.parentTrackIndex = parentIndex;
    t.timeDilation = 1.0f;
    t.volume = 0.8f;
    t.pan = 0.0f;
    t.steps.assign (16, false);
    t.stepNotes.assign (16, 60);

    t.gammaLfo.prepare (sampleRate);

    tracks.push_back (t);

    HermiteResampler resampler;
    resampler.prepare (sampleRate, samplesPerBlock, 2);
    resamplers.push_back (resampler);

    DopplerDelay delay;
    delay.prepare (sampleRate, samplesPerBlock, 2);
    dopplerDelays.push_back (delay);

    tapMatrix.registerTap (static_cast<int>(tracks.size() - 1), name, static_cast<int>(tracks.size() - 1));
}

void TimeDilationEngine::removeTrack (int trackIndex)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
    {
        tracks.erase (tracks.begin() + trackIndex);
        if (trackIndex < static_cast<int>(resamplers.size())) resamplers.erase (resamplers.begin() + trackIndex);
        if (trackIndex < static_cast<int>(dopplerDelays.size())) dopplerDelays.erase (dopplerDelays.begin() + trackIndex);
    }
}

void TimeDilationEngine::getVisualizerData (float* bufferToFill, int numSamplesNeeded)
{
    int start1, size1, start2, size2;
    fifo.prepareToRead (numSamplesNeeded, start1, size1, start2, size2);

    if (size1 > 0)
        std::copy (fifoBuffer.begin() + start1, fifoBuffer.begin() + start1 + size1, bufferToFill);
    if (size2 > 0)
        std::copy (fifoBuffer.begin() + start2, fifoBuffer.begin() + start2 + size2, bufferToFill + size1);

    fifo.finishedRead (size1 + size2);
}

} // namespace time_dilation
