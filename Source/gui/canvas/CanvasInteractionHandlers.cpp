#include "../RelativisticCanvasComponent.h"
#include "../../dsp/RelativisticNodeObjects.h"
#include "../../dsp/RelativisticExpressionParser.h"
#include "../StepSequencerGridComponent.h"
#include <cmath>

namespace time_dilation
{

static float getDistanceToBezier (juce::Point<float> p1, juce::Point<float> p2, juce::Point<float> mousePos)
{
    float dy = std::max (30.0f, std::abs (p2.y - p1.y) * 0.5f);
    juce::Point<float> cp1 = { p1.x, p1.y + dy };
    juce::Point<float> cp2 = { p2.x, p2.y - dy };

    float minDist = 99999.0f;
    const int numSamples = 24;
    for (int i = 0; i <= numSamples; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(numSamples);
        float u = 1.0f - t;
        float x = u*u*u*p1.x + 3.0f*u*u*t*cp1.x + 3.0f*u*t*t*cp2.x + t*t*t*p2.x;
        float y = u*u*u*p1.y + 3.0f*u*u*t*cp1.y + 3.0f*u*t*t*cp2.y + t*t*t*p2.y;

        float dist = mousePos.getDistanceFrom ({ x, y });
        if (dist < minDist) minDist = dist;
    }
    return minDist;
}

void RelativisticCanvasComponent::mouseDown (const juce::MouseEvent& e)
{
    juce::Point<float> mousePos = e.position;
    bool isShift = e.mods.isShiftDown();

    // Reset snap & drag states
    snappedInletNodeId = 0;
    snappedInletIdx = -1;

    // 0. Check Click on Number Box or Slider Control for Direct Mode Value Scrubbing
    if (!isEditMode)
    {
        for (auto it = nodeGraph.getNodes().rbegin(); it != nodeGraph.getNodes().rend(); ++it)
        {
            const auto& node = *it;
            float nx = node->getX() + panX;
            float ny = node->getY() + panY;
            float nw = getNodeWidth (*node);
            float nh = getNodeHeight (*node);

            bool isNum = (node->getTypeName() == "number" || node->getTypeName() == "num" || node->getTypeName() == "nb");
            if (isNum && mousePos.x >= nx && mousePos.x <= nx + nw && mousePos.y >= ny && mousePos.y <= ny + nh)
            {
                valueDragNodeId = node->getId();
                valueDragStartMouseY = e.position.y;
                valueDragStartVal = node->getParameter ("value", 0.0f);
                return;
            }
        }
    }

    // 1. Check Outlet / Inlet Port Click (Cable Patching Start & Bang Triggering)
    for (const auto& node : nodeGraph.getNodes())
    {
        for (size_t i = 0; i < node->getOutlets().size(); ++i)
        {
            auto p = getOutletPos (*node, static_cast<int>(i));
            if (p.getDistanceFrom (mousePos) < 12.0f)
            {
                isDraggingCable = true;
                cableSrcNodeId = node->getId();
                cableSrcOutletIdx = static_cast<int>(i);
                cableDragPos = p;
                repaint();
                return;
            }
        }

        // Click on bang LED button directly triggers bang pulse in Perform Mode
        if (node->getTypeName() == "bang" || node->getTypeName() == "b" || node->getTypeName() == "bang~" || node->getTypeName() == "b~")
        {
            float nx = node->getX() + panX;
            float ny = node->getY() + panY;
            float nw = getNodeWidth (*node);
            float nh = getNodeHeight (*node);

            float cx = nx + nw - 24.0f;
            float cy = ny + nh * 0.5f;

            if (mousePos.getDistanceFrom ({ cx, cy }) < 12.0f)
            {
                node->receiveMessage ("bang", 1.0f);
                showNotificationBanner ("Triggered BANG pulse on [" + node->getLabel() + "]", false);
                repaint();
                return;
            }
        }
    }

    // 2. Check Cable Selection Click
    for (const auto& conn : nodeGraph.getConnections())
    {
        auto srcNode = nodeGraph.getNodeById (conn.sourceNodeId);
        auto destNode = nodeGraph.getNodeById (conn.destNodeId);
        if (srcNode && destNode)
        {
            if (conn.sourceOutletIdx < static_cast<int>(srcNode->getOutlets().size()) &&
                conn.destInletIdx < static_cast<int>(destNode->getInlets().size()))
            {
                auto p1 = getOutletPos (*srcNode, conn.sourceOutletIdx);
                auto p2 = getInletPos (*destNode, conn.destInletIdx);

                if (mousePos.getDistanceFrom (p1) < 10.0f || mousePos.getDistanceFrom (p2) < 10.0f)
                {
                    selectedConnectionId = conn.id;
                    selectedNodeIds.clear();
                    rebuildInspector();
                    repaint();
                    return;
                }

                float distToCurve = getDistanceToBezier (p1, p2, mousePos);
                if (distToCurve < 14.0f)
                {
                    selectedConnectionId = conn.id;
                    selectedNodeIds.clear();
                    rebuildInspector();
                    repaint();
                    return;
                }
            }
        }
    }

    // 3. Check Node Box Click
    for (auto it = nodeGraph.getNodes().rbegin(); it != nodeGraph.getNodes().rend(); ++it)
    {
        const auto& node = *it;
        float nx = node->getX() + panX;
        float ny = node->getY() + panY;
        float nw = getNodeWidth (*node);
        float nh = getNodeHeight (*node);

        if (mousePos.x >= nx && mousePos.x <= nx + nw &&
            mousePos.y >= ny && mousePos.y <= ny + nh)
        {
            if (isShift)
            {
                if (selectedNodeIds.count (node->getId()) > 0)
                    selectedNodeIds.erase (node->getId());
                else
                    selectedNodeIds.insert (node->getId());
            }
            else
            {
                if (selectedNodeIds.count (node->getId()) == 0)
                {
                    selectedNodeIds.clear();
                    selectedNodeIds.insert (node->getId());
                }
            }

            selectedConnectionId = 0;
            if (isEditMode)
            {
                draggingNodeId = node->getId();
                dragOffset = { mousePos.x - nx, mousePos.y - ny };

                initialNodePositions.clear();
                for (int id : selectedNodeIds)
                {
                    auto n = nodeGraph.getNodeById (id);
                    if (n) initialNodePositions[id] = { n->getX(), n->getY() };
                }
            }

            rebuildInspector();
            repaint();
            return;
        }
    }

    // 4. Empty Canvas Click -> Panel Panning OR Rubberband Selection
    if (e.mods.isMiddleButtonDown() || e.mods.isRightButtonDown() || (e.mods.isLeftButtonDown() && juce::KeyPress::isKeyCurrentlyDown (juce::KeyPress::spaceKey)))
    {
        isCanvasPanning = true;
        panStartPos = e.position;
        initialPanOffset = { panX, panY };
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);
        return;
    }

    if (isShift)
    {
        selectedConnectionId = 0;
        isMarqueeDragging = true;
        marqueeRect = { mousePos.x, mousePos.y, 0.0f, 0.0f };
    }
    else
    {
        selectedNodeIds.clear();
        selectedConnectionId = 0;
        isCanvasPanning = true;
        panStartPos = e.position;
        initialPanOffset = { panX, panY };
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);
        rebuildInspector();
    }
    repaint();
}

void RelativisticCanvasComponent::mouseDoubleClick (const juce::MouseEvent& e)
{
    juce::Point<float> mousePos = e.position;

    for (const auto& node : nodeGraph.getNodes())
    {
        float nx = node->getX() + panX;
        float ny = node->getY() + panY;
        float nw = getNodeWidth (*node);
        float nh = getNodeHeight (*node);

        if (mousePos.x >= nx && mousePos.x <= nx + nw &&
            mousePos.y >= ny && mousePos.y <= ny + nh)
        {
            if (auto numNode = std::dynamic_pointer_cast<NumberNode> (node))
            {
                auto alert = std::make_unique<juce::AlertWindow> ("SET NUMBER VALUE", "Enter new floating-point value for [number]:", juce::AlertWindow::QuestionIcon);
                alert->addTextEditor ("numVal", juce::String (numNode->getParameter ("value", 0.0f)), "Value:");
                alert->addButton ("OK", 1);
                alert->addButton ("Cancel", 0);
                alert->enterModalState (true, juce::ModalCallbackFunction::create ([this, numNode, a = alert.get()] (int res) {
                    if (res == 1)
                    {
                        std::string exprText = a->getTextEditorContents ("numVal").toStdString();
                        float val = static_cast<float>(RelativisticExpressionParser::evaluateExpression (exprText, {}));
                        numNode->setParameter ("value", val);
                        rebuildInspector();
                        repaint();
                    }
                }), true);
                return;
            }
            if (auto drumNode = std::dynamic_pointer_cast<DrumSequencerNode> (node))
            {
                auto* dialog = new juce::DialogWindow::LaunchOptions();
                dialog->dialogTitle = "SLATE SCI-FI 16-STEP DRUM MATRIX (" + juce::String (drumNode->getLabel()) + ")";
                dialog->content.setOwned (new StepSequencerGridComponent (drumNode));
                dialog->dialogBackgroundColour = juce::Colour (0xff070a12);
                dialog->escapeKeyTriggersCloseButton = true;
                dialog->useNativeTitleBar = true;
                dialog->resizable = true;
                dialog->launchAsync();
                return;
            }
            if (auto seqNode = std::dynamic_pointer_cast<StepSequencerNode> (node))
            {
                auto* dialog = new juce::DialogWindow::LaunchOptions();
                dialog->dialogTitle = "SLATE SCI-FI STEP & VALUE AUTOMATION MATRIX (" + juce::String (seqNode->getLabel()) + ")";
                dialog->content.setOwned (new StepSequencerGridComponent (seqNode));
                dialog->dialogBackgroundColour = juce::Colour (0xff070a12);
                dialog->escapeKeyTriggersCloseButton = true;
                dialog->useNativeTitleBar = true;
                dialog->resizable = true;
                dialog->launchAsync();
                return;
            }

            editingNodeId = node->getId();
            selectedNodeIds.clear();
            selectedNodeIds.insert (node->getId());
            inlineLabelEditor.setBounds (static_cast<int>(nx + 4), static_cast<int>(ny + 4), static_cast<int>(nw - 8), static_cast<int>(nh - 8));
            inlineLabelEditor.setText (node->getLabel(), false);
            inlineLabelEditor.setVisible (true);
            inlineLabelEditor.grabKeyboardFocus();
            inlineLabelEditor.selectAll();
            return;
        }
    }

    // Double-click empty canvas -> Open Object Search Menu
    showObjectSearchMenu (mousePos);
}

void RelativisticCanvasComponent::mouseDrag (const juce::MouseEvent& e)
{
    juce::Point<float> mousePos = e.position;

    // 0. Perform Mode Value Dragging (number / slider up/down scroll)
    if (!isEditMode && valueDragNodeId > 0)
    {
        auto node = nodeGraph.getNodeById (valueDragNodeId);
        if (node)
        {
            float deltaY = (e.position.y - valueDragStartMouseY);
            float step = (e.mods.isShiftDown()) ? 0.1f : 1.0f;
            float newVal = valueDragStartVal - deltaY * step;
            node->setParameter ("value", newVal);
            repaint();
            return;
        }
    }

    // 1. Node Dragging Positioning
    if (draggingNodeId > 0)
    {
        auto initAnchorIt = initialNodePositions.find (draggingNodeId);
        if (initAnchorIt != initialNodePositions.end())
        {
            float targetX = (mousePos.x - dragOffset.x - panX);
            float targetY = (mousePos.y - dragOffset.y - panY);

            if (snapToGrid)
            {
                targetX = std::round (targetX / gridSize) * gridSize;
                targetY = std::round (targetY / gridSize) * gridSize;
            }

            float deltaX = targetX - initAnchorIt->second.x;
            float deltaY = targetY - initAnchorIt->second.y;

            for (int id : selectedNodeIds)
            {
                auto n = nodeGraph.getNodeById (id);
                if (n)
                {
                    auto initIt = initialNodePositions.find (id);
                    if (initIt != initialNodePositions.end())
                    {
                        n->setPosition (initIt->second.x + deltaX, initIt->second.y + deltaY);
                    }
                    else
                    {
                        n->setPosition (n->getX() + deltaX, n->getY() + deltaY);
                    }
                }
            }
            markUnsavedChanges();
            repaint();
            return;
        }
    }

    // 2. Canvas Panning
    if (isCanvasPanning)
    {
        panX = initialPanOffset.x + (e.position.x - panStartPos.x);
        panY = initialPanOffset.y + (e.position.y - panStartPos.y);
        updateDraftObjectBounds();
        repaint();
        return;
    }

    // 3. Dragging Cable (with 24px Magnetic Port Snapping!)
    if (isDraggingCable)
    {
        cableDragPos = e.position;
        snappedInletNodeId = 0;
        snappedInletIdx = -1;

        float minDist = 24.0f;
        for (const auto& node : nodeGraph.getNodes())
        {
            if (node->getId() == cableSrcNodeId) continue;
            for (size_t i = 0; i < node->getInlets().size(); ++i)
            {
                auto p = getInletPos (*node, static_cast<int>(i));
                float dist = p.getDistanceFrom (e.position);
                if (dist < minDist)
                {
                    minDist = dist;
                    snappedInletNodeId = node->getId();
                    snappedInletIdx = static_cast<int>(i);
                    snappedInletPos = p;
                    cableDragPos = p;
                }
            }
        }
        repaint();
        return;
    }

    // 4. Marquee Selection Rubberband
    if (isMarqueeDragging)
    {
        float x1 = std::min (e.mouseDownPosition.x, e.position.x);
        float y1 = std::min (e.mouseDownPosition.y, e.position.y);
        float w = std::abs (e.position.x - e.mouseDownPosition.x);
        float h = std::abs (e.position.y - e.mouseDownPosition.y);
        marqueeRect = { x1, y1, w, h };

        for (const auto& node : nodeGraph.getNodes())
        {
            juce::Rectangle<float> nodeRect (node->getX() + panX, node->getY() + panY, getNodeWidth (*node), getNodeHeight (*node));
            if (marqueeRect.intersects (nodeRect))
            {
                selectedNodeIds.insert (node->getId());
            }
        }
        repaint();
        return;
    }

    // 5. Special Widget Dragging (Table Drawing & Number Value Scrubbing when Alt/Option key is held)
    if (selectedNodeIds.size() == 1 && e.mods.isAltDown())
    {
        auto n = nodeGraph.getNodeById (*selectedNodeIds.begin());
        if (auto tableNode = std::dynamic_pointer_cast<TableNode> (n))
        {
            float graphX = tableNode->getX() + panX + 8.0f;
            float graphY = tableNode->getY() + panY + 22.0f;
            float graphW = getNodeWidth (*tableNode) - 16.0f;
            float graphH = getNodeHeight (*tableNode) - 26.0f;

            if (e.mouseDownPosition.x >= graphX && e.mouseDownPosition.x <= graphX + graphW &&
                e.mouseDownPosition.y >= graphY && e.mouseDownPosition.y <= graphY + graphH)
            {
                float normX = std::clamp ((mousePos.x - graphX) / graphW, 0.0f, 1.0f);
                float normY = 1.0f - 2.0f * std::clamp ((mousePos.y - graphY) / graphH, 0.0f, 1.0f);
                tableNode->writeSampleNormalized (normX, normY);
                repaint();
                return;
            }
        }
        else if (auto numNode = std::dynamic_pointer_cast<NumberNode> (n))
        {
            float currVal = numNode->getParameter ("value", 0.0f);
            float step = e.mods.isShiftDown() ? 0.1f : 1.0f;
            float newVal = currVal - (static_cast<float>(e.getDistanceFromDragStartY()) * 0.1f * step);
            numNode->setParameter ("value", newVal);
            repaint();
            return;
        }
    }
}

void RelativisticCanvasComponent::mouseUp (const juce::MouseEvent& e)
{
    valueDragNodeId = 0;
    if (isCanvasPanning)
    {
        isCanvasPanning = false;
        setMouseCursor (juce::MouseCursor::NormalCursor);
    }
    if (isDraggingCable)
    {
        juce::Point<float> mousePos = e.position;

        for (const auto& node : nodeGraph.getNodes())
        {
            for (size_t i = 0; i < node->getInlets().size(); ++i)
            {
                auto p = getInletPos (*node, static_cast<int>(i));
                if ((snappedInletNodeId == node->getId() && snappedInletIdx == static_cast<int>(i)) || p.getDistanceFrom (mousePos) < 24.0f)
                {
                    auto srcNode = nodeGraph.getNodeById (cableSrcNodeId);
                    if (srcNode && cableSrcOutletIdx < static_cast<int>(srcNode->getOutlets().size()))
                    {
                        NodePortType srcType = srcNode->getOutlets()[cableSrcOutletIdx].type;
                        NodePortType destType = node->getInlets()[i].type;

                        bool isCompatible = (srcType == destType) ||
                                           (srcType == NodePortType::Time && destType == NodePortType::Control) ||
                                           (srcType == NodePortType::Control && destType == NodePortType::Time);

                        if (!isCompatible)
                        {
                            std::string srcName = (srcType == NodePortType::Audio) ? "Audio~" : (srcType == NodePortType::Time ? "Time" : "Control");
                            std::string destName = (destType == NodePortType::Audio) ? "Audio~" : (destType == NodePortType::Time ? "Time" : "Control");

                            std::string tip = "";
                            if (srcType == NodePortType::Audio && destType == NodePortType::Time)
                                tip = "Use [audio2time~] or [a2t~] to convert Audio~ into Time gamma!";
                            else if (srcType == NodePortType::Time && destType == NodePortType::Audio)
                                tip = "Use [time2audio~] or [t2a~] to convert Time gamma into Audio~!";
                            else if (srcType == NodePortType::Audio && destType == NodePortType::Control)
                                tip = "Use [env~] or [snapshot~] to convert Audio~ into Control!";
                            else
                                tip = "Use converter node!";

                            showNotificationBanner ("Incompatible Port Types (" + srcName + " -> " + destName + "). " + tip, true);
                        }
                        else
                        {
                            nodeGraph.pushUndoState();
                            nodeGraph.addConnection (cableSrcNodeId, cableSrcOutletIdx, node->getId(), static_cast<int>(i));
                            showNotificationBanner ("Connected " + srcNode->getTypeName() + " -> " + node->getTypeName(), false);
                        }
                    }
                    break;
                }
            }
        }

        isDraggingCable = false;
        cableSrcNodeId = 0;
        snappedInletNodeId = 0;
        snappedInletIdx = -1;
        repaint();
    }
    else if (isMarqueeDragging)
    {
        isMarqueeDragging = false;
        repaint();
    }

    draggingNodeId = 0;
}

void RelativisticCanvasComponent::mouseMove (const juce::MouseEvent& e)
{
    juce::Point<float> mousePos = e.position;
    HoveredPortInfo newHover;

    for (const auto& node : nodeGraph.getNodes())
    {
        for (size_t i = 0; i < node->getInlets().size(); ++i)
        {
            auto p = getInletPos (*node, static_cast<int>(i));
            if (p.getDistanceFrom (mousePos) < 12.0f)
            {
                newHover.nodeId = node->getId();
                newHover.portIdx = static_cast<int>(i);
                newHover.isInlet = true;
                newHover.pos = p;

                const auto& port = node->getInlets()[i];
                newHover.portName = port.name;

                if (port.audioData.getNumSamples() > 0 && port.audioData.getMagnitude (0, port.audioData.getNumSamples()) > 0.0001f)
                {
                    newHover.signalTypeName = "AUDIO RATE (CYAN)";
                    float mag = port.audioData.getMagnitude (0, port.audioData.getNumSamples());
                    newHover.routedValueText = "Audio Peak: " + juce::String (mag, 3).toStdString();
                }
                else if (port.type == NodePortType::Time)
                {
                    newHover.signalTypeName = "TIME DILATION (PURPLE)";
                    newHover.routedValueText = "Gamma: " + juce::String (port.timeGamma, 2).toStdString() + "x";
                }
                else
                {
                    newHover.signalTypeName = "CONTROL RATE (AMBER)";
                    newHover.routedValueText = "Val: " + juce::String (port.controlValue, 3).toStdString();
                }
                break;
            }
        }

        if (newHover.nodeId != 0) break;

        for (size_t i = 0; i < node->getOutlets().size(); ++i)
        {
            auto p = getOutletPos (*node, static_cast<int>(i));
            if (p.getDistanceFrom (mousePos) < 12.0f)
            {
                newHover.nodeId = node->getId();
                newHover.portIdx = static_cast<int>(i);
                newHover.isInlet = false;
                newHover.pos = p;

                const auto& port = node->getOutlets()[i];
                newHover.portName = port.name;

                if (port.type == NodePortType::Audio)
                {
                    newHover.signalTypeName = "AUDIO RATE (CYAN)";
                    float mag = port.audioData.getMagnitude (0, port.audioData.getNumSamples());
                    newHover.routedValueText = "Audio Peak: " + juce::String (mag, 3).toStdString();
                }
                else if (port.type == NodePortType::Time)
                {
                    newHover.signalTypeName = "TIME DILATION (PURPLE)";
                    newHover.routedValueText = "Gamma: " + juce::String (port.timeGamma, 2).toStdString() + "x";
                }
                else
                {
                    newHover.signalTypeName = "CONTROL RATE (AMBER)";
                    newHover.routedValueText = "Val: " + juce::String (port.controlValue, 3).toStdString();
                }
                break;
            }
        }
    }

    if (newHover.nodeId != hoveredPort.nodeId || newHover.portIdx != hoveredPort.portIdx || newHover.isInlet != hoveredPort.isInlet)
    {
        hoveredPort = newHover;
        repaint();
    }
}

} // namespace time_dilation
