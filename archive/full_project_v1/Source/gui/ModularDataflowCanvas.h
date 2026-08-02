#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/TimeDilationEngine.h"

namespace time_dilation
{

struct SocketPos
{
    std::string nodeId;
    std::string portId;
    juce::Point<float> pos;
    juce::Colour color;
    bool isOutput;
    PortType type;
};

struct CableLink
{
    std::string fromNodeId;
    std::string fromPortId;
    std::string toNodeId;
    std::string toPortId;
    juce::Colour color;
};

class ModularDataflowCanvas : public juce::Component
{
public:
    explicit ModularDataflowCanvas (TimeDilationEngine& engine);
    ~ModularDataflowCanvas() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    void addSynthNode();
    void addMixerBusNode();

private:
    TimeDilationEngine& engine;

    std::vector<SocketPos> activeSockets;
    std::vector<CableLink> persistentCables;

    bool isDraggingCable = false;
    SocketPos dragStartSocket;
    juce::Point<float> cableCurrentPos;

    int synthNodeCount = 1;
    int mixerBusCount = 1;

    void drawNodeCard (juce::Graphics& g, const std::string& type, const std::string& name, juce::Colour color, float x, float y, float width, float height);
    void drawPatchCables (juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModularDataflowCanvas)
};

} // namespace time_dilation
