#include "PianoRollComponent.h"

namespace time_dilation
{

PianoRollComponent::PianoRollComponent (TimeDilationEngine& e)
    : engine (e)
{
    // Populate default pentatonic synth melo-chords
    notes.push_back ({ 60, 0.0f, 1.0f, 0.8f });
    notes.push_back ({ 64, 1.0f, 1.0f, 0.85f });
    notes.push_back ({ 67, 2.0f, 1.0f, 0.9f });
    notes.push_back ({ 72, 3.0f, 1.0f, 0.95f });
    notes.push_back ({ 69, 4.0f, 1.0f, 0.8f });
    notes.push_back ({ 67, 5.0f, 1.0f, 0.85f });
}

void PianoRollComponent::setSelectedTrack (int trackIndex)
{
    selectedTrackIndex = trackIndex;
    repaint();
}

void PianoRollComponent::mouseDown (const juce::MouseEvent& e)
{
    const float keyboardWidth = 40.0f;
    if (e.position.x > keyboardWidth)
    {
        float beat = (e.position.x - keyboardWidth) / pixelsPerBeat;
        int noteNum = 127 - static_cast<int>(e.position.y / keyHeight);
        noteNum = juce::jlimit (0, 127, noteNum);

        notes.push_back ({ noteNum, std::floor (beat * 2.0f) / 2.0f, 1.0f, 0.85f });
        engine.triggerAuditionNote();
        repaint();
    }
}

void PianoRollComponent::mouseDrag (const juce::MouseEvent& /*e*/)
{
}

void PianoRollComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0f141d));

    const float keyboardWidth = 40.0f;
    const float width = getWidth() - keyboardWidth;
    const float height = getHeight();

    // Draw Left Piano Keys
    for (int n = 0; n < 128; ++n)
    {
        float y = (127 - n) * keyHeight;
        if (y > height) continue;

        bool isBlackKey = (n % 12 == 1 || n % 12 == 3 || n % 12 == 6 || n % 12 == 8 || n % 12 == 10);
        g.setColour (isBlackKey ? juce::Colour (0xff1e293b) : juce::Colour (0xfff8fafc));
        g.fillRect (0.0f, y, keyboardWidth - 1.0f, keyHeight - 1.0f);

        if (n % 12 == 0) // C key label
        {
            g.setColour (juce::Colour (0xff0f172a));
            g.setFont (juce::FontOptions (8.0f, juce::Font::bold));
            g.drawText ("C" + juce::String (n / 12 - 1), 2.0f, y, 35.0f, keyHeight, juce::Justification::left);
        }
    }

    // Draw Grid Lanes
    g.setColour (juce::Colour (0x15334155));
    const int maxBeats = static_cast<int>(width / pixelsPerBeat) + 1;
    for (int b = 0; b < maxBeats; ++b)
    {
        float x = keyboardWidth + b * pixelsPerBeat;
        g.drawLine (x, 0.0f, x, height, (b % 4 == 0) ? 1.5f : 0.5f);
    }

    // Draw MIDI Note Blocks
    for (const auto& note : notes)
    {
        float x = keyboardWidth + note.startBeat * pixelsPerBeat;
        float y = (127 - note.noteNumber) * keyHeight;
        float w = note.durationBeats * pixelsPerBeat;

        g.setColour (juce::Colour (0xfff59e0b));
        g.fillRoundedRectangle (x + 1.0f, y + 1.0f, w - 2.0f, keyHeight - 2.0f, 2.0f);

        g.setColour (juce::Colour (0xffffffff));
        g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
        g.drawText (juce::MidiMessage::getMidiNoteName (note.noteNumber, true, true, 3), x + 3.0f, y, w, keyHeight, juce::Justification::left);
    }
}

void PianoRollComponent::resized()
{
}

} // namespace time_dilation
