#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Midi2Packet.h"

namespace time_dilation
{

enum class PortType
{
    Audio,
    Midi2,
    GammaTimeDilation
};

struct DataflowPort
{
    std::string id;
    std::string name;
    PortType type;
    bool isOutput = false;
};

struct Connection
{
    std::string fromNodeId;
    std::string fromPortId;
    std::string toNodeId;
    std::string toPortId;
};

class DataflowNode
{
public:
    DataflowNode (const std::string& nodeId, const std::string& nodeName)
        : id (nodeId), name (nodeName) {}
    virtual ~DataflowNode() = default;

    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }

    void addPort (const std::string& portId, const std::string& portName, PortType type, bool isOutput)
    {
        ports.push_back ({ portId, portName, type, isOutput });
    }

    const std::vector<DataflowPort>& getPorts() const { return ports; }

    virtual void process (juce::AudioBuffer<float>& audioBuffer,
                          std::vector<Midi2Packet>& midiPackets,
                          float& gammaStream) = 0;

private:
    std::string id;
    std::string name;
    std::vector<DataflowPort> ports;

    JUCE_DECLARE_NON_COPYABLE (DataflowNode)
};

class DataflowGraph
{
public:
    DataflowGraph() = default;
    ~DataflowGraph() = default;

    DataflowGraph (DataflowGraph&&) = default;
    DataflowGraph& operator= (DataflowGraph&&) = default;

    void addNode (std::unique_ptr<DataflowNode> node)
    {
        if (node != nullptr)
        {
            auto id = node->getId();
            nodes[id] = std::move (node);
        }
    }

    void removeNode (const std::string& nodeId)
    {
        nodes.erase (nodeId);
        std::erase_if (connections, [&nodeId] (const Connection& c) {
            return c.fromNodeId == nodeId || c.toNodeId == nodeId;
        });
    }

    void connectPorts (const std::string& fromNode, const std::string& fromPort,
                       const std::string& toNode, const std::string& toPort)
    {
        connections.push_back ({ fromNode, fromPort, toNode, toPort });
    }

    void disconnectPorts (const std::string& fromNode, const std::string& fromPort,
                          const std::string& toNode, const std::string& toPort)
    {
        std::erase_if (connections, [&] (const Connection& c) {
            return c.fromNodeId == fromNode && c.fromPortId == fromPort &&
                   c.toNodeId == toNode && c.toPortId == toPort;
        });
    }

    const std::vector<Connection>& getConnections() const { return connections; }

    void processGraph (juce::AudioBuffer<float>& masterBuffer,
                       std::vector<Midi2Packet>& masterMidi,
                       float masterGamma)
    {
        for (auto& [id, node] : nodes)
        {
            if (node != nullptr)
            {
                float nodeGamma = masterGamma;
                node->process (masterBuffer, masterMidi, nodeGamma);
            }
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<DataflowNode>> nodes;
    std::vector<Connection> connections;

    JUCE_DECLARE_NON_COPYABLE (DataflowGraph)
};

} // namespace time_dilation
