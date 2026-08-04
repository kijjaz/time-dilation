#include "StepSequencerGridComponent.h"
#include "FontManager.h"

namespace time_dilation
{

static const std::vector<std::string> drumTrackNames = {
    "KICK (36)", "SNARE (38)", "CLAP (39)", "C-HAT (42)", "O-HAT (46)", "PERC (48)"
};

StepSequencerGridComponent::StepSequencerGridComponent (std::shared_ptr<DrumSequencerNode> node)
    : gridType (SequencerGridType::Drum), drumNode (node)
{
    setupUI();
}

StepSequencerGridComponent::StepSequencerGridComponent (std::shared_ptr<StepSequencerNode> node)
    : gridType (SequencerGridType::Step), stepNode (node)
{
    setupUI();
}

StepSequencerGridComponent::~StepSequencerGridComponent()
{
    stopTimer();
}

void StepSequencerGridComponent::setupUI()
{
    addAndMakeVisible (presetTrapBtn);
    addAndMakeVisible (presetJerseyBtn);
    addAndMakeVisible (presetGlitchBtn);
    addAndMakeVisible (presetPolyBtn);
    addAndMakeVisible (clearBtn);
    addAndMakeVisible (stepCountBox);
    addAndMakeVisible (timeSigBox);
    addAndMakeVisible (bpmLabel);
    addAndMakeVisible (bpmSlider);
    addAndMakeVisible (zoomLabel);
    addAndMakeVisible (zoomSlider);

    stepCountBox.addItem ("8 STEPS", 1);
    stepCountBox.addItem ("16 STEPS", 2);
    stepCountBox.addItem ("32 STEPS", 3);
    stepCountBox.setSelectedId (2, juce::dontSendNotification);

    stepCountBox.onChange = [this] {
        int steps = 16;
        if (stepCountBox.getSelectedId() == 1) steps = 8;
        else if (stepCountBox.getSelectedId() == 3) steps = 32;

        if (drumNode) drumNode->setStepCount (steps);
        else if (stepNode) stepNode->setStepCount (steps);
        repaint();
    };

    timeSigBox.addItem ("4/4 METER", 1);
    timeSigBox.addItem ("3/4 WALTZ", 2);
    timeSigBox.addItem ("5/4 ODD METER", 3);
    timeSigBox.addItem ("7/4 ODD METER", 4);
    timeSigBox.addItem ("7/8 COMPLEX", 5);
    timeSigBox.addItem ("9/8 COMPLEX", 6);
    timeSigBox.addItem ("12/8 COMPOUND", 7);
    timeSigBox.setSelectedId (1, juce::dontSendNotification);

    timeSigBox.onChange = [this] {
        int id = timeSigBox.getSelectedId();
        int beats = 4, val = 4;
        if (id == 2) { beats = 3; val = 4; }
        else if (id == 3) { beats = 5; val = 4; }
        else if (id == 4) { beats = 7; val = 4; }
        else if (id == 5) { beats = 7; val = 8; }
        else if (id == 6) { beats = 9; val = 8; }
        else if (id == 7) { beats = 12; val = 8; }

        if (drumNode) drumNode->setTimeSignature (beats, val);
        else if (stepNode) stepNode->setTimeSignature (beats, val);
        repaint();
    };

    presetTrapBtn.onClick = [this] { applyPreset ("Future Bass Trap Beat"); };
    presetJerseyBtn.onClick = [this] { applyPreset ("Jersey Bounce"); };
    presetGlitchBtn.onClick = [this] { applyPreset ("Glitch Roll"); };
    presetPolyBtn.onClick = [this] { applyPreset ("Poly Rhythm"); };
    clearBtn.onClick = [this] { applyPreset ("Clear"); };

    bpmSlider.setRange (20.0, 300.0, 1.0);
    float currentBpm = 120.0f;
    if (drumNode) currentBpm = drumNode->getParameter ("bpm", 120.0f);
    else if (stepNode) currentBpm = stepNode->getParameter ("bpm", 120.0f);
    bpmSlider.setValue (currentBpm);

    bpmSlider.onValueChange = [this] {
        if (drumNode) drumNode->setParameter ("bpm", static_cast<float>(bpmSlider.getValue()));
        else if (stepNode) stepNode->setParameter ("bpm", static_cast<float>(bpmSlider.getValue()));
    };

    zoomSlider.setRange (0.5, 3.0, 0.1);
    zoomSlider.setValue (1.0);
    zoomSlider.onValueChange = [this] {
        zoomScale = static_cast<float>(zoomSlider.getValue());
        repaint();
    };

    startTimerHz (30);
    setSize (920, 560);
}

int StepSequencerGridComponent::getStepCount() const
{
    if (drumNode) return drumNode->getStepCount();
    if (stepNode) return stepNode->getStepCount();
    return 16;
}

int StepSequencerGridComponent::getCurrentStep() const
{
    if (drumNode) return drumNode->getCurrentStep();
    if (stepNode) return stepNode->getCurrentStep();
    return 0;
}

void StepSequencerGridComponent::applyPreset (const std::string& presetName)
{
    if (presetName == "Clear")
    {
        int numSteps = getStepCount();
        if (drumNode)
        {
            for (int s = 0; s < numSteps; ++s)
                for (int t = 0; t < 6; ++t)
                    drumNode->setDrumStep (s, t, false);
        }
        else if (stepNode)
        {
            for (int s = 0; s < numSteps; ++s)
                stepNode->setStepValue (s, 60.0f);
        }
    }
    else if (drumNode)
    {
        drumNode->setPresetPattern (presetName);
    }
    repaint();
}

void StepSequencerGridComponent::timerCallback()
{
    repaint();
}

void StepSequencerGridComponent::resized()
{
    auto area = getLocalBounds().reduced (12);
    auto topRow = area.removeFromTop (36);

    presetTrapBtn.setBounds (topRow.removeFromLeft (90));
    topRow.removeFromLeft (4);
    presetJerseyBtn.setBounds (topRow.removeFromLeft (110));
    topRow.removeFromLeft (4);
    presetGlitchBtn.setBounds (topRow.removeFromLeft (95));
    topRow.removeFromLeft (4);
    presetPolyBtn.setBounds (topRow.removeFromLeft (100));
    topRow.removeFromLeft (4);
    clearBtn.setBounds (topRow.removeFromLeft (65));
    topRow.removeFromLeft (8);

    stepCountBox.setBounds (topRow.removeFromLeft (90));
    topRow.removeFromLeft (6);
    timeSigBox.setBounds (topRow.removeFromLeft (110));
    topRow.removeFromLeft (8);

    bpmLabel.setBounds (topRow.removeFromLeft (32));
    bpmSlider.setBounds (topRow.removeFromLeft (80));
    topRow.removeFromLeft (8);

    zoomLabel.setBounds (topRow.removeFromLeft (42));
    zoomSlider.setBounds (topRow.removeFromLeft (80));
}

void StepSequencerGridComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff070a12));

    auto bounds = getLocalBounds().toFloat();
    float topBarH = 50.0f;
    float trackLabelWidth = 100.0f;
    int numSteps = getStepCount();
    int activeStep = getCurrentStep();
    int beatsPerBar = drumNode ? drumNode->getBeatsPerBar() : (stepNode ? stepNode->getBeatsPerBar() : 4);

    g.setColour (juce::Colour (0xff1e293b));
    g.drawHorizontalLine (static_cast<int>(topBarH), 0.0f, bounds.getWidth());

    juce::Rectangle<float> gridRect (trackLabelWidth, topBarH + 10.0f,
                                     bounds.getWidth() - trackLabelWidth - 12.0f,
                                     bounds.getHeight() - topBarH - 20.0f);

    float baseCellW = gridRect.getWidth() / static_cast<float>(numSteps);
    float cellW = baseCellW * zoomScale;

    if (gridType == SequencerGridType::Drum && drumNode)
    {
        int numTracks = static_cast<int>(drumTrackNames.size());
        float trackH = (gridRect.getHeight() - 70.0f) / static_cast<float>(numTracks);

        // 1. Track Headers
        g.setFont (FontManager::getInstance().getOxaniumFont (11.0f, true));
        for (int t = 0; t < numTracks; ++t)
        {
            float y = gridRect.getY() + t * trackH;
            juce::Rectangle<float> lblRect (8.0f, y, trackLabelWidth - 12.0f, trackH - 2.0f);
            g.setColour (juce::Colour (0xff0f172a));
            g.fillRoundedRectangle (lblRect, 4.0f);
            g.setColour (juce::Colour (0xff94a3b8));
            g.drawText (drumTrackNames[t], lblRect.toNearestInt(), juce::Justification::centredLeft, true);
        }

        // 2. Step & Tuplet Grid
        for (int s = 0; s < numSteps; ++s)
        {
            float x = gridRect.getX() + s * cellW - scrollOffsetX;
            if (x + cellW < trackLabelWidth || x > bounds.getWidth()) continue;

            bool isBarLine = (s % beatsPerBar == 0);
            int subCount = drumNode->getStepSubdivision (s);

            for (int t = 0; t < numTracks; ++t)
            {
                float y = gridRect.getY() + t * trackH;
                juce::Rectangle<float> stepSlotRect (x + 2.0f, y + 2.0f, cellW - 4.0f, trackH - 4.0f);

                // Slot background frame
                g.setColour (isBarLine ? juce::Colour (0xff1e293b) : juce::Colour (0xff0f172a));
                g.fillRoundedRectangle (stepSlotRect, 4.0f);
                g.setColour (isBarLine ? juce::Colour (0xfff59e0b).withAlpha (0.4f) : juce::Colour (0xff334155));
                g.drawRoundedRectangle (stepSlotRect, 4.0f, isBarLine ? 1.5f : 1.0f);

                float subW = stepSlotRect.getWidth() / static_cast<float>(subCount);

                for (int sub = 0; sub < subCount; ++sub)
                {
                    juce::Rectangle<float> padRect (stepSlotRect.getX() + sub * subW + 1.0f,
                                                     stepSlotRect.getY() + 1.0f,
                                                     subW - 2.0f,
                                                     stepSlotRect.getHeight() - 2.0f);

                    bool isActive = drumNode->getSubStep (s, sub, t);
                    float vel = drumNode->getDrumStepVelocity (s, t);

                    if (isActive)
                    {
                        juce::Colour fillCol = (t == 0 || t == 1)
                            ? juce::Colour (0xfff59e0b).withMultipliedAlpha (0.4f + 0.6f * vel)
                            : juce::Colour (0xff06b6d4).withMultipliedAlpha (0.4f + 0.6f * vel);
                        g.setColour (fillCol);
                        g.fillRoundedRectangle (padRect, 3.0f);
                        g.setColour (fillCol.brighter (0.5f));
                        g.drawRoundedRectangle (padRect, 3.0f, 1.2f);
                    }
                    else if (subCount > 1)
                    {
                        g.setColour (juce::Colour (0xff8b5cf6).withAlpha (0.15f)); // Tuplet tint
                        g.fillRoundedRectangle (padRect, 2.0f);
                        g.setColour (juce::Colour (0xff8b5cf6).withAlpha (0.4f));
                        g.drawRoundedRectangle (padRect, 2.0f, 0.8f);
                    }
                }

                // Draw Tuplet Badge if subCount > 1
                if (subCount > 1)
                {
                    g.setFont (FontManager::getInstance().getOxaniumFont (9.0f, true));
                    g.setColour (juce::Colour (0xff8b5cf6));
                    g.drawText (juce::String (subCount) + "T", stepSlotRect.toNearestInt(), juce::Justification::topRight, true);
                }
            }
        }

        // 3. Automation Lane
        float autoY = gridRect.getY() + numTracks * trackH + 10.0f;
        float autoH = gridRect.getHeight() - (numTracks * trackH + 10.0f);

        g.setFont (FontManager::getInstance().getOxaniumFont (10.0f, true));
        g.setColour (juce::Colour (0xff64748b));
        g.drawText ("ACCENT / VELOCITY", juce::Rectangle<int> (8, static_cast<int>(autoY), static_cast<int>(trackLabelWidth - 12), static_cast<int>(autoH)), juce::Justification::centredLeft, true);

        for (int s = 0; s < numSteps; ++s)
        {
            float x = gridRect.getX() + s * cellW - scrollOffsetX;
            if (x + cellW < trackLabelWidth || x > bounds.getWidth()) continue;

            float vel = drumNode->getDrumStepVelocity (s, 0);
            juce::Rectangle<float> barArea (x + 4.0f, autoY, cellW - 8.0f, autoH);
            g.setColour (juce::Colour (0xff0f172a));
            g.fillRect (barArea);

            float fillH = autoH * vel;
            juce::Rectangle<float> barFill (x + 4.0f, autoY + autoH - fillH, cellW - 8.0f, fillH);
            g.setColour (juce::Colour (0xff8b5cf6).withAlpha (0.8f));
            g.fillRoundedRectangle (barFill, 2.0f);
        }

        // Active Glowing Step Cursor Line
        if (activeStep >= 0 && activeStep < numSteps)
        {
            float cursorX = gridRect.getX() + activeStep * cellW + cellW * 0.5f - scrollOffsetX;
            g.setColour (juce::Colour (0xfff59e0b));
            g.drawLine (cursorX, gridRect.getY(), cursorX, gridRect.getBottom(), 3.0f);
        }
    }
    else if (gridType == SequencerGridType::Step && stepNode)
    {
        float pitchH = (gridRect.getHeight() - 60.0f) * 0.6f;
        float autoY = gridRect.getY() + pitchH + 10.0f;
        float autoH = gridRect.getHeight() - pitchH - 10.0f;

        g.setFont (FontManager::getInstance().getOxaniumFont (11.0f, true));
        g.setColour (juce::Colour (0xff94a3b8));
        g.drawText ("PITCH & TUPLETS", juce::Rectangle<int> (8, static_cast<int>(gridRect.getY()), static_cast<int>(trackLabelWidth - 12), 24), juce::Justification::centredLeft, true);
        g.drawText ("VALUE AUTO", juce::Rectangle<int> (8, static_cast<int>(autoY), static_cast<int>(trackLabelWidth - 12), 24), juce::Justification::centredLeft, true);

        for (int s = 0; s < numSteps; ++s)
        {
            float x = gridRect.getX() + s * cellW - scrollOffsetX;
            if (x + cellW < trackLabelWidth || x > bounds.getWidth()) continue;

            int subCount = stepNode->getStepSubdivision (s);
            juce::Rectangle<float> padRect (x + 2.0f, gridRect.getY() + 30.0f, cellW - 4.0f, pitchH - 30.0f);
            g.setColour (juce::Colour (0xff0d1322));
            g.fillRoundedRectangle (padRect, 4.0f);
            g.setColour (juce::Colour (0xfff59e0b));
            g.drawRoundedRectangle (padRect, 4.0f, 1.5f);

            float subW = padRect.getWidth() / static_cast<float>(subCount);
            for (int sub = 0; sub < subCount; ++sub)
            {
                float subPitch = stepNode->getSubStepValue (s, sub);
                juce::Rectangle<float> subPad (padRect.getX() + sub * subW + 1.0f, padRect.getY() + 1.0f, subW - 2.0f, padRect.getHeight() - 2.0f);
                if (subCount > 1)
                {
                    g.setColour (juce::Colour (0xff8b5cf6).withAlpha (0.2f));
                    g.fillRoundedRectangle (subPad, 2.0f);
                    g.setColour (juce::Colour (0xff8b5cf6));
                    g.drawRoundedRectangle (subPad, 2.0f, 0.8f);
                }

                g.setFont (FontManager::getInstance().getOxaniumFont (subCount > 3 ? 9.0f : 12.0f, true));
                g.setColour (juce::Colour (0xffffffff));
                g.drawText (juce::String (static_cast<int>(subPitch)), subPad.toNearestInt(), juce::Justification::centred, true);
            }

            // Value Automation Vertical Bar
            float velVal = stepNode->getStepVelocity (s);
            juce::Rectangle<float> barArea (x + 4.0f, autoY + 30.0f, cellW - 8.0f, autoH - 30.0f);
            g.setColour (juce::Colour (0xff0f172a));
            g.fillRect (barArea);

            float fillH = barArea.getHeight() * velVal;
            juce::Rectangle<float> barFill (x + 4.0f, autoY + 30.0f + (barArea.getHeight() - fillH), cellW - 8.0f, fillH);
            g.setColour (juce::Colour (0xff06b6d4));
            g.fillRoundedRectangle (barFill, 2.0f);
        }

        // Active Glowing Step Cursor Line
        if (activeStep >= 0 && activeStep < numSteps)
        {
            float cursorX = gridRect.getX() + activeStep * cellW + cellW * 0.5f - scrollOffsetX;
            g.setColour (juce::Colour (0xfff59e0b));
            g.drawLine (cursorX, gridRect.getY(), cursorX, gridRect.getBottom(), 3.0f);
        }
    }
}

void StepSequencerGridComponent::showTupletContextMenu (int stepIdx)
{
    juce::PopupMenu menu;
    menu.addSectionHeader ("STEP TUPLET SUB-DIVISION");
    menu.addItem (1, "1 - Straight (1/16)");
    menu.addItem (2, "2 - Duplet (1/32)");
    menu.addItem (3, "3 - Triplet");
    menu.addItem (4, "4 - Quadruplet");
    menu.addItem (5, "5 - Quintuplet");
    menu.addItem (6, "6 - Sextuplet");
    menu.addItem (7, "7 - Septuplet");
    menu.addItem (8, "8 - Octuplet");
    menu.addItem (9, "9 - Nonuplet");
    menu.addItem (10, "10 - Decuplet");
    menu.addItem (11, "11 - Undecuplet");
    menu.addItem (12, "12 - Dodecuplet");
    menu.addItem (13, "13 - Tridecuplet");
    menu.addItem (16, "16 - Hexadecuplet");
    menu.addSeparator();
    menu.addItem (100, "Custom Tuplet (N)...");

    menu.showMenuAsync (juce::PopupMenu::Options(), [this, stepIdx] (int result) {
        if (result >= 1 && result <= 16)
        {
            if (drumNode) drumNode->setStepSubdivision (stepIdx, result);
            else if (stepNode) stepNode->setStepSubdivision (stepIdx, result);
            repaint();
        }
        else if (result == 100)
        {
            auto alert = std::make_unique<juce::AlertWindow> ("CUSTOM TUPLET SUB-DIVISION", "Enter custom tuplet integer division (e.g. 11, 13, 17, 19, 23, 31, 64):", juce::AlertWindow::QuestionIcon);
            alert->addTextEditor ("tupletVal", "11", "Tuplet N:");
            alert->addButton ("OK", 1);
            alert->addButton ("Cancel", 0);
            alert->enterModalState (true, juce::ModalCallbackFunction::create ([this, stepIdx, a = alert.get()] (int res) {
                if (res == 1)
                {
                    int n = a->getTextEditorContents ("tupletVal").getIntValue();
                    if (n >= 1 && n <= 64)
                    {
                        if (drumNode) drumNode->setStepSubdivision (stepIdx, n);
                        else if (stepNode) stepNode->setStepSubdivision (stepIdx, n);
                        repaint();
                    }
                }
            }), true);
        }
    });
}

void StepSequencerGridComponent::mouseDown (const juce::MouseEvent& e)
{
    auto bounds = getLocalBounds().toFloat();
    float topBarH = 50.0f;
    float trackLabelWidth = 100.0f;
    int numSteps = getStepCount();

    juce::Rectangle<float> gridRect (trackLabelWidth, topBarH + 10.0f,
                                     bounds.getWidth() - trackLabelWidth - 12.0f,
                                     bounds.getHeight() - topBarH - 20.0f);

    if (!gridRect.contains (e.position)) return;

    float cellW = (gridRect.getWidth() / static_cast<float>(numSteps)) * zoomScale;
    int stepIdx = static_cast<int>((e.position.x - gridRect.getX() + scrollOffsetX) / cellW);
    stepIdx = std::clamp (stepIdx, 0, numSteps - 1);

    if (e.mods.isPopupMenu() || e.mods.isRightButtonDown())
    {
        showTupletContextMenu (stepIdx);
        return;
    }

    int subCount = drumNode ? drumNode->getStepSubdivision (stepIdx) : (stepNode ? stepNode->getStepSubdivision (stepIdx) : 1);
    float stepX = gridRect.getX() + stepIdx * cellW - scrollOffsetX;
    float relXInStep = e.position.x - stepX;
    int subStepIdx = std::clamp (static_cast<int>((relXInStep / cellW) * subCount), 0, subCount - 1);

    if (gridType == SequencerGridType::Drum && drumNode)
    {
        float trackH = (gridRect.getHeight() - 70.0f) / 6.0f;
        float relY = e.position.y - gridRect.getY();

        if (relY < 6.0f * trackH)
        {
            int trackIdx = static_cast<int>(relY / trackH);
            trackIdx = std::clamp (trackIdx, 0, 5);

            bool currentState = drumNode->getSubStep (stepIdx, subStepIdx, trackIdx);
            drumNode->setSubStep (stepIdx, subStepIdx, trackIdx, !currentState);

            draggingTrackIdx = trackIdx;
            draggingStepIdx = stepIdx;
            draggingSubStepIdx = subStepIdx;
            isDraggingValue = false;
        }
        else
        {
            isDraggingValue = true;
            draggingStepIdx = stepIdx;
            dragStartPos = e.getPosition();
        }
    }
    else if (gridType == SequencerGridType::Step && stepNode)
    {
        float pitchH = (gridRect.getHeight() - 60.0f) * 0.6f;
        if (e.position.y <= gridRect.getY() + pitchH)
        {
            float currentPitch = stepNode->getSubStepValue (stepIdx, subStepIdx);
            stepNode->setSubStepValue (stepIdx, subStepIdx, e.mods.isShiftDown() ? currentPitch - 1.0f : currentPitch + 1.0f);
        }
        else
        {
            isDraggingValue = true;
            draggingStepIdx = stepIdx;
            dragStartPos = e.getPosition();
        }
    }

    repaint();
}

void StepSequencerGridComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (isDraggingValue && draggingStepIdx >= 0)
    {
        auto bounds = getLocalBounds().toFloat();
        float topBarH = 50.0f;
        float trackLabelWidth = 100.0f;
        int numSteps = getStepCount();
        juce::Rectangle<float> gridRect (trackLabelWidth, topBarH + 10.0f, bounds.getWidth() - trackLabelWidth - 12.0f, bounds.getHeight() - topBarH - 20.0f);

        float autoY = gridRect.getY() + gridRect.getHeight() - 60.0f;
        float normVal = 1.0f - ((e.position.y - autoY) / 60.0f);
        normVal = std::clamp (normVal, 0.05f, 1.0f);

        if (drumNode) drumNode->setDrumStep (draggingStepIdx, 0, drumNode->getDrumStep (draggingStepIdx, 0), normVal);
        else if (stepNode) stepNode->setStepVelocity (draggingStepIdx, normVal);

        repaint();
    }
}

void StepSequencerGridComponent::mouseUp (const juce::MouseEvent&)
{
    draggingTrackIdx = -1;
    draggingStepIdx = -1;
    draggingSubStepIdx = -1;
    isDraggingValue = false;
}

void StepSequencerGridComponent::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    zoomScale = std::clamp (zoomScale + wheel.deltaY * 0.25f, 0.5f, 3.0f);
    zoomSlider.setValue (zoomScale, juce::dontSendNotification);
    repaint();
}

} // namespace time_dilation
