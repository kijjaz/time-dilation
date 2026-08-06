#include "../RelativisticCanvasComponent.h"
#include "../../dsp/RelativisticNodeObjects.h"
#include <cmath>

namespace time_dilation
{

void RelativisticCanvasComponent::drawNode (juce::Graphics& g, const std::shared_ptr<RelativisticNode>& node)
{
    const float x = node->getX() + panX;
    const float y = node->getY() + panY;
    const float w = getNodeWidth (*node);
    const float h = getNodeHeight (*node);

    bool isTimeObj = node->getTypeName().rfind ("time.", 0) == 0;
    bool isAudioObj = node->getTypeName().find ("~") != std::string::npos || node->getTypeName() == "dac~" || node->getTypeName() == "gain~" || node->getTypeName() == "out~";

    juce::Colour typeBadgeCol = isTimeObj ? juce::Colour (0xff8b5cf6) : (isAudioObj ? juce::Colour (0xff06b6d4) : juce::Colour (0xfff59e0b));

    // PlugData Slim Rounded Card Body & Clip Path
    juce::Path cardPath;
    cardPath.addRoundedRectangle (x, y, w, h, 6.0f);

    g.setColour (juce::Colour (0xff181825));
    g.fillPath (cardPath);

    // Left Type Indicator Strip (Clipped inside rounded corners)
    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (cardPath);
        g.setColour (typeBadgeCol);
        g.fillRect (x, y, 4.0f, h);
    }

    // Selection Halo & Border
    if (selectedNodeIds.count (node->getId()) > 0)
    {
        g.setColour (juce::Colour (0xff38bdf8)); // Glowing Cyan Outline
        g.drawRoundedRectangle (x - 1.5f, y - 1.5f, w + 3.0f, h + 3.0f, 7.0f, 2.0f);
    }
    else
    {
        g.setColour (juce::Colour (0xff2e2e42)); // Hairline Dark Slate Border
        g.drawRoundedRectangle (x, y, w, h, 6.0f, 1.0f);
    }

    // Render [bang] Control Trigger LED Button
    if (node->getTypeName() == "bang" || node->getTypeName() == "b")
    {
        float cx = x + w - 24.0f;
        float cy = y + h * 0.5f;
        g.setColour (juce::Colour (0xff0f172a));
        g.fillEllipse (cx - 10.0f, cy - 10.0f, 20.0f, 20.0f);
        g.setColour (juce::Colour (0xfff59e0b));
        g.drawEllipse (cx - 10.0f, cy - 10.0f, 20.0f, 20.0f, 1.5f);
        g.setColour (juce::Colour (0xfff59e0b).withAlpha (0.9f));
        g.fillEllipse (cx - 6.0f, cy - 6.0f, 12.0f, 12.0f);
    }

    // Render [bang~] Audio Impulse Spike LED Ring
    if (node->getTypeName() == "bang~" || node->getTypeName() == "b~")
    {
        float cx = x + w - 24.0f;
        float cy = y + h * 0.5f;
        g.setColour (juce::Colour (0xff0f172a));
        g.fillEllipse (cx - 10.0f, cy - 10.0f, 20.0f, 20.0f);
        g.setColour (juce::Colour (0xff06b6d4));
        g.drawEllipse (cx - 10.0f, cy - 10.0f, 20.0f, 20.0f, 1.5f);
        g.setColour (juce::Colour (0xff06b6d4).withAlpha (0.9f));
        g.fillEllipse (cx - 6.0f, cy - 6.0f, 12.0f, 12.0f);
    }

    // Special Canvas Visualization for [toggle] Control Switch Box Object
    if (node->getTypeName() == "toggle" || node->getTypeName() == "tgl")
    {
        bool isOn = (node->getParameter ("state", 0.0f) > 0.5f);
        float boxW = 22.0f;
        float boxX = x + w - boxW - 8.0f;
        float boxY = y + (h * 0.5f - 11.0f);
        float boxH = 22.0f;

        g.setColour (juce::Colour (0xff070a12));
        g.fillRoundedRectangle (boxX, boxY, boxW, boxH, 4.0f);
        g.setColour (isOn ? juce::Colour (0xff22c55e) : juce::Colour (0xff64748b));
        g.drawRoundedRectangle (boxX, boxY, boxW, boxH, 4.0f, 1.5f);

        if (isOn)
        {
            g.setColour (juce::Colour (0xff22c55e));
            g.drawLine (boxX + 5.0f, boxY + 5.0f, boxX + boxW - 5.0f, boxY + boxH - 5.0f, 2.5f);
            g.drawLine (boxX + boxW - 5.0f, boxY + 5.0f, boxX + 5.0f, boxY + boxH - 5.0f, 2.5f);
        }
    }

    // Special Canvas Visualization for [slider] Control Slider UI Object
    if (node->getTypeName() == "slider" || node->getTypeName() == "hslider" || node->getTypeName() == "vslider")
    {
        float val = node->getParameter ("value", 0.0f);
        float minV = node->getParameter ("min", 0.0f);
        float maxV = node->getParameter ("max", 1.0f);
        float normVal = std::clamp ((val - minV) / (maxV - minV > 1e-6f ? maxV - minV : 1.0f), 0.0f, 1.0f);

        float barW = 85.0f;
        float barX = x + w - barW - 8.0f;
        float barY = y + (h * 0.5f - 9.0f);
        float barH = 18.0f;

        g.setColour (juce::Colour (0xff070a12));
        g.fillRoundedRectangle (barX, barY, barW, barH, 3.0f);
        g.setColour (juce::Colour (0xfff59e0b).withAlpha (0.4f));
        g.drawRoundedRectangle (barX, barY, barW, barH, 3.0f, 1.0f);

        g.setColour (juce::Colour (0xfff59e0b).withAlpha (0.85f));
        g.fillRoundedRectangle (barX + 2.0f, barY + 2.0f, (barW - 4.0f) * normVal, barH - 4.0f, 2.0f);

        g.setFont (FontManager::getInstance().getOxaniumFont (10.0f, true));
        g.setColour (juce::Colours::white);
        juce::String valStr = (std::abs (val - std::round (val)) < 0.001f) ? juce::String (static_cast<int>(std::round (val))) : juce::String (val, 2);
        g.drawText (valStr, barX, barY, barW, barH, juce::Justification::centred);
    }

    // Special Canvas Visualization for [radio] Multi-Option Selector Strip Object
    if (node->getTypeName() == "radio" || node->getTypeName() == "hradio" || node->getTypeName() == "vradio" || node->getTypeName().rfind ("radio ", 0) == 0)
    {
        int opts = std::max (2, static_cast<int>(node->getParameter ("options", 4.0f)));
        int activeIdx = std::clamp (static_cast<int>(std::round (node->getParameter ("index", 0.0f))), 0, opts - 1);

        float stripW = std::min (w - 30.0f, opts * 18.0f);
        float stripX = x + w - stripW - 8.0f;
        float stripY = y + (h * 0.5f - 8.0f);
        float stripH = 16.0f;

        float cellW = stripW / static_cast<float>(opts);
        for (int i = 0; i < opts; ++i)
        {
            float cx = stripX + i * cellW;
            bool isActive = (i == activeIdx);
            g.setColour (isActive ? juce::Colour (0xfff59e0b) : juce::Colour (0xff1e293b));
            g.fillRoundedRectangle (cx + 1.0f, stripY, cellW - 2.0f, stripH, 2.0f);
            g.setColour (isActive ? juce::Colour (0xff070a12) : juce::Colour (0xff94a3b8));
            g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, true));
            g.drawText (juce::String (i), cx, stripY, cellW, stripH, juce::Justification::centred);
        }
    }

    // Special Canvas Visualization for [time.transport] Realtime Beat & Status Display + Beat Flash LED
    auto transportNode = std::dynamic_pointer_cast<TimeTransportNode> (node);
    if (transportNode)
    {
        double beats = transportNode->getCurrentBeatPosition();
        int bar = static_cast<int>(std::floor (beats / 4.0)) + 1;
        double beatInBar = std::fmod (beats, 4.0) + 1.0;
        bool playing = transportNode->getIsPlaying();
        bool flashing = transportNode->getIsBeatFlashing();

        float ledX = x + w - 20.0f;
        float ledY = y + h * 0.5f;
        g.setColour (juce::Colour (0xff070a12));
        g.fillEllipse (ledX - 8.0f, ledY - 8.0f, 16.0f, 16.0f);
        
        juce::Colour ledCol = flashing ? juce::Colour (0xfff59e0b) : (playing ? juce::Colour (0xff059669) : juce::Colour (0xff334155));
        g.setColour (ledCol);
        g.fillEllipse (ledX - 5.0f, ledY - 5.0f, 10.0f, 10.0f);
        if (flashing)
        {
            g.setColour (juce::Colours::white);
            g.drawEllipse (ledX - 8.0f, ledY - 8.0f, 16.0f, 16.0f, 2.0f);
        }

        g.setFont (FontManager::getInstance().getOxaniumFont (10.5f, true));
        g.setColour (playing ? juce::Colour (0xff38bdf8) : juce::Colour (0xff94a3b8));
        juce::String posStr = juce::String::formatted ("Bar %d : Beat %.1f", bar, beatInBar);
        g.drawText (posStr, x + 10.0f, y + 37.0f, w - 36.0f, 16.0f, juce::Justification::centredLeft);
    }

    // Special Canvas Visualization for [time.scope] CRT Oscilloscope Display
    auto timeScope = std::dynamic_pointer_cast<TimeScopeNode> (node);
    if (timeScope)
    {
        float gamma = timeScope->getMonitoredGamma();
        double tSec = timeScope->getMonitoredTimeSec();
        float timeWin = timeScope->getParameter ("timeWindow", 1.0f);

        float screenX = x + 8.0f;
        float screenY = y + 20.0f;
        float screenW = w - 16.0f;
        float screenH = h - 26.0f;

        // Dark CRT Scope Background
        g.setColour (juce::Colour (0xff050811));
        g.fillRoundedRectangle (screenX, screenY, screenW, screenH, 4.0f);
        g.setColour (juce::Colour (0xff06b6d4).withAlpha (0.4f));
        g.drawRoundedRectangle (screenX, screenY, screenW, screenH, 4.0f, 1.0f);

        // 1. Time Axis & Division Grid Reticle
        g.setColour (juce::Colour (0x1f06b6d4));
        float midY = screenY + screenH * 0.5f;
        g.drawHorizontalLine (static_cast<int>(midY), screenX + 2.0f, screenX + screenW - 2.0f);
        g.drawHorizontalLine (static_cast<int>(screenY + screenH * 0.25f), screenX + 2.0f, screenX + screenW - 2.0f);
        g.drawHorizontalLine (static_cast<int>(screenY + screenH * 0.75f), screenX + 2.0f, screenX + screenW - 2.0f);

        // Vertical Time Grid Division Lines & Labels
        int numTimeDivs = 4;
        g.setFont (FontManager::getInstance().getOxaniumFont (8.5f, false));
        for (int div = 0; div <= numTimeDivs; ++div)
        {
            float divX = screenX + (static_cast<float>(div) / static_cast<float>(numTimeDivs)) * screenW;
            g.setColour (juce::Colour (0x1a94a3b8));
            g.drawVerticalLine (static_cast<int>(divX), screenY + 12.0f, screenY + screenH - 12.0f);

            double divTime = (div / static_cast<double>(numTimeDivs)) * timeWin;
            g.setColour (juce::Colour (0xff64748b));
            g.drawText (juce::String (divTime, 2) + "s", divX - 15.0f, screenY + screenH - 11.0f, 30.0f, 10.0f, juce::Justification::centred);
        }

        // 2. Waveform Trace
        const auto& hist = timeScope->getSignalHistory();
        size_t writePos = timeScope->getHistoryWritePos();
        float scaleMax = std::max (0.01f, timeScope->getAutoScaleMax());

        if (!hist.empty())
        {
            juce::Path wavePath;
            size_t total = hist.size();

            for (size_t i = 0; i < total; ++i)
            {
                size_t readIdx = (writePos + i) % total;
                float val = std::clamp (hist[readIdx] / scaleMax, -1.0f, 1.0f);
                float px = screenX + (static_cast<float>(i) / static_cast<float>(total - 1)) * screenW;
                float py = midY - val * (screenH * 0.38f);

                if (i == 0) wavePath.startNewSubPath (px, py);
                else        wavePath.lineTo (px, py);
            }

            // Glowing Cyan Scope Line
            g.setColour (juce::Colour (0xff06b6d4));
            g.strokePath (wavePath, juce::PathStrokeType (1.5f));

            // Telemetry Overlay HUD (Gamma factor & Local Coordinate Time)
            g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, true));
            g.setColour (juce::Colour (0xff8b5cf6)); // Royal Violet Accent
            g.drawText (juce::String::formatted ("\xCE\xB3=%.2fx", gamma), screenX + 4.0f, screenY + 2.0f, 60.0f, 12.0f, juce::Justification::left);
            g.setColour (juce::Colour (0xfff59e0b)); // Relativistic Gold
            g.drawText (juce::String::formatted ("t=%.2fs", tSec), screenX + screenW - 65.0f, screenY + 2.0f, 60.0f, 12.0f, juce::Justification::right);
        }
    }

    // Special Canvas Visualization for [table] Interactive Waveform / Step Sequencer Canvas
    auto tableNode = std::dynamic_pointer_cast<TableNode> (node);
    if (tableNode)
    {
        float graphX = x + 8.0f;
        float graphY = y + 22.0f;
        float graphW = w - 16.0f;
        float graphH = h - 26.0f;

        g.setColour (juce::Colour (0xff070a12));
        g.fillRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f);
        g.setColour (juce::Colour (0xff1e293b));
        g.drawRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f, 1.0f);

        const auto& data = tableNode->getTableData();
        int dataSize = static_cast<int>(data.size());
        if (dataSize > 0)
        {
            juce::Path wavePath;
            float midY = graphY + graphH * 0.5f;

            if (dataSize <= 16) // Step Bar Mode
            {
                float stepW = graphW / static_cast<float>(dataSize);
                for (int i = 0; i < dataSize; ++i)
                {
                    float val = data[i];
                    float normVal = (val > 1.5f) ? (val / 127.0f) : val;
                    float barH = (normVal * 0.5f) * graphH;
                    float bx = graphX + i * stepW;

                    g.setColour (juce::Colour (0xff06b6d4).withAlpha (0.85f));
                    if (normVal < 0.0f)
                        g.fillRect (bx + 1.0f, midY, stepW - 2.0f, -barH);
                    else
                        g.fillRect (bx + 1.0f, midY - barH, stepW - 2.0f, barH);
                }
            }
            else // Continuous Waveform Mode
            {
                int sampleStep = std::max (1, dataSize / 64);
                for (int i = 0; i < 64; ++i)
                {
                    int sampleIdx = std::min (dataSize - 1, i * sampleStep);
                    float val = std::clamp (data[sampleIdx], -1.0f, 1.0f);
                    float px = graphX + (static_cast<float>(i) / 63.0f) * graphW;
                    float py = midY - val * (graphH * 0.45f);

                    if (i == 0) wavePath.startNewSubPath (px, py);
                    else        wavePath.lineTo (px, py);
                }

                g.setColour (juce::Colour (0xff06b6d4)); // Cyber Cyan
                g.strokePath (wavePath, juce::PathStrokeType (1.5f));
            }
        }
    }

    // Special Canvas Visualization for [time.xy] Dual-Time X-Y Lissajous Scope
    auto xyNode = std::dynamic_pointer_cast<TimeXYNode> (node);
    if (xyNode)
    {
        float graphX = x + 8.0f;
        float graphY = y + 22.0f;
        float graphW = w - 16.0f;
        float graphH = h - 26.0f;

        g.setColour (juce::Colour (0xff050811));
        g.fillRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f);
        g.setColour (juce::Colour (0xff38bdf8).withAlpha (0.4f));
        g.drawRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f, 1.0f);

        const auto& pts = xyNode->getPointHistory();
        float autoR = std::max (0.01f, xyNode->getAutoScaleRadius());

        if (!pts.empty())
        {
            juce::Path xyPath;
            float centerX = graphX + graphW * 0.5f;
            float centerY = graphY + graphH * 0.5f;

            for (size_t i = 0; i < pts.size(); ++i)
            {
                float normX = pts[i].x / autoR;
                float normY = pts[i].y / autoR;
                float px = centerX + normX * (graphW * 0.42f);
                float py = centerY - normY * (graphH * 0.42f);

                if (i == 0) xyPath.startNewSubPath (px, py);
                else        xyPath.lineTo (px, py);
            }

            g.setColour (juce::Colour (0xff38bdf8)); // Glowing Cyan Phase Orbit
            g.strokePath (xyPath, juce::PathStrokeType (1.5f));

            g.setColour (juce::Colour (0xff7dd3fc));
            g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, true));
            g.drawText ("RADIUS: ±" + juce::String (autoR, 2), graphX + 4, graphY + 2, graphW - 8, 12, juce::Justification::topRight);
        }
    }

    // Special Canvas Visualization for [out~] Live VU RMS Meters & Oscilloscope Screen
    auto outNode = std::dynamic_pointer_cast<OutNode> (node);
    if (outNode)
    {
        float graphX = x + 8.0f;
        float graphY = y + 22.0f;
        float graphW = w - 46.0f;
        float graphH = h - 26.0f;

        // Dark Scope Screen
        g.setColour (juce::Colour (0xff050811));
        g.fillRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f);
        g.setColour (juce::Colour (0xff1e293b));
        g.drawRoundedRectangle (graphX, graphY, graphW, graphH, 3.0f, 1.0f);

        // Center reticle crosshair
        g.setColour (juce::Colour (0xff1e293b));
        g.drawHorizontalLine (static_cast<int>(graphY + graphH * 0.5f), graphX, graphX + graphW);
        g.drawVerticalLine (static_cast<int>(graphX + graphW * 0.5f), graphY, graphY + graphH);

        const auto& scopeL = outNode->getScopeL();
        const auto& scopeR = outNode->getScopeR();
        int writeIdx = outNode->getScopeWriteIndex();

        float displayMode = outNode->getParameter ("displayMode", 0.0f);

        if (!scopeL.empty() && !scopeR.empty())
        {
            if (displayMode < 0.5f)
            {
                // Mode 0: Dual Trace Time Domain Scope (Left Cyan, Right Gold)
                juce::Path pathL, pathR;
                int total = static_cast<int>(scopeL.size());
                float midY = graphY + graphH * 0.5f;

                for (int i = 0; i < 64; ++i)
                {
                    int idx = (writeIdx + i * (total / 64)) % total;
                    float sampleL = std::clamp (scopeL[idx], -1.2f, 1.2f);
                    float sampleR = std::clamp (scopeR[idx], -1.2f, 1.2f);

                    float px = graphX + (static_cast<float>(i) / 63.0f) * graphW;
                    float pyL = midY - sampleL * (graphH * 0.42f);
                    float pyR = midY - sampleR * (graphH * 0.42f);

                    if (i == 0) { pathL.startNewSubPath (px, pyL); pathR.startNewSubPath (px, pyR); }
                    else        { pathL.lineTo (px, pyL);           pathR.lineTo (px, pyR); }
                }

                g.setColour (juce::Colour (0xff06b6d4)); // Cyber Cyan Left Channel
                g.strokePath (pathL, juce::PathStrokeType (1.5f));

                g.setColour (juce::Colour (0xfff59e0b)); // Relativistic Gold Right Channel
                g.strokePath (pathR, juce::PathStrokeType (1.2f));
            }
            else
            {
                // Mode 1: X-Y Lissajous Phase Plot Scope (Royal Violet)
                juce::Path pathXY;
                int total = static_cast<int>(scopeL.size());
                float centerX = graphX + graphW * 0.5f;
                float centerY = graphY + graphH * 0.5f;
                float scaleX = graphW * 0.42f;
                float scaleY = graphH * 0.42f;

                for (int i = 0; i < 64; ++i)
                {
                    int idx = (writeIdx + i * (total / 64)) % total;
                    float sampleL = std::clamp (scopeL[idx], -1.2f, 1.2f);
                    float sampleR = std::clamp (scopeR[idx], -1.2f, 1.2f);

                    float px = centerX + sampleL * scaleX;
                    float py = centerY - sampleR * scaleY;

                    if (i == 0) pathXY.startNewSubPath (px, py);
                    else        pathXY.lineTo (px, py);
                }

                g.setColour (juce::Colour (0xffa855f7)); // Royal Violet Lissajous Scope
                g.strokePath (pathXY, juce::PathStrokeType (1.5f));
            }
        }

        // Live VU RMS Meters on the right edge
        float meterX = x + w - 32.0f;
        float meterY = y + 22.0f;
        float meterW = 10.0f;
        float meterH = graphH;

        g.setColour (juce::Colour (0xff0b0f19));
        g.fillRoundedRectangle (meterX, meterY, meterW * 2.2f, meterH, 2.0f);

        float rmsL = std::clamp (outNode->getRmsL(), 0.0f, 1.2f);
        float rmsR = std::clamp (outNode->getRmsR(), 0.0f, 1.2f);

        auto drawBar = [&] (float bx, float level) {
            float barH = level * meterH;
            juce::Colour barColor = juce::Colour (0xff22c55e);
            if (level > 0.85f) barColor = juce::Colour (0xffeab308);
            if (level >= 1.0f) barColor = juce::Colour (0xffef4444);

            g.setColour (barColor);
            g.fillRect (bx, meterY + (meterH - barH), meterW - 1.0f, barH);
        };

        drawBar (meterX, rmsL);
        drawBar (meterX + meterW, rmsR);

        if (outNode->isRecordingActive())
        {
            g.setColour (juce::Colours::red.withAlpha (0.9f));
            g.fillEllipse (graphX + 6.0f, graphY + 6.0f, 8.0f, 8.0f);
            g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, true));
            g.drawText ("REC (/tmp)", graphX + 16.0f, graphY + 4.0f, 60.0f, 12.0f, juce::Justification::centredLeft);
        }
    }

    // Title Text in Sci-Fi Oxanium Font (No Truncation)
    g.setColour (juce::Colour (0xfff8fafc));
    g.setFont (FontManager::getInstance().getOxaniumFont (14.0f, true));
    bool isNumObj = (node->getTypeName() == "number" || node->getTypeName() == "num" || node->getTypeName() == "nb");
    bool isSliderObj = (node->getTypeName() == "slider" || node->getTypeName() == "hslider" || node->getTypeName() == "vslider");
    bool isToggleObj = (node->getTypeName() == "toggle" || node->getTypeName() == "tgl");
    bool isRadioObj = (node->getTypeName() == "radio" || node->getTypeName() == "hradio" || node->getTypeName() == "vradio" || node->getTypeName().rfind ("radio ", 0) == 0);

    float labelTextW = outNode ? w - 40.0f : ((isNumObj || isSliderObj || isToggleObj || isRadioObj) ? w - 95.0f : w - 12.0f);
    float labelY = transportNode ? (y + 17.0f) : (y + (h * 0.5f - 10.0f));
    float labelH = transportNode ? 18.0f : 20.0f;
    g.drawText (node->getLabel(), x + 10, labelY, labelTextW, labelH, juce::Justification::centredLeft);

    // Inlets (Top Edge Dots with Smart Spaced Labels)
    juce::Font portFont = FontManager::getInstance().getOxaniumFont (9.5f, false);
    g.setFont (portFont);

    for (size_t i = 0; i < node->getInlets().size(); ++i)
    {
        auto p = getInletPos (*node, static_cast<int>(i));
        const auto& port = node->getInlets()[i];
        NodePortType type = port.type;

        bool hasAudioInput = (port.audioData.getNumSamples() > 0 && port.audioData.getMagnitude (0, port.audioData.getNumSamples()) > 0.0001f);

        juce::Colour portCol = (hasAudioInput || type == NodePortType::Audio) ? juce::Colour (0xff06b6d4) : (type == NodePortType::Time ? juce::Colour (0xff8b5cf6) : (type == NodePortType::Message ? juce::Colour (0xff10b981) : juce::Colour (0xfff59e0b)));

        g.setColour (juce::Colour (0xff181825));
        g.fillEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f);
        g.setColour (portCol);
        g.fillEllipse (p.x - 2.5f, p.y - 2.5f, 5.0f, 5.0f);
        g.setColour (juce::Colours::white);
        g.drawEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f, 1.0f);

        // Smart Port Name Label without Truncation
        float lw = std::max (44.0f, getTextWidth (portFont, port.name) + 6.0f);
        g.setColour (portCol.withAlpha (0.95f));
        g.drawText (port.name, p.x - lw * 0.5f, p.y + 3.0f, lw, 11.0f, juce::Justification::centred);

        if (isDraggingCable && snappedInletNodeId == node->getId() && static_cast<int>(i) == snappedInletIdx)
        {
            g.setColour (juce::Colour (0xff06b6d4).withAlpha (0.35f));
            g.fillEllipse (p.x - 12.0f, p.y - 12.0f, 24.0f, 24.0f);
            g.setColour (juce::Colour (0xff38bdf8));
            g.drawEllipse (p.x - 12.0f, p.y - 12.0f, 24.0f, 24.0f, 2.0f);
        }
    }

    // Draw 128px Dot Visualizers if enabled
    float curVisY = y + h - (node->isShowDelaylineEnabled() ? 128.0f : 0.0f) - (node->isShowPipeEnabled() ? 128.0f : 0.0f) - 6.0f;
    if (node->isShowDelaylineEnabled())
    {
        drawDelaylineDots (g, *node, x + 6.0f, curVisY, w - 12.0f, 122.0f);
        curVisY += 128.0f;
    }
    if (node->isShowPipeEnabled())
    {
        drawControlPipeDots (g, *node, x + 6.0f, curVisY, w - 12.0f, 122.0f);
        curVisY += 128.0f;
    }

    // Outlets (Bottom Edge Dots with Smart Spaced Labels)
    for (size_t i = 0; i < node->getOutlets().size(); ++i)
    {
        auto p = getOutletPos (*node, static_cast<int>(i));
        const auto& port = node->getOutlets()[i];
        NodePortType type = port.type;

        juce::Colour portCol = (type == NodePortType::Audio) ? juce::Colour (0xff06b6d4) : (type == NodePortType::Time ? juce::Colour (0xff8b5cf6) : (type == NodePortType::Message ? juce::Colour (0xff10b981) : juce::Colour (0xfff59e0b)));

        g.setColour (juce::Colour (0xff181825));
        g.fillEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f);
        g.setColour (portCol);
        g.fillEllipse (p.x - 2.5f, p.y - 2.5f, 5.0f, 5.0f);
        g.setColour (juce::Colours::white);
        g.drawEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f, 1.0f);

        // Smart Port Name Label without Truncation
        float lw = std::max (44.0f, getTextWidth (portFont, port.name) + 6.0f);
        g.setColour (portCol.withAlpha (0.95f));
        g.drawText (port.name, p.x - lw * 0.5f, p.y - 14.0f, lw, 11.0f, juce::Justification::centred);
    }
}

} // namespace time_dilation
