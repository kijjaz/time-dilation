#include "AnsiBoxStudioComponent.h"

namespace time_dilation
{

AnsiBoxStudioComponent::AnsiBoxStudioComponent (TimeDilationEngine& e)
    : engine (e)
{
    // Populate Initial ANSI Box Objects
    objects.push_back ({ "midi_1", "MIDI 2.0 PLAYER", AnsiObjectType::MidiPlayer, { 30.0f, 60.0f, 220.0f, 130.0f }, juce::Colour (0xffa855f7), 1.0f, 0.8f, false, { "synth_1" } });
    objects.push_back ({ "synth_1", "SYNTH / SAMPLER", AnsiObjectType::SynthSampler, { 280.0f, 60.0f, 240.0f, 150.0f }, juce::Colour (0xff00ff66), 1.0f, 0.85f, false, {} });
    objects.push_back ({ "audio_1", "AUDIO EVENT BUFFER", AnsiObjectType::AudioEvent, { 550.0f, 60.0f, 240.0f, 130.0f }, juce::Colour (0xff00e5ff), 1.0f, 0.9f, false, {} });
    objects.push_back ({ "time_gen_1", "TIME GENERATOR (GAMMA)", AnsiObjectType::TimeGenerator, { 280.0f, 250.0f, 260.0f, 120.0f }, juce::Colour (0xffffb000), 1.0f, 1.0f, false, { "synth_1", "audio_1" } }); // Fan-out tapping to Synth & Audio!

    // Setup ANSI Action Buttons
    auto setupAnsiBtn = [this] (juce::TextButton& btn, juce::Colour c) {
        addAndMakeVisible (btn);
        btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f0f0f));
        btn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff1e293b));
        btn.setColour (juce::TextButton::textColourOffId, c);
        btn.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffffffff));
    };

    setupAnsiBtn (btnAddMidi, juce::Colour (0xffa855f7));
    btnAddMidi.onClick = [this] { addObject (AnsiObjectType::MidiPlayer); };

    setupAnsiBtn (btnAddSynth, juce::Colour (0xff00ff66));
    btnAddSynth.onClick = [this] { addObject (AnsiObjectType::SynthSampler); };

    setupAnsiBtn (btnAddAudio, juce::Colour (0xff00e5ff));
    btnAddAudio.onClick = [this] { addObject (AnsiObjectType::AudioEvent); };

    setupAnsiBtn (btnAddTimeGen, juce::Colour (0xffffb000));
    btnAddTimeGen.onClick = [this] { addObject (AnsiObjectType::TimeGenerator); };

    setupAnsiBtn (btnPlay, juce::Colour (0xff00ff66));
    btnPlay.onClick = [this] { engine.play(); };

    setupAnsiBtn (btnStop, juce::Colour (0xffff0055));
    btnStop.onClick = [this] { engine.stop(); };

    setupAnsiBtn (btnAudition, juce::Colour (0xffffb000));
    btnAudition.onClick = [this] {
        engine.play();
        engine.triggerAuditionNote();
    };

    startTimerHz (4); // 4 Hz ANSI text blinking clock
}

AnsiBoxStudioComponent::~AnsiBoxStudioComponent()
{
    stopTimer();
}

void AnsiBoxStudioComponent::timerCallback()
{
    blinkState = !blinkState;
    repaint();
}

void AnsiBoxStudioComponent::addObject (AnsiObjectType type)
{
    int count = static_cast<int>(objects.size()) + 1;
    float x = 40.0f + (count % 3) * 230.0f;
    float y = 80.0f + (count / 3) * 140.0f;

    switch (type)
    {
        case AnsiObjectType::MidiPlayer:
            objects.push_back ({ "midi_" + juce::String (count), "MIDI PLAYER " + juce::String (count), type, { x, y, 220.0f, 130.0f }, juce::Colour (0xffa855f7), 1.0f, 0.8f, false, {} });
            break;
        case AnsiObjectType::SynthSampler:
            objects.push_back ({ "synth_" + juce::String (count), "SYNTH/SAMPLER " + juce::String (count), type, { x, y, 240.0f, 150.0f }, juce::Colour (0xff00ff66), 1.0f, 0.85f, false, {} });
            break;
        case AnsiObjectType::AudioEvent:
            objects.push_back ({ "audio_" + juce::String (count), "AUDIO EVENT " + juce::String (count), type, { x, y, 240.0f, 130.0f }, juce::Colour (0xff00e5ff), 1.0f, 0.9f, false, {} });
            break;
        case AnsiObjectType::TimeGenerator:
            objects.push_back ({ "time_gen_" + juce::String (count), "TIME GENERATOR " + juce::String (count), type, { x, y, 260.0f, 120.0f }, juce::Colour (0xffffb000), 1.0f, 1.0f, false, {} });
            break;
    }
    repaint();
}

void AnsiBoxStudioComponent::mouseDown (const juce::MouseEvent& e)
{
    auto pt = e.position;

    // Check click on objects or tap output sockets
    for (size_t i = 0; i < objects.size(); ++i)
    {
        auto& obj = objects[i];
        if (obj.bounds.contains (pt))
        {
            // Tap out socket in bottom right corner [TAP OUT]
            juce::Rectangle<float> tapSocket (obj.bounds.getRight() - 65.0f, obj.bounds.getBottom() - 20.0f, 60.0f, 18.0f);
            if (tapSocket.contains (pt))
            {
                isTappingSignal = true;
                tapStartObjectIdx = static_cast<int>(i);
                tapCurrentPos = pt;
                return;
            }

            draggedObjectIdx = static_cast<int>(i);
            dragOffset = pt - obj.bounds.getPosition();
            return;
        }
    }
}

void AnsiBoxStudioComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (isTappingSignal)
    {
        tapCurrentPos = e.position;
        repaint();
    }
    else if (draggedObjectIdx >= 0 && draggedObjectIdx < static_cast<int>(objects.size()))
    {
        objects[draggedObjectIdx].bounds.setPosition (e.position - dragOffset);
        repaint();
    }
}

void AnsiBoxStudioComponent::mouseUp (const juce::MouseEvent& e)
{
    if (isTappingSignal && tapStartObjectIdx >= 0 && tapStartObjectIdx < static_cast<int>(objects.size()))
    {
        auto pt = e.position;
        for (size_t i = 0; i < objects.size(); ++i)
        {
            if (static_cast<int>(i) != tapStartObjectIdx && objects[i].bounds.contains (pt))
            {
                // Add fan-out output tap target!
                objects[tapStartObjectIdx].outputTapTargets.push_back (objects[i].id.toStdString());
                break;
            }
        }
    }

    draggedObjectIdx = -1;
    isTappingSignal = false;
    tapStartObjectIdx = -1;
    repaint();
}

void AnsiBoxStudioComponent::paint (juce::Graphics& g)
{
    // Deep Retro Terminal Background
    g.fillAll (juce::Colour (0xff0b0e14));

    // Top Terminal Header Status Banner
    g.setColour (juce::Colour (0xff121824));
    g.fillRect (0, 0, getWidth(), 35);
    g.setColour (juce::Colour (0xff1e293b));
    g.drawLine (0.0f, 35.0f, (float) getWidth(), 35.0f, 1.5f);

    drawAnsiText (g, "==========================================================================================", 10, 4, juce::Colour (0xff334155));
    drawAnsiText (g, "SYS://TIME_DILATION.DAW -- ANSI TEXT-MODE BOX OBJECT ENGINE [VER 4.0]", 15, 12, juce::Colour (0xff00ff66), true);

    juce::String statusStr = "STATUS: " + juce::String (engine.isPlaying() ? "PLAYING" : "STOPPED") + " | MASTER GAMMA: " + juce::String (engine.getMasterDilation(), 2) + "x";
    drawAnsiText (g, statusStr, 580, 12, juce::Colour (0xffffb000));

    drawTapCables (g);

    // Draw All ANSI Box Objects
    for (const auto& obj : objects)
    {
        drawAnsiBox (g, obj);
    }
}

void AnsiBoxStudioComponent::drawAnsiBox (juce::Graphics& g, const AnsiBoxObject& obj)
{
    float x = obj.bounds.getX();
    float y = obj.bounds.getY();
    float w = obj.bounds.getWidth();
    float h = obj.bounds.getHeight();

    // Dark Box Interior
    g.setColour (juce::Colour (0xff141a26));
    g.fillRect (x, y, w, h);

    // ANSI Border Box Drawing
    g.setColour (obj.color);
    g.drawRect (x, y, w, h, 1.5f);

    // ANSI Header Box
    g.setColour (obj.color.withAlpha (0.2f));
    g.fillRect (x, y, w, 22.0f);
    g.setColour (obj.color);
    g.drawLine (x, y + 22.0f, x + w, y + 22.0f, 1.5f);

    // Header Title
    g.setColour (juce::Colour (0xffffffff));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText ("+--[" + obj.title + "]--+", x + 6.0f, y + 3.0f, w - 12.0f, 16.0f, juce::Justification::left);

    // Object Body Details
    g.setFont (juce::FontOptions (10.0f, juce::Font::plain));

    if (obj.type == AnsiObjectType::TimeGenerator)
    {
        g.setColour (juce::Colour (0xffffb000));
        g.drawText ("+ TIME STREAM GENERATOR (GAMMA)", x + 10.0f, y + 30.0f, w - 20.0f, 14.0f, juce::Justification::left);
        g.drawText ("| RELATIVISTIC WARP: " + juce::String (engine.getMasterDilation(), 2) + "x", x + 10.0f, y + 46.0f, w - 20.0f, 14.0f, juce::Justification::left);
        g.drawText ("| FAN-OUT TAPS: " + juce::String (obj.outputTapTargets.size()) + " DESTINATIONS", x + 10.0f, y + 62.0f, w - 20.0f, 14.0f, juce::Justification::left);
    }
    else if (obj.type == AnsiObjectType::SynthSampler)
    {
        g.setColour (juce::Colour (0xff00ff66));
        g.drawText ("+ INPUT: MIDI & AUDIO ACCEPTED", x + 10.0f, y + 30.0f, w - 20.0f, 14.0f, juce::Justification::left);
        g.drawText ("| POLYPHONIC FM/SUBTRACTIVE SYNTH", x + 10.0f, y + 46.0f, w - 20.0f, 14.0f, juce::Justification::left);
        g.drawText ("| SAMPLE BUFFER: ACTIVE", x + 10.0f, y + 62.0f, w - 20.0f, 14.0f, juce::Justification::left);
        g.drawText ("| VARISPEED TAPE RESAMPLER: READY", x + 10.0f, y + 78.0f, w - 20.0f, 14.0f, juce::Justification::left);
    }
    else if (obj.type == AnsiObjectType::AudioEvent)
    {
        g.setColour (juce::Colour (0xff00e5ff));
        g.drawText ("+ AUDIO CLIP EVENT MATRIX", x + 10.0f, y + 30.0f, w - 20.0f, 14.0f, juce::Justification::left);
        g.drawText ("| RETROGRADE BUFFER: " + juce::String (engine.getMasterDilation() < 0 ? "REVERSE" : "FORWARD"), x + 10.0f, y + 46.0f, w - 20.0f, 14.0f, juce::Justification::left);
        g.drawText ("| SAMPLE RATE: 96000 Hz 24-BIT", x + 10.0f, y + 62.0f, w - 20.0f, 14.0f, juce::Justification::left);
    }
    else // MIDI Player
    {
        g.setColour (juce::Colour (0xffa855f7));
        g.drawText ("+ 128-BIT UMP PACKET SEQUENCER", x + 10.0f, y + 30.0f, w - 20.0f, 14.0f, juce::Justification::left);
        g.drawText ("| MIDI 2.0 HIGH-RES VELOCITY", x + 10.0f, y + 46.0f, w - 20.0f, 14.0f, juce::Justification::left);
        g.drawText ("| STEP PATTERN: 16-STEPS ACTIVE", x + 10.0f, y + 62.0f, w - 20.0f, 14.0f, juce::Justification::left);
    }

    // Bottom Fan-Out Tap Out Socket Button
    g.setColour (obj.color);
    g.fillRect (x + w - 75.0f, y + h - 22.0f, 70.0f, 18.0f);
    g.setColour (juce::Colour (0xff0f0f0f));
    g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
    g.drawText ("[TAP OUT]", x + w - 75.0f, y + h - 22.0f, 70.0f, 18.0f, juce::Justification::centred);
}

void AnsiBoxStudioComponent::drawTapCables (juce::Graphics& g)
{
    // Draw Fan-Out Output Tap Cables
    for (const auto& obj : objects)
    {
        juce::Point<float> startPos (obj.bounds.getRight() - 40.0f, obj.bounds.getBottom() - 11.0f);

        for (const auto& targetId : obj.outputTapTargets)
        {
            for (const auto& targetObj : objects)
            {
                if (targetObj.id == targetId.c_str())
                {
                    juce::Point<float> endPos = targetObj.bounds.getTopLeft() + juce::Point<float> (30.0f, 11.0f);

                    juce::Path p;
                    p.startNewSubPath (startPos);
                    float ctrlY1 = startPos.y + 40.0f;
                    float ctrlY2 = endPos.y - 40.0f;
                    p.cubicTo (startPos.x, ctrlY1, endPos.x, ctrlY2, endPos.x, endPos.y);

                    g.setColour (obj.color);
                    g.strokePath (p, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                    break;
                }
            }
        }
    }

    // Draw Live Dragging Tap Cable
    if (isTappingSignal && tapStartObjectIdx >= 0 && tapStartObjectIdx < static_cast<int>(objects.size()))
    {
        const auto& startObj = objects[tapStartObjectIdx];
        juce::Point<float> startPos (startObj.bounds.getRight() - 40.0f, startObj.bounds.getBottom() - 11.0f);

        juce::Path p;
        p.startNewSubPath (startPos);
        p.cubicTo (startPos.x, startPos.y + 40.0f, tapCurrentPos.x, tapCurrentPos.y - 40.0f, tapCurrentPos.x, tapCurrentPos.y);

        g.setColour (startObj.color);
        g.strokePath (p, juce::PathStrokeType (2.5f));
    }
}

void AnsiBoxStudioComponent::drawAnsiText (juce::Graphics& g, const juce::String& text, float x, float y, juce::Colour color, bool blink)
{
    if (blink && !blinkState)
    {
        color = color.withAlpha (0.4f);
    }
    g.setColour (color);
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText (text, x, y, 600.0f, 16.0f, juce::Justification::left);
}

void AnsiBoxStudioComponent::resized()
{
    btnAddMidi.setBounds (10, getHeight() - 35, 120, 26);
    btnAddSynth.setBounds (135, getHeight() - 35, 140, 26);
    btnAddAudio.setBounds (280, getHeight() - 35, 130, 26);
    btnAddTimeGen.setBounds (415, getHeight() - 35, 110, 26);

    btnPlay.setBounds (getWidth() - 250, getHeight() - 35, 70, 26);
    btnStop.setBounds (getWidth() - 175, getHeight() - 35, 70, 26);
    btnAudition.setBounds (getWidth() - 100, getHeight() - 35, 90, 26);
}

} // namespace time_dilation
