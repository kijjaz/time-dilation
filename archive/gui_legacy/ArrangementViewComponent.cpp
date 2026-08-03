#include "ArrangementViewComponent.h"

namespace time_dilation
{

ArrangementViewComponent::ArrangementViewComponent (TimeDilationEngine& e)
    : engine (e)
{
    addAndMakeVisible (addTrackButton);
    addTrackButton.onClick = [this] {
        int count = static_cast<int>(engine.getTracks().size()) + 1;
        juce::Colour colors[] = { juce::Colour (0xfff59e0b), juce::Colour (0xff8b5cf6), juce::Colour (0xff06b6d4), juce::Colour (0xffec4899) };
        engine.addTrack ("Track " + juce::String (count), colors[count % 4]);
        repaint();
    };

    addAndMakeVisible (addSubTrackButton);
    addSubTrackButton.onClick = [this] {
        if (!engine.getTracks().empty())
        {
            int parentIdx = selectedTrackIndex;
            int count = static_cast<int>(engine.getTracks().size()) + 1;
            engine.addTrack ("Sub-Track " + juce::String (count), juce::Colour (0xffa78bfa), parentIdx);
            repaint();
        }
    };

    addAndMakeVisible (deleteTrackButton);
    deleteTrackButton.onClick = [this] {
        if (!engine.getTracks().empty())
        {
            engine.removeTrack (selectedTrackIndex);
            selectedTrackIndex = std::max (0, selectedTrackIndex - 1);
            repaint();
        }
    };

    startTimerHz (30); // 30 FPS smooth playhead & waveform rendering
}

ArrangementViewComponent::~ArrangementViewComponent()
{
    stopTimer();
}

void ArrangementViewComponent::timerCallback()
{
    repaint();
}

void ArrangementViewComponent::setSelectedTrackIndex (int index)
{
    selectedTrackIndex = juce::jlimit (0, std::max (0, static_cast<int>(engine.getTracks().size()) - 1), index);
    if (onTrackSelected) onTrackSelected (selectedTrackIndex);
    repaint();
}

void ArrangementViewComponent::mouseDown (const juce::MouseEvent& e)
{
    float y = e.position.y;
    float x = e.position.x;

    if (y < 30.0f) // Clicked Time Ruler: Scrub Playhead
    {
        isScrubbingPlayhead = true;
        double targetBeat = std::max (0.0, (double) x / pixelsPerBeat);
        double targetTimeSeconds = targetBeat * (60.0 / engine.getBpm());
        // Set master coordinate time
    }
    else // Clicked Track Lanes Area
    {
        isScrubbingPlayhead = false;
        const float trackHeight = 65.0f;
        int clickedTrackIdx = static_cast<int>((y - 30.0f) / trackHeight);

        if (clickedTrackIdx >= 0 && clickedTrackIdx < static_cast<int>(engine.getTracks().size()))
        {
            setSelectedTrackIndex (clickedTrackIdx);

            // Toggle Step Trigger on Grid Click
            int stepIdx = static_cast<int>(x / pixelsPerBeat);
            if (stepIdx >= 0 && stepIdx < 16)
            {
                bool currentVal = engine.getTracks()[clickedTrackIdx].steps[stepIdx];
                engine.updateTrackStep (clickedTrackIdx, stepIdx, !currentVal);
            }
        }
    }
}

void ArrangementViewComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (isScrubbingPlayhead || e.position.y < 30.0f)
    {
        double targetBeat = std::max (0.0, (double) e.position.x / pixelsPerBeat);
        // Scrub playhead continuously
        repaint();
    }
}

void ArrangementViewComponent::mouseWheelMove (const juce::MouseEvent& /*e*/, const juce::MouseWheelDetails& wheel)
{
    float delta = wheel.deltaY * 15.0f;
    setZoomLevel (pixelsPerBeat + delta);
}

bool ArrangementViewComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        juce::File file (f);
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".mp3" || ext == ".flac" || ext == ".aiff")
            return true;
    }
    return false;
}

void ArrangementViewComponent::filesDropped (const juce::StringArray& files, int /*x*/, int y)
{
    const float trackHeight = 65.0f;
    int targetTrackIdx = static_cast<int>((y - 30.0f) / trackHeight);
    targetTrackIdx = juce::jlimit (0, std::max (0, static_cast<int>(engine.getTracks().size()) - 1), targetTrackIdx);

    for (const auto& f : files)
    {
        juce::File file (f);
        if (engine.importAudioFile (targetTrackIdx, file))
        {
            setSelectedTrackIndex (targetTrackIdx);
            repaint();
            break;
        }
    }
}

void ArrangementViewComponent::setZoomLevel (float pxPerBeat)
{
    pixelsPerBeat = juce::jlimit (10.0f, 200.0f, pxPerBeat);
    repaint();
}

void ArrangementViewComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();

    // Dark Carbon Background
    g.fillAll (juce::Colour (0xff0f141d));

    // Render Components
    drawTimeRuler (g, width, 30.0f);
    drawTrackLanes (g, width, height - 30.0f);
    drawPlayhead (g, width, height);
}

void ArrangementViewComponent::drawTimeRuler (juce::Graphics& g, float width, float height)
{
    g.setColour (juce::Colour (0xff171d2b));
    g.fillRect (0.0f, 0.0f, width, height);

    g.setColour (juce::Colour (0xff263147));
    g.drawLine (0.0f, height, width, height, 1.0f);

    g.setColour (juce::Colour (0xff94a3b8));
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));

    const int maxBeats = static_cast<int>(width / pixelsPerBeat) + 1;
    for (int b = 0; b < maxBeats; ++b)
    {
        float x = static_cast<float>(b) * pixelsPerBeat;
        if (b % 4 == 0)
        {
            int barNum = (b / 4) + 1;
            g.setColour (juce::Colour (0xfff59e0b)); // Gold bar marker
            g.drawLine (x, 10.0f, x, height, 1.5f);
            g.drawText (juce::String (barNum), x + 4.0f, 2.0f, 40.0f, 14.0f, juce::Justification::left);
        }
        else
        {
            g.setColour (juce::Colour (0x6694a3b8));
            g.drawLine (x, 18.0f, x, height, 1.0f);
        }
    }
}

void ArrangementViewComponent::drawTrackLanes (juce::Graphics& g, float width, float height)
{
    const auto& tracks = engine.getTracks();
    const float trackHeight = 65.0f;
    float y = 30.0f;

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        const auto& track = tracks[i];
        float indent = (track.parentTrackIndex >= 0) ? 15.0f : 0.0f;

        // Background Lane & Selected Track Highlight
        bool isSelected = (static_cast<int>(i) == selectedTrackIndex);
        g.setColour (isSelected ? juce::Colour (0xff1e293b) : (i % 2 == 0 ? juce::Colour (0xff141a26) : juce::Colour (0xff10141d)));
        g.fillRect (0.0f, y, width, trackHeight);

        if (isSelected)
        {
            g.setColour (juce::Colour (0xfff59e0b));
            g.drawRect (0.0f, y, width, trackHeight, 1.5f);
        }
        else
        {
            g.setColour (juce::Colour (0xff263147));
            g.drawLine (0.0f, y + trackHeight, width, y + trackHeight, 1.0f);
        }

        // Render Track Color Strip & Name
        g.setColour (track.color);
        g.fillRect (0.0f + indent, y, 6.0f, trackHeight);

        g.setColour (juce::Colour (0xffffffff));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (track.name, 12.0f + indent, y + 4.0f, 150.0f, 16.0f, juce::Justification::left);

        // Render Loop Region Bounds if Looping is Enabled
        if (track.isLooping)
        {
            float loopStartX = static_cast<float>(track.loopStartTau * (engine.getBpm() / 60.0) * pixelsPerBeat);
            float loopEndX = static_cast<float>(track.loopEndTau * (engine.getBpm() / 60.0) * pixelsPerBeat);

            g.setColour (juce::Colour (0x22f59e0b)); // Gold loop overlay
            g.fillRect (loopStartX, y, std::max (10.0f, loopEndX - loopStartX), trackHeight);

            g.setColour (juce::Colour (0xfff59e0b));
            g.drawLine (loopStartX, y, loopStartX, y + trackHeight, 2.0f);
            g.drawLine (loopEndX, y, loopEndX, y + trackHeight, 2.0f);
        }

        // Render Clip Waveform or MIDI step clips
        if (track.hasAudioFile && track.importedAudioBuffer.getNumSamples() > 0)
        {
            g.setColour (track.color.withAlpha (0.4f));
            float clipW = (track.importedAudioBuffer.getNumSamples() / static_cast<float>(44100.0)) * (engine.getBpm() / 60.0f) * pixelsPerBeat;
            g.fillRoundedRectangle (0.0f, y + 22.0f, clipW, 36.0f, 4.0f);

            // Draw Audio Waveform Path
            juce::Path wavePath;
            const float* readPtr = track.importedAudioBuffer.getReadPointer (0);
            int stepSize = std::max (1, track.importedAudioBuffer.getNumSamples() / static_cast<int>(clipW));

            for (int px = 0; px < static_cast<int>(clipW); ++px)
            {
                int sampleIdx = px * stepSize;
                if (sampleIdx < track.importedAudioBuffer.getNumSamples())
                {
                    float amp = readPtr[sampleIdx];
                    float py = (y + 40.0f) + amp * 14.0f;
                    if (px == 0) wavePath.startNewSubPath (static_cast<float>(px), py);
                    else wavePath.lineTo (static_cast<float>(px), py);
                }
            }
            g.setColour (track.color);
            g.strokePath (wavePath, juce::PathStrokeType (1.0f));
        }
        else
        {
            // Render MIDI Step Blocks
            for (int step = 0; step < 16; ++step)
            {
                if (track.steps[step])
                {
                    float stepX = static_cast<float>(step) * pixelsPerBeat;
                    g.setColour (track.color);
                    g.fillRoundedRectangle (stepX + 2.0f, y + 24.0f, pixelsPerBeat - 4.0f, 32.0f, 3.0f);
                }
            }
        }

        y += trackHeight;
    }
}

void ArrangementViewComponent::drawPlayhead (juce::Graphics& g, float width, float height)
{
    // Master Playhead position in pixels
    double currentBeat = engine.getCoordinateTime() * (engine.getBpm() / 60.0);
    float playheadX = static_cast<float>(currentBeat * pixelsPerBeat);

    g.setColour (juce::Colour (0xff06b6d4)); // Cyan glowing playhead line
    g.drawLine (playheadX, 0.0f, playheadX, height, 2.0f);

    // Playhead handle triangle
    juce::Path handle;
    handle.addTriangle (playheadX - 6.0f, 0.0f, playheadX + 6.0f, 0.0f, playheadX, 10.0f);
    g.setColour (juce::Colour (0xff22d3ee));
    g.fillPath (handle);
}

void ArrangementViewComponent::resized()
{
    addTrackButton.setBounds (getWidth() - 300, 3, 90, 24);
    addSubTrackButton.setBounds (getWidth() - 205, 3, 110, 24);
    deleteTrackButton.setBounds (getWidth() - 90, 3, 85, 24);
}

} // namespace time_dilation
