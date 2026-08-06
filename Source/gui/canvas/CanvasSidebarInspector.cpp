#include "../RelativisticCanvasComponent.h"
#include "../../dsp/RelativisticNodeObjects.h"
#include "../../dsp/RelativisticExpressionParser.h"
#include <cmath>

namespace time_dilation
{

void RelativisticCanvasComponent::rebuildInspector()
{
    propertyRows.clear();
    methodButtons.clear();

    int primaryId = !selectedNodeIds.empty() ? *selectedNodeIds.begin() : 0;
    auto node = nodeGraph.getNodeById (primaryId);
    if (!node)
    {
        inspectorTitleLabel.setText ("INSPECTOR: NO NODE SELECTED", juce::dontSendNotification);
        formulaEditor.setText ("// Select a node on the canvas to view or edit its C++ / DSP math formula.");
        resized();
        return;
    }

    inspectorTitleLabel.setText ("INSPECTOR: [" + node->getLabel() + "]", juce::dontSendNotification);
    formulaEditor.setText (node->getFormulaScript());

    std::string typeName = node->getTypeName();
    std::string label = node->getLabel();
    std::string formula = label;
    if (typeName == "expr" && label.rfind ("expr ", 0) == 0)             formula = label.substr (5);
    else if (typeName == "expr~" && label.rfind ("expr~ ", 0) == 0)       formula = label.substr (6);
    else if (typeName == "fexpr~" && label.rfind ("fexpr~ ", 0) == 0)     formula = label.substr (7);

    exprFormulaLabel.setText ("MATH EXPRESSION (" + typeName + "):", juce::dontSendNotification);
    exprFormulaEditor.setText (formula);

    auto defs = node->getParameterDefs();
    for (const auto& def : defs)
    {
        InspectorPropertyRow row;
        row.key = def.key;
        std::string paramKey = def.key;

        row.label = std::make_unique<juce::Label>();
        row.label->setText (def.name + ":", juce::dontSendNotification);
        row.label->setFont (juce::FontOptions (12.0f, juce::Font::bold));
        inspectorContainer.addAndMakeVisible (*row.label);

        if (def.type == ParameterType::Toggle || (def.minValue == 0.0f && def.maxValue == 1.0f && def.isInteger))
        {
            bool isOn = (def.value > 0.5f);
            row.btnToggle = std::make_unique<juce::TextButton> (isOn ? "ON (1)" : "OFF (0)");
            row.btnToggle->setColour (juce::TextButton::buttonColourId, isOn ? juce::Colour (0xff15803d) : juce::Colour (0xff374151));
            row.btnToggle->onClick = [this, primaryId, paramKey] {
                auto n = nodeGraph.getNodeById (primaryId);
                if (n)
                {
                    float current = n->getParameter (paramKey, 0.0f);
                    n->setParameter (paramKey, (current > 0.5f) ? 0.0f : 1.0f);
                    rebuildInspector();
                    repaint();
                }
            };
            inspectorContainer.addAndMakeVisible (*row.btnToggle);
        }
        else if (def.type == ParameterType::Symbol)
        {
            row.symbolEditor = std::make_unique<juce::TextEditor>();
            row.symbolEditor->setText (def.stringValue.empty() ? def.expression : def.stringValue);
            row.symbolEditor->onReturnKey = [this, primaryId, paramKey, ed = row.symbolEditor.get()] {
                auto n = nodeGraph.getNodeById (primaryId);
                if (n) n->setParamExpression (paramKey, ed->getText().toStdString());
                repaint();
            };
            inspectorContainer.addAndMakeVisible (*row.symbolEditor);
        }
        else
        {
            row.slider = std::make_unique<juce::Slider>();
            row.slider->setSliderStyle (juce::Slider::LinearHorizontal);
            row.slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 120, 20);
            row.slider->setTextBoxIsEditable (true);

            row.slider->valueFromTextFunction = [] (const juce::String& text) -> double {
                return RelativisticExpressionParser::evaluateExpression (text.toStdString(), {});
            };

            float minR = def.minValue;
            float maxR = def.maxValue;
            if (minR >= maxR)
            {
                minR = (def.value < 0.0f) ? def.value * 2.0f : -10.0f;
                maxR = (def.value > 0.0f) ? def.value * 2.0f : 10.0f;
            }

            // Ensure slider range expands dynamically to accommodate current value (including negative numbers)
            if (def.value < minR) minR = def.value * 1.5f;
            if (def.value > maxR) maxR = def.value * 1.5f;

            if (def.type == ParameterType::Integer || def.isInteger)
            {
                row.slider->setRange (minR, maxR, 1.0);
                row.slider->setNumDecimalPlacesToDisplay (0);
            }
            else
            {
                row.slider->setRange (minR, maxR, 0.0001);
                row.slider->setNumDecimalPlacesToDisplay (4);
                if (minR > 0.0f && (paramKey == "frequency" || paramKey == "cutoff" || paramKey == "lfoSpeed" || paramKey == "centerFreq"))
                {
                    row.slider->setSkewFactorFromMidPoint (std::sqrt (minR * maxR));
                }
            }

            row.slider->setValue (def.value, juce::dontSendNotification);
            row.slider->onValueChange = [this, primaryId, paramKey, sl = row.slider.get()] {
                auto n = nodeGraph.getNodeById (primaryId);
                if (n)
                {
                    n->setParameter (paramKey, static_cast<float>(sl->getValue()));
                    repaint();
                }
            };
            inspectorContainer.addAndMakeVisible (*row.slider);
        }

        row.exprEditor = std::make_unique<juce::TextEditor>();
        row.exprEditor->setText (def.expression.empty() ? "expr: " + def.name : def.expression);
        row.exprEditor->setFont (juce::FontOptions (11.0f));
        row.exprEditor->onReturnKey = [this, primaryId, paramKey, ed = row.exprEditor.get()] {
            nodeGraph.pushUndoState();
            auto n = nodeGraph.getNodeById (primaryId);
            if (n) n->setParamExpression (paramKey, ed->getText().toStdString());
        };
        inspectorContainer.addAndMakeVisible (*row.exprEditor);

        bool hasMod = node->hasModulationInlet (paramKey);
        row.btnModInlet = std::make_unique<juce::TextButton> (hasMod ? "[- REMOVE MOD]" : "+ MOD INLET");
        row.btnModInlet->setColour (juce::TextButton::buttonColourId, hasMod ? juce::Colour (0xff991b1b) : juce::Colour (0xff0e7490));
        row.btnModInlet->onClick = [this, primaryId, paramKey, hasMod] {
            if (hasMod)
            {
                nodeGraph.removeModulationInlet (primaryId, paramKey);
            }
            else
            {
                auto n = nodeGraph.getNodeById (primaryId);
                if (n) n->addModulationInlet (paramKey);
            }
            rebuildInspector();
            repaint();
        };
        inspectorContainer.addAndMakeVisible (*row.btnModInlet);

        row.btnTapValue = std::make_unique<juce::TextButton> ("[TAP]");
        row.btnTapValue->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f766e));
        row.btnTapValue->onClick = [this, primaryId, paramKey] {
            auto n = nodeGraph.getNodeById (primaryId);
            if (n)
            {
                std::string snippet = "tap('" + n->getLabel() + "." + paramKey + "')";
                juce::SystemClipboard::copyTextToClipboard (snippet);
                formulaEditor.insertTextAtCaret (" + " + snippet);
            }
        };
        inspectorContainer.addAndMakeVisible (*row.btnTapValue);

        propertyRows.push_back (std::move (row));
    }

    auto methods = node->getExposedMethods();
    for (const auto& mName : methods)
    {
        auto btn = std::make_unique<juce::TextButton> ("[EXEC] " + mName);
        std::string m = mName;
        btn->onClick = [this, primaryId, m] {
            auto n = nodeGraph.getNodeById (primaryId);
            if (n)
            {
                if (m == "Load Audio File..." || m == "Load Sample File...")
                {
                    auto fc = std::make_shared<juce::FileChooser> ("Select Audio Sample File...",
                                                                   juce::File::getSpecialLocation (juce::File::userHomeDirectory),
                                                                   "*.wav;*.aif;*.aiff;*.mp3;*.flac");
                    fc->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, primaryId, fc] (const juce::FileChooser& chooser) {
                            auto result = chooser.getResult();
                            if (result.existsAsFile())
                            {
                                auto n = nodeGraph.getNodeById (primaryId);
                                auto sampler = std::dynamic_pointer_cast<SamplerNode> (n);
                                if (sampler) sampler->loadAudioFile (result);
                                juce::MessageManager::callAsync ([this] {
                                    rebuildInspector();
                                    repaint();
                                });
                            }
                        });
                }
                else
                {
                    n->invokeMethod (m);
                    juce::MessageManager::callAsync ([this] {
                        rebuildInspector();
                        repaint();
                    });
                }
            }
        };
        btn->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff8b5cf6));
        btn->setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        inspectorContainer.addAndMakeVisible (*btn);
        methodButtons.push_back (std::move (btn));
    }

    // Toggle 128px Delayline Dot Visualizer Button
    bool hasDL = node->isShowDelaylineEnabled();
    auto btnDL = std::make_unique<juce::TextButton> (hasDL ? "[TOGGLE DELAYLINE (128px)]: ON" : "[TOGGLE DELAYLINE (128px)]: OFF");
    btnDL->setColour (juce::TextButton::buttonColourId, hasDL ? juce::Colour (0xff0e7490) : juce::Colour (0xff374151));
    btnDL->onClick = [this, primaryId, hasDL] {
        auto n = nodeGraph.getNodeById (primaryId);
        if (n) {
            n->setShowDelaylineEnabled (!hasDL);
            juce::MessageManager::callAsync ([this] {
                rebuildInspector();
                repaint();
            });
        }
    };
    inspectorContainer.addAndMakeVisible (*btnDL);
    methodButtons.push_back (std::move (btnDL));

    // Toggle 128px Control Pipe Dot Visualizer Button
    bool hasPipe = node->isShowPipeEnabled();
    auto btnPipe = std::make_unique<juce::TextButton> (hasPipe ? "[TOGGLE PIPE (128px)]: ON" : "[TOGGLE PIPE (128px)]: OFF");
    btnPipe->setColour (juce::TextButton::buttonColourId, hasPipe ? juce::Colour (0xff7c3aed) : juce::Colour (0xff374151));
    btnPipe->onClick = [this, primaryId, hasPipe] {
        auto n = nodeGraph.getNodeById (primaryId);
        if (n) {
            n->setShowPipeEnabled (!hasPipe);
            juce::MessageManager::callAsync ([this] {
                rebuildInspector();
                repaint();
            });
        }
    };
    inspectorContainer.addAndMakeVisible (*btnPipe);
    methodButtons.push_back (std::move (btnPipe));

    // Populate INCOMING and OUTGOING connection sections
    connectionRows.clear();

    incomingSectionHeader = std::make_unique<juce::Label>();
    incomingSectionHeader->setText ("INCOMING CONNECTIONS (FROM):", juce::dontSendNotification);
    incomingSectionHeader->setFont (juce::FontOptions (12.0f, juce::Font::bold));
    incomingSectionHeader->setColour (juce::Label::textColourId, juce::Colour (0xff06b6d4));
    inspectorContainer.addAndMakeVisible (*incomingSectionHeader);

    outgoingSectionHeader = std::make_unique<juce::Label>();
    outgoingSectionHeader->setText ("OUTGOING CONNECTIONS (TO):", juce::dontSendNotification);
    outgoingSectionHeader->setFont (juce::FontOptions (12.0f, juce::Font::bold));
    outgoingSectionHeader->setColour (juce::Label::textColourId, juce::Colour (0xff8b5cf6));
    inspectorContainer.addAndMakeVisible (*outgoingSectionHeader);

    int incCount = 0;
    for (const auto& conn : nodeGraph.getConnections())
    {
        if (conn.destNodeId == primaryId)
        {
            incCount++;
            InspectorConnectionRow r;
            r.connectionId = conn.id;
            r.isIncoming = true;

            auto srcNode = nodeGraph.getNodeById (conn.sourceNodeId);
            std::string srcName = srcNode ? srcNode->getLabel() : "node";
            std::string inPortName = (conn.destInletIdx >= 0 && conn.destInletIdx < static_cast<int>(node->getInlets().size())) ? node->getInlets()[conn.destInletIdx].name : std::to_string(conn.destInletIdx);

            r.label = std::make_unique<juce::Label>();
            r.label->setText ("FROM [" + srcName + "] out" + std::to_string(conn.sourceOutletIdx) + " -> in" + std::to_string(conn.destInletIdx) + " (" + inPortName + ")", juce::dontSendNotification);
            r.label->setFont (juce::FontOptions (11.0f));
            inspectorContainer.addAndMakeVisible (*r.label);

            int cid = conn.id;
            r.btnRemoveWire = std::make_unique<juce::TextButton> ("[REMOVE]");
            r.btnRemoveWire->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff991b1b));
            r.btnRemoveWire->onClick = [this, cid] {
                nodeGraph.pushUndoState();
                nodeGraph.removeConnection (cid);
                juce::MessageManager::callAsync ([this] {
                    rebuildInspector();
                    repaint();
                });
            };
            inspectorContainer.addAndMakeVisible (*r.btnRemoveWire);

            connectionRows.push_back (std::move (r));
        }
    }

    if (incCount == 0)
    {
        InspectorConnectionRow r;
        r.label = std::make_unique<juce::Label>();
        r.label->setText (" (No incoming connections)", juce::dontSendNotification);
        r.label->setFont (juce::FontOptions (10.5f, juce::Font::italic));
        r.label->setColour (juce::Label::textColourId, juce::Colours::grey);
        inspectorContainer.addAndMakeVisible (*r.label);
        r.isIncoming = true;
        connectionRows.push_back (std::move (r));
    }

    int outCount = 0;
    for (const auto& conn : nodeGraph.getConnections())
    {
        if (conn.sourceNodeId == primaryId)
        {
            outCount++;
            InspectorConnectionRow r;
            r.connectionId = conn.id;
            r.isIncoming = false;

            auto destNode = nodeGraph.getNodeById (conn.destNodeId);
            std::string destName = destNode ? destNode->getLabel() : "node";
            std::string outPortName = (conn.sourceOutletIdx >= 0 && conn.sourceOutletIdx < static_cast<int>(node->getOutlets().size())) ? node->getOutlets()[conn.sourceOutletIdx].name : std::to_string(conn.sourceOutletIdx);

            r.label = std::make_unique<juce::Label>();
            r.label->setText ("TO [" + destName + "] out" + std::to_string(conn.sourceOutletIdx) + " (" + outPortName + ") -> in" + std::to_string(conn.destInletIdx), juce::dontSendNotification);
            r.label->setFont (juce::FontOptions (11.0f));
            inspectorContainer.addAndMakeVisible (*r.label);

            int cid = conn.id;
            r.btnRemoveWire = std::make_unique<juce::TextButton> ("[REMOVE]");
            r.btnRemoveWire->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff991b1b));
            r.btnRemoveWire->onClick = [this, cid] {
                nodeGraph.pushUndoState();
                nodeGraph.removeConnection (cid);
                juce::MessageManager::callAsync ([this] {
                    rebuildInspector();
                    repaint();
                });
            };
            inspectorContainer.addAndMakeVisible (*r.btnRemoveWire);

            connectionRows.push_back (std::move (r));
        }
    }

    if (outCount == 0)
    {
        InspectorConnectionRow r;
        r.label = std::make_unique<juce::Label>();
        r.label->setText (" (No outgoing connections)", juce::dontSendNotification);
        r.label->setFont (juce::FontOptions (10.5f, juce::Font::italic));
        r.label->setColour (juce::Label::textColourId, juce::Colours::grey);
        inspectorContainer.addAndMakeVisible (*r.label);
        r.isIncoming = false;
        connectionRows.push_back (std::move (r));
    }

    resized();
}

} // namespace time_dilation
