#include "ModularDataflowCanvas.h"

namespace time_dilation
{

ModularDataflowCanvas::ModularDataflowCanvas (TimeDilationEngine& e)
    : engine (e)
{
    // Default pre-connected patch cables: Track 1 -> Mixer Bus 1 -> Master Out
    persistentCables.push_back ({ "track_0", "AUDIO_OUT", "mixer_1", "AUDIO_IN_1", juce::Colour (0xff06b6d4) });
    persistentCables.push_back ({ "track_1", "AUDIO_OUT", "mixer_1", "AUDIO_IN_2", juce::Colour (0xff8b5cf6) });
    persistentCables.push_back ({ "mixer_1", "AUDIO_OUT", "master_out", "AUDIO_IN", juce::Colour (0xfff59e0b) });
}

void ModularDataflowCanvas::addSynthNode()
{
    synthNodeCount++;
    repaint();
}

void ModularDataflowCanvas::addMixerBusNode()
{
    mixerBusCount++;
    repaint();
}

void ModularDataflowCanvas::mouseDown (const juce::MouseEvent& e)
{
    auto pt = e.position;
    for (const auto& sock : activeSockets)
    {
        if (sock.pos.getDistanceFrom (pt) < 14.0f)
        {
            isDraggingCable = true;
            dragStartSocket = sock;
            cableCurrentPos = pt;
            repaint();
            return;
        }
    }
}

void ModularDataflowCanvas::mouseDrag (const juce::MouseEvent& e)
{
    if (isDraggingCable)
    {
        cableCurrentPos = e.position;
        repaint();
    }
}

void ModularDataflowCanvas::mouseUp (const juce::MouseEvent& e)
{
    if (isDraggingCable)
    {
        auto pt = e.position;
        for (const auto& sock : activeSockets)
        {
            if (sock.pos.getDistanceFrom (pt) < 16.0f && sock.isOutput != dragStartSocket.isOutput)
            {
                if (dragStartSocket.isOutput)
                {
                    persistentCables.push_back ({ dragStartSocket.nodeId, dragStartSocket.portId, sock.nodeId, sock.portId, dragStartSocket.color });
                }
                else
                {
                    persistentCables.push_back ({ sock.nodeId, sock.portId, dragStartSocket.nodeId, dragStartSocket.portId, sock.color });
                }
                break;
            }
        }
        isDraggingCable = false;
        repaint();
    }
}

void ModularDataflowCanvas::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0e17));

    // Draw Column Dividers and Column Headers
    const float colW = 230.0f;

    // Column 1 Header: TRACKS & GENERATORS
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRect (10.0f, 10.0f, colW, 25.0f);
    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText ("1. TRACKS & GENERATORS", 15.0f, 12.0f, colW, 20.0f, juce::Justification::left);

    // Column 2 Header: MIXER BUSES
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRect (260.0f, 10.0f, colW, 25.0f);
    g.setColour (juce::Colour (0xff06b6d4));
    g.drawText ("2. MIXER BUSES", 265.0f, 12.0f, colW, 20.0f, juce::Justification::left);

    // Column 3 Header: MASTER OUTPUT BUS
    g.setColour (juce::Colour (0xff1e293b));
    g.fillRect (510.0f, 10.0f, colW, 25.0f);
    g.setColour (juce::Colour (0xffa855f7));
    g.drawText ("3. MASTER OUTPUT BUS", 515.0f, 12.0f, colW, 20.0f, juce::Justification::left);

    activeSockets.clear();

    // Render Column 1: Tracks
    const auto& tracks = engine.getTracks();
    float trackY = 45.0f;
    const float cardH = 140.0f;

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        drawNodeCard (g, "track_" + juce::String (i).toStdString(), tracks[i].name.toStdString(), tracks[i].color, 10.0f, trackY, colW, cardH);
        trackY += cardH + 15.0f;
    }

    // Render Column 2: Mixer Buses
    drawNodeCard (g, "mixer_1", "MIXER BUS 1", juce::Colour (0xff06b6d4), 260.0f, 45.0f, colW, 180.0f);

    // Render Column 3: Master Output Bus
    drawNodeCard (g, "master_out", "MASTER OUTPUT BUS", juce::Colour (0xfff59e0b), 510.0f, 45.0f, colW, 200.0f);

    drawPatchCables (g);
}

void ModularDataflowCanvas::drawNodeCard (juce::Graphics& g, const std::string& nodeId, const std::string& name, juce::Colour color, float x, float y, float width, float height)
{
    // Card Background
    g.setColour (juce::Colour (0xff141a26));
    g.fillRoundedRectangle (x, y, width, height, 6.0f);

    g.setColour (color);
    g.drawRoundedRectangle (x, y, width, height, 6.0f, 1.5f);

    // Header Tag
    g.setColour (color.withAlpha (0.2f));
    g.fillRect (x, y, width, 24.0f);

    g.setColour (juce::Colour (0xffffffff));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText (name, x + 8.0f, y + 4.0f, width - 16.0f, 16.0f, juce::Justification::left);

    float sockY = y + 36.0f;

    if (nodeId == "master_out") // Master Output Bus
    {
        // Master Volume Fader Representation
        g.setColour (juce::Colour (0xff1e293b));
        g.fillRect (x + 20.0f, y + 40.0f, 12.0f, 110.0f);
        g.setColour (juce::Colour (0xfff59e0b));
        g.fillRect (x + 20.0f, y + 70.0f, 12.0f, 80.0f);

        // Peak Meter Bar
        g.setColour (juce::Colour (0xff10b981)); // Green VU
        g.fillRect (x + 40.0f, y + 50.0f, 8.0f, 100.0f);

        g.setColour (juce::Colour (0xff94a3b8));
        g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
        g.drawText ("MASTER VOL & VU", x + 55.0f, y + 50.0f, 120.0f, 16.0f, juce::Justification::left);
        g.drawText ("GAMMA: " + juce::String (engine.getMasterDilation(), 2) + "x", x + 55.0f, y + 70.0f, 120.0f, 16.0f, juce::Justification::left);

        // Audio Input Socket
        g.setColour (juce::Colour (0xfff59e0b));
        g.fillEllipse (x - 6.0f, y + 40.0f, 12.0f, 12.0f);
        activeSockets.push_back ({ nodeId, "AUDIO_IN", { x, y + 46.0f }, juce::Colour (0xfff59e0b), false, PortType::Audio });
    }
    else if (nodeId.rfind ("mixer", 0) == 0) // Mixer Bus Strip
    {
        // Mixer Sub-Faders
        g.setColour (juce::Colour (0xff1e293b));
        g.fillRect (x + 15.0f, y + 35.0f, 8.0f, 100.0f);
        g.fillRect (x + 35.0f, y + 35.0f, 8.0f, 100.0f);

        g.setColour (juce::Colour (0xff06b6d4));
        g.fillRect (x + 15.0f, y + 55.0f, 8.0f, 80.0f);
        g.fillRect (x + 35.0f, y + 65.0f, 8.0f, 70.0f);

        g.setColour (juce::Colour (0xff94a3b8));
        g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
        g.drawText ("CH 1 & CH 2 SUB-FADERS", x + 50.0f, y + 40.0f, 130.0f, 16.0f, juce::Justification::left);

        // Input & Output Sockets
        g.setColour (juce::Colour (0xff06b6d4)); // Audio In 1
        g.fillEllipse (x - 5.0f, sockY, 10.0f, 10.0f);
        activeSockets.push_back ({ nodeId, "AUDIO_IN_1", { x, sockY + 5.0f }, juce::Colour (0xff06b6d4), false, PortType::Audio });

        sockY += 20.0f;
        g.setColour (juce::Colour (0xff8b5cf6)); // Audio In 2
        g.fillEllipse (x - 5.0f, sockY, 10.0f, 10.0f);
        activeSockets.push_back ({ nodeId, "AUDIO_IN_2", { x, sockY + 5.0f }, juce::Colour (0xff8b5cf6), false, PortType::Audio });

        sockY += 30.0f;
        g.setColour (juce::Colour (0xff06b6d4)); // Audio Out to Master
        g.fillEllipse (x + width - 5.0f, sockY, 10.0f, 10.0f);
        activeSockets.push_back ({ nodeId, "AUDIO_OUT", { x + width, sockY + 5.0f }, juce::Colour (0xff06b6d4), true, PortType::Audio });
    }
    else // Track Node Strip
    {
        // Track Controls Representation
        g.setColour (juce::Colour (0xff1e293b));
        g.fillRect (x + 10.0f, y + 35.0f, 6.0f, 85.0f);
        g.setColour (color);
        g.fillRect (x + 10.0f, y + 55.0f, 6.0f, 65.0f);

        g.setColour (juce::Colour (0xff94a3b8));
        g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
        g.drawText ("TRACK FADER & PAN", x + 25.0f, y + 35.0f, 130.0f, 16.0f, juce::Justification::left);
        g.drawText ("GAMMA: 1.0x", x + 25.0f, y + 55.0f, 130.0f, 16.0f, juce::Justification::left);

        // Input & Output Sockets
        g.setColour (color); // Audio Out
        g.fillEllipse (x + width - 5.0f, sockY + 10.0f, 10.0f, 10.0f);
        activeSockets.push_back ({ nodeId, "AUDIO_OUT", { x + width, sockY + 15.0f }, color, true, PortType::Audio });

        g.setColour (juce::Colour (0xffa855f7)); // MIDI In
        g.fillEllipse (x - 5.0f, sockY + 10.0f, 10.0f, 10.0f);
        activeSockets.push_back ({ nodeId, "MIDI2_IN", { x, sockY + 15.0f }, juce::Colour (0xffa855f7), false, PortType::Midi2 });
    }
}

void ModularDataflowCanvas::drawPatchCables (juce::Graphics& g)
{
    // Render Persistent Cable Links
    for (const auto& cable : persistentCables)
    {
        juce::Point<float> p1, p2;
        bool foundStart = false, foundEnd = false;

        for (const auto& sock : activeSockets)
        {
            if (sock.nodeId == cable.fromNodeId && sock.portId == cable.fromPortId) { p1 = sock.pos; foundStart = true; }
            if (sock.nodeId == cable.toNodeId && sock.portId == cable.toPortId) { p2 = sock.pos; foundEnd = true; }
        }

        if (foundStart && foundEnd)
        {
            juce::Path path;
            path.startNewSubPath (p1);
            float ctrlX1 = p1.x + 60.0f;
            float ctrlX2 = p2.x - 60.0f;
            path.cubicTo (ctrlX1, p1.y, ctrlX2, p2.y, p2.x, p2.y);

            g.setColour (cable.color);
            g.strokePath (path, juce::PathStrokeType (3.0f));
        }
    }

    // Render Live Dragged Cable
    if (isDraggingCable)
    {
        juce::Path p;
        p.startNewSubPath (dragStartSocket.pos);
        float ctrlX1 = dragStartSocket.pos.x + (dragStartSocket.isOutput ? 60.0f : -60.0f);
        float ctrlX2 = cableCurrentPos.x - 60.0f;
        p.cubicTo (ctrlX1, dragStartSocket.pos.y, ctrlX2, cableCurrentPos.y, cableCurrentPos.x, cableCurrentPos.y);

        g.setColour (dragStartSocket.color);
        g.strokePath (p, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

void ModularDataflowCanvas::resized()
{
}

} // namespace time_dilation
