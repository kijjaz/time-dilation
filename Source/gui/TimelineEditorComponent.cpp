#include "TimelineEditorComponent.h"
#include "FontManager.h"

namespace time_dilation
{

TimelineEditorComponent::TimelineEditorComponent (TimelineNode& timelineNode)
    : node (timelineNode)
{
    addAndMakeVisible (addAudioTrackBtn);
    addAndMakeVisible (addMidiTrackBtn);
    addAndMakeVisible (addDilTrackBtn);
    addAndMakeVisible (recordArmToggle);
    addAndMakeVisible (bpmSlider);
    addAndMakeVisible (bpmLabel);

    bpmSlider.setRange (20.0, 300.0, 1.0);
    bpmSlider.setValue (node.getParameter ("bpm", 120.0f));
    bpmSlider.onValueChange = [this] { node.setParameter ("bpm", static_cast<float>(bpmSlider.getValue())); };

    addAudioTrackBtn.onClick = [this] {
        node.addTrack ("Audio Track " + std::to_string (node.getTracks().size() + 1), TrackType::Audio);
        repaint();
    };

    addMidiTrackBtn.onClick = [this] {
        node.addTrack ("MIDI Track " + std::to_string (node.getTracks().size() + 1), TrackType::Midi);
        repaint();
    };

    addDilTrackBtn.onClick = [this] {
        node.addTrack ("Time Dilation Track", TrackType::TimeDilation);
        repaint();
    };

    recordArmToggle.setToggleState (node.getParameter ("isArmed", 0.0f) > 0.5f, juce::dontSendNotification);
    recordArmToggle.onClick = [this] {
        node.setParameter ("isArmed", recordArmToggle.getToggleState() ? 1.0f : 0.0f);
    };

    startTimerHz (30);
    setSize (840, 520);
}

TimelineEditorComponent::~TimelineEditorComponent()
{
    stopTimer();
}

void TimelineEditorComponent::timerCallback()
{
    repaint();
}

void TimelineEditorComponent::resized()
{
    auto area = getLocalBounds().reduced (12);
    auto topRow = area.removeFromTop (36);

    addAudioTrackBtn.setBounds (topRow.removeFromLeft (110));
    topRow.removeFromLeft (8);
    addMidiTrackBtn.setBounds (topRow.removeFromLeft (105));
    topRow.removeFromLeft (8);
    addDilTrackBtn.setBounds (topRow.removeFromLeft (150));
    topRow.removeFromLeft (16);

    recordArmToggle.setBounds (topRow.removeFromLeft (90));
    topRow.removeFromLeft (16);

    bpmLabel.setBounds (topRow.removeFromLeft (36));
    bpmSlider.setBounds (topRow.removeFromLeft (100));
}

void TimelineEditorComponent::paint (juce::Graphics& g)
{
    // Deep Sci-Fi Slate background
    g.fillAll (juce::Colour (0xff070a12));

    auto bounds = getLocalBounds().toFloat();
    float topBarH = 50.0f;
    float rulerH = 26.0f;

    // Header bar border
    g.setColour (juce::Colour (0xff1e293b));
    g.drawHorizontalLine (static_cast<int>(topBarH), 0.0f, bounds.getWidth());

    // Ruler background
    juce::Rectangle<float> rulerRect (headerWidth, topBarH, bounds.getWidth() - headerWidth, rulerH);
    g.setColour (juce::Colour (0xff0f172a));
    g.fillRect (rulerRect);

    // Draw Bar/Beat Grid lines
    g.setFont (FontManager::getInstance().getOxaniumFont (10.0f, false));
    float startX = headerWidth;
    for (int b = 0; b < 64; ++b)
    {
        float x = startX + b * zoomFactor;
        if (x > bounds.getWidth()) break;

        if (b % 4 == 0)
        {
            g.setColour (juce::Colour (0xff334155));
            g.drawVerticalLine (static_cast<int>(x), topBarH, bounds.getHeight());
            g.setColour (juce::Colour (0xff94a3b8));
            g.drawText (std::to_string (b / 4 + 1), x + 3.0f, topBarH + 2.0f, 30.0f, 16.0f, juce::Justification::left);
        }
        else
        {
            g.setColour (juce::Colour (0xff1e293b));
            g.drawVerticalLine (static_cast<int>(x), topBarH + rulerH, bounds.getHeight());
        }
    }

    // Render Track Lanes
    const auto& tracks = node.getTracks();
    float trackY = topBarH + rulerH;

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        const auto& trk = tracks[i];
        juce::Rectangle<float> trackHeader (0.0f, trackY, headerWidth, trackHeight);
        juce::Rectangle<float> trackLane (headerWidth, trackY, bounds.getWidth() - headerWidth, trackHeight);

        // Header Card
        g.setColour (juce::Colour (0xff0f172a));
        g.fillRect (trackHeader);
        g.setColour (juce::Colour (0xff1e293b));
        g.drawRect (trackHeader, 1.0f);

        // Track Name & Color Accent
        juce::Colour trackCol = (trk.getType() == TrackType::Audio) ? juce::Colour (0xff06b6d4) : (trk.getType() == TrackType::TimeDilation ? juce::Colour (0xff8b5cf6) : juce::Colour (0xfff59e0b));
        g.setColour (trackCol);
        g.fillRect (0.0f, trackY, 4.0f, trackHeight);

        g.setColour (juce::Colours::white);
        g.setFont (FontManager::getInstance().getOxaniumFont (12.0f, true));
        g.drawText (trk.getName(), 10.0f, trackY + 8.0f, headerWidth - 15.0f, 18.0f, juce::Justification::left);

        // Track Lane background
        g.setColour (i % 2 == 0 ? juce::Colour (0xff0b0f19) : juce::Colour (0xff080c14));
        g.fillRect (trackLane);
        g.setColour (juce::Colour (0xff1e293b));
        g.drawRect (trackLane, 1.0f);

        // Render Clips / Events
        if (trk.getType() == TrackType::Audio)
        {
            for (const auto& clip : trk.getAudioClips())
            {
                float clipX = headerWidth + static_cast<float>(clip.startBeat) * zoomFactor;
                float clipW = static_cast<float>(clip.durationBeats) * zoomFactor;
                juce::Rectangle<float> clipRect (clipX, trackY + 4.0f, clipW, trackHeight - 8.0f);

                g.setColour (juce::Colour (0xff0284c7).withAlpha (0.4f));
                g.fillRoundedRectangle (clipRect, 4.0f);
                g.setColour (juce::Colour (0xff38bdf8));
                g.drawRoundedRectangle (clipRect, 4.0f, 1.0f);

                g.setFont (FontManager::getInstance().getOxaniumFont (10.0f, false));
                g.drawText (clip.clipName, clipRect.reduced (4.0f), juce::Justification::topLeft);
            }
        }
        else if (trk.getType() == TrackType::TimeDilation)
        {
            juce::Path dilPath;
            bool first = true;
            for (const auto& pt : trk.getDilationPoints())
            {
                float ptX = headerWidth + static_cast<float>(pt.beat) * zoomFactor;
                float normGamma = std::clamp ((pt.gamma - 0.25f) / 3.75f, 0.0f, 1.0f);
                float ptY = trackY + trackHeight - normGamma * (trackHeight - 12.0f) - 6.0f;

                if (first) { dilPath.startNewSubPath (ptX, ptY); first = false; }
                else { dilPath.lineTo (ptX, ptY); }

                g.setColour (juce::Colour (0xffa855f7));
                g.fillEllipse (ptX - 3.0f, ptY - 3.0f, 6.0f, 6.0f);
            }

            g.setColour (juce::Colour (0xffc084fc));
            g.strokePath (dilPath, juce::PathStrokeType (2.0f));
        }

        trackY += trackHeight + 2.0f;
    }

    // Draw Live Transport Playhead Line
    double playheadBeat = node.getCurrentPlayheadBeat();
    float playheadX = headerWidth + static_cast<float>(playheadBeat) * zoomFactor;
    if (playheadX >= headerWidth && playheadX <= bounds.getWidth())
    {
        g.setColour (juce::Colour (0xfff59e0b));
        g.drawVerticalLine (static_cast<int>(playheadX), topBarH, bounds.getHeight());

        // Playhead head indicator
        juce::Path headTriangle;
        headTriangle.addTriangle (playheadX - 6.0f, topBarH, playheadX + 6.0f, topBarH, playheadX, topBarH + 8.0f);
        g.fillPath (headTriangle);
    }
}

void TimelineEditorComponent::mouseDown (const juce::MouseEvent& e)
{
    if (e.x > headerWidth && e.y > 50.0f && e.y < 76.0f)
    {
        double clickedBeat = (e.x - headerWidth) / zoomFactor;
        node.setPlayheadBeat (clickedBeat);
        repaint();
    }
}

void TimelineEditorComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (e.x > headerWidth && e.y > 50.0f && e.y < 76.0f)
    {
        double clickedBeat = std::max (0.0, static_cast<double>((e.x - headerWidth) / zoomFactor));
        node.setPlayheadBeat (clickedBeat);
        repaint();
    }
}

} // namespace time_dilation
