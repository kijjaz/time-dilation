#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_osc/juce_osc.h>
#include "RelativisticNodeGraph.h"

namespace time_dilation
{

/**
    @class RelativisticOscServer
    @brief Real-time OSC (Open Sound Control) Remote Control & LLM Live-Patching Server.
    Listens on UDP Port 9000 (configurable) for live node creation, cable connection,
    parameter modulation, and graph state queries.
*/
class RelativisticOscServer : public juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>
{
public:
    RelativisticOscServer (RelativisticNodeGraph& graph);
    ~RelativisticOscServer() override;

    bool startServer (int port = 9000);
    void stopServer();
    bool isRunning() const { return listeningPort > 0; }
    int getPort() const { return listeningPort; }

    void oscMessageReceived (const juce::OSCMessage& message) override;

    // Callback fired on GUI main thread when graph topology is modified via OSC
    std::function<void()> onGraphModified;
    std::function<void(const std::string&, bool isWarning)> onOscLogMessage;
    std::function<void(const std::string& filePath, float durationSec)> onExportWavRequested;
    std::function<void(const std::string& filePath)> onExportPngRequested;

private:
    RelativisticNodeGraph& nodeGraph;
    juce::OSCReceiver oscReceiver;
    int listeningPort = 0;

    int resolveNodeId (const juce::OSCArgument& arg) const;
};

} // namespace time_dilation
