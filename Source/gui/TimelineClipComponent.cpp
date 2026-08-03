#include "TimelineClipComponent.h"

namespace time_dilation
{

TimelineClipComponent::TimelineClipComponent (TimeDilationEngine& e)
    : engine (e)
{
    // Populate default audio & MIDI clips on tracks
    clips.push_back ({ "c1", "SYNTH LEAD RHYTHM", 0, 0.0f, 4.0f, 1.0f, juce::Colour (0xfff59e0b), false });
    clips.push_back ({ "c2", "SYNTH ARPEGGIO", 0, 4.0f, 4.0f, 1.0f, juce::Colour (0xfff59e0b), false });
    clips.push_back ({ "c3", "VARISPEED AUDIO SAMPLE", 1, 0.0f, 8.0f, 1.0f, juce::Colour (0xff8b5cf6), true });
    clips.push_back ({ "c4", "SUB-BASS GROOVE", 2, 0.0f, 6.0f, 1.0f, juce::Colour (0xff06b6d4), false });
}

void TimelineClipComponent::mouseDown (const juce::MouseEvent& e)
{
    auto pt = e.position;
    draggedClipIdx = -1;

    for (size_t i = 0; i < clips.size(); ++i)
    {
        auto& clip = clips[i];
        float clipX = clip.startBeat * pixelsPerBeat;
        float clipY = clip.trackIndex * trackRowHeight;
        float clipW = clip.lengthBeats * pixelsPerBeat;
        float clipH = trackRowHeight - 4.0f;

        juce::Rectangle<float> clipBounds (clipX, clipY, clipW, clipH);
        juce::Rectangle<float> warpHandleBounds (clipX + clipW * 0.4f, clipY, clipW * 0.2f, 12.0f);
        juce::Rectangle<float> rightEdgeBounds (clipX + clipW - 8.0f, clipY, 8.0f, clipH);

        if (warpHandleBounds.contains (pt))
        {
            draggedClipIdx = static_cast<int>(i);
            isDraggingWarpHandle = true;
            dragStartPos = pt;
            clipStartSnapshot = clip;
            return;
        }
        else if (rightEdgeBounds.contains (pt))
        {
            draggedClipIdx = static_cast<int>(i);
            isResizingEdge = true;
            dragStartPos = pt;
            clipStartSnapshot = clip;
            return;
        }
        else if (clipBounds.contains (pt))
        {
            draggedClipIdx = static_cast<int>(i);
            isDraggingWarpHandle = false;
            isResizingEdge = false;
            dragStartPos = pt;
            clipStartSnapshot = clip;
            return;
        }
    }
}

void TimelineClipComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (draggedClipIdx < 0 || draggedClipIdx >= static_cast<int>(clips.size())) return;

    auto delta = e.position - dragStartPos;
    auto& clip = clips[draggedClipIdx];

    if (isDraggingWarpHandle)
    {
        // Dragging Warp Handle adjusts clip length & gamma in real-time!
        float deltaBeats = delta.x / pixelsPerBeat;
        float newLength = juce::jmax (0.5f, clipStartSnapshot.lengthBeats + deltaBeats);
        clip.lengthBeats = newLength;
        clip.gamma = clipStartSnapshot.lengthBeats / newLength; // Stretching length dilates gamma!
        engine.updateTrackGamma (clip.trackIndex, clip.gamma);
        repaint();
    }
    else if (isResizingEdge)
    {
        float deltaBeats = delta.x / pixelsPerBeat;
        clip.lengthBeats = juce::jmax (0.5f, clipStartSnapshot.lengthBeats + deltaBeats);
        repaint();
    }
    else
    {
        // Dragging clip moves its startBeat position
        float deltaBeats = delta.x / pixelsPerBeat;
        clip.startBeat = juce::jmax (0.0f, clipStartSnapshot.startBeat + deltaBeats);
        repaint();
    }
}

void TimelineClipComponent::mouseUp (const juce::MouseEvent& /*e*/)
{
    draggedClipIdx = -1;
    isDraggingWarpHandle = false;
    isResizingEdge = false;
}

void TimelineClipComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0x00000000)); // Transparent background overlaying grid

    for (size_t i = 0; i < clips.size(); ++i)
    {
        const auto& clip = clips[i];
        float clipX = clip.startBeat * pixelsPerBeat;
        float clipY = clip.trackIndex * trackRowHeight;
        float clipW = clip.lengthBeats * pixelsPerBeat;
        float clipH = trackRowHeight - 4.0f;

        // Clip Card Body
        g.setColour (clip.color.withAlpha (0.25f));
        g.fillRoundedRectangle (clipX, clipY, clipW, clipH, 4.0f);

        g.setColour (clip.color);
        g.drawRoundedRectangle (clipX, clipY, clipW, clipH, 4.0f, 1.5f);

        // Header Strip
        g.setColour (clip.color.withAlpha (0.6f));
        g.fillRect (clipX, clipY, clipW, 14.0f);

        // Clip Title & Gamma Label
        g.setColour (juce::Colour (0xffffffff));
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        juce::String gamStr = " (" + juce::String (clip.gamma, 2) + "x)";
        g.drawText (clip.name + gamStr, clipX + 4.0f, clipY + 1.0f, clipW - 8.0f, 12.0f, juce::Justification::left);

        // Render Draggable WARP Handle in Center Top
        float handleW = juce::jmin (60.0f, clipW * 0.4f);
        float handleX = clipX + (clipW - handleW) * 0.5f;
        g.setColour (juce::Colour (0xfff59e0b));
        g.fillRoundedRectangle (handleX, clipY + 1.0f, handleW, 12.0f, 2.0f);

        g.setColour (juce::Colour (0xff0f141d));
        g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
        g.drawText ("WARP", handleX, clipY + 1.0f, handleW, 12.0f, juce::Justification::centred);

        // Draw Waveform or Note Blocks inside Clip
        if (clip.isAudio)
        {
            drawWaveform (g, { clipX + 4.0f, clipY + 16.0f, clipW - 8.0f, clipH - 18.0f }, clip.color);
        }
        else
        {
            // Draw MIDI Note Blocks inside Clip
            g.setColour (clip.color.withAlpha (0.9f));
            float noteW = clipW / 8.0f;
            for (int n = 0; n < 8; ++n)
            {
                float noteY = clipY + 16.0f + (n % 4) * 7.0f;
                g.fillRect (clipX + n * noteW + 2.0f, noteY, noteW - 3.0f, 5.0f);
            }
        }
    }
}

void TimelineClipComponent::drawWaveform (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour color)
{
    g.setColour (color);
    juce::Path p;
    p.startNewSubPath (bounds.getX(), bounds.getCentreY());

    const int points = static_cast<int>(bounds.getWidth() / 3.0f);
    for (int i = 0; i < points; ++i)
    {
        float x = bounds.getX() + i * 3.0f;
        float h = (sin (i * 0.4f) * cos (i * 0.15f)) * (bounds.getHeight() * 0.45f);
        p.lineTo (x, bounds.getCentreY() + h);
    }
    g.strokePath (p, juce::PathStrokeType (1.5f));
}

void TimelineClipComponent::resized()
{
}

} // namespace time_dilation
