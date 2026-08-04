#include "RelativisticOscServer.h"

namespace time_dilation
{

RelativisticOscServer::RelativisticOscServer (RelativisticNodeGraph& graph)
    : nodeGraph (graph)
{
}

RelativisticOscServer::~RelativisticOscServer()
{
    stopServer();
}

bool RelativisticOscServer::startServer (int port)
{
    stopServer();

    if (oscReceiver.connect (port))
    {
        listeningPort = port;
        oscReceiver.addListener (this);

        if (onOscLogMessage)
            onOscLogMessage ("OSC Server Listening on UDP Port " + std::to_string (port), false);

        return true;
    }

    listeningPort = 0;
    if (onOscLogMessage)
        onOscLogMessage ("Failed to bind OSC Server to UDP Port " + std::to_string (port), true);

    return false;
}

void RelativisticOscServer::stopServer()
{
    if (listeningPort > 0)
    {
        oscReceiver.removeListener (this);
        oscReceiver.disconnect();
        listeningPort = 0;
    }
}

int RelativisticOscServer::resolveNodeId (const juce::OSCArgument& arg) const
{
    if (arg.isString())
    {
        std::string label = arg.getString().toStdString();
        auto n = nodeGraph.getNodeByLabel (label);
        if (n) return n->getId();
        // Fallback: check integer string
        try {
            return std::stoi (label);
        } catch (...) {}
    }
    else if (arg.isInt32())
    {
        return arg.getInt32();
    }
    else if (arg.isFloat32())
    {
        return static_cast<int>(arg.getFloat32());
    }
    return -1;
}

void RelativisticOscServer::oscMessageReceived (const juce::OSCMessage& message)
{
    juce::String address = message.getAddressPattern().toString();

    // 1. /node/add <type> [x] [y]
    if (address == "/node/add")
    {
        if (message.size() >= 1 && message[0].isString())
        {
            std::string typeName = message[0].getString().toStdString();
            float x = (message.size() >= 2 && (message[1].isFloat32() || message[1].isInt32())) ? (message[1].isFloat32() ? message[1].getFloat32() : static_cast<float>(message[1].getInt32())) : 420.0f;
            float y = (message.size() >= 3 && (message[2].isFloat32() || message[2].isInt32())) ? (message[2].isFloat32() ? message[2].getFloat32() : static_cast<float>(message[2].getInt32())) : 300.0f;

            juce::MessageManager::callAsync ([this, typeName, x, y] {
                int newId = nodeGraph.addNode (typeName, x, y);
                if (onOscLogMessage)
                    onOscLogMessage ("OSC: Created [" + typeName + "] (ID " + std::to_string (newId) + ")", false);
                if (onGraphModified)
                    onGraphModified();
            });
        }
    }
    // 2. /node/delete <nodeId_or_label>
    else if (address == "/node/delete")
    {
        if (message.size() >= 1)
        {
            int nodeId = resolveNodeId (message[0]);
            if (nodeId >= 0)
            {
                juce::MessageManager::callAsync ([this, nodeId] {
                    nodeGraph.removeNode (nodeId);
                    if (onOscLogMessage)
                        onOscLogMessage ("OSC: Removed Node ID " + std::to_string (nodeId), false);
                    if (onGraphModified)
                        onGraphModified();
                });
            }
        }
    }
    // 3. /node/param <nodeId_or_label> <paramKey> <value>
    else if (address == "/node/param")
    {
        if (message.size() >= 3 && message[1].isString())
        {
            int nodeId = resolveNodeId (message[0]);
            std::string paramKey = message[1].getString().toStdString();
            float val = message[2].isFloat32() ? message[2].getFloat32() : (message[2].isInt32() ? static_cast<float>(message[2].getInt32()) : 0.0f);

            if (nodeId >= 0)
            {
                juce::MessageManager::callAsync ([this, nodeId, paramKey, val] {
                    auto n = nodeGraph.getNodeById (nodeId);
                    if (n)
                    {
                        n->setParameter (paramKey, val);
                        if (onOscLogMessage)
                            onOscLogMessage ("OSC: Set " + n->getLabel() + "." + paramKey + " = " + std::to_string (val), false);
                        if (onGraphModified)
                            onGraphModified();
                    }
                });
            }
        }
    }
    // 4. /node/connect <srcNode> <srcOutlet> <dstNode> <dstInlet>
    else if (address == "/node/connect")
    {
        if (message.size() >= 4)
        {
            int srcId = resolveNodeId (message[0]);
            int srcOutlet = message[1].isInt32() ? message[1].getInt32() : static_cast<int>(message[1].getFloat32());
            int dstId = resolveNodeId (message[2]);
            int dstInlet = message[3].isInt32() ? message[3].getInt32() : static_cast<int>(message[3].getFloat32());

            if (srcId >= 0 && dstId >= 0)
            {
                juce::MessageManager::callAsync ([this, srcId, srcOutlet, dstId, dstInlet] {
                    bool ok = nodeGraph.addConnection (srcId, srcOutlet, dstId, dstInlet);
                    if (onOscLogMessage)
                        onOscLogMessage ("OSC: Cable Connection " + std::to_string (srcId) + ":" + std::to_string (srcOutlet) + " -> " + std::to_string (dstId) + ":" + std::to_string (dstInlet) + (ok ? " [OK]" : " [FAILED]"), !ok);
                    if (onGraphModified)
                        onGraphModified();
                });
            }
        }
    }
    // 5. /node/disconnect <srcNode> <srcOutlet> <dstNode> <dstInlet>
    else if (address == "/node/disconnect")
    {
        if (message.size() >= 4)
        {
            int srcId = resolveNodeId (message[0]);
            int srcOutlet = message[1].isInt32() ? message[1].getInt32() : static_cast<int>(message[1].getFloat32());
            int dstId = resolveNodeId (message[2]);
            int dstInlet = message[3].isInt32() ? message[3].getInt32() : static_cast<int>(message[3].getFloat32());

            if (srcId >= 0 && dstId >= 0)
            {
                juce::MessageManager::callAsync ([this, srcId, srcOutlet, dstId, dstInlet] {
                    nodeGraph.removeConnection (srcId, srcOutlet, dstId, dstInlet);
                    if (onOscLogMessage)
                        onOscLogMessage ("OSC: Cable Disconnected " + std::to_string (srcId) + ":" + std::to_string (srcOutlet) + " -> " + std::to_string (dstId) + ":" + std::to_string (dstInlet), false);
                    if (onGraphModified)
                        onGraphModified();
                });
            }
        }
    }
    // 6. /transport/power <1_or_0>
    else if (address == "/transport/power")
    {
        if (message.size() >= 1)
        {
            bool enable = message[0].isInt32() ? (message[0].getInt32() > 0) : (message[0].getFloat32() > 0.5f);
            juce::MessageManager::callAsync ([this, enable] {
                nodeGraph.setAudioEngineEnabled (enable);
                if (onOscLogMessage)
                    onOscLogMessage ("OSC: Audio Engine " + std::string (enable ? "ON (ACTIVE)" : "OFF (SAFE)"), false);
                if (onGraphModified)
                    onGraphModified();
            });
        }
    }
    // 7. /global/set <varName> <value>
    else if (address == "/global/set")
    {
        if (message.size() >= 2 && message[0].isString())
        {
            std::string varName = message[0].getString().toStdString();
            double val = message[1].isFloat32() ? static_cast<double>(message[1].getFloat32()) : (message[1].isInt32() ? static_cast<double>(message[1].getInt32()) : 0.0);

            juce::MessageManager::callAsync ([this, varName, val] {
                nodeGraph.setGlobalVariable (varName, val);
                if (onOscLogMessage)
                    onOscLogMessage ("OSC: Set Global Variable $" + varName + " = " + std::to_string (val), false);
                if (onGraphModified)
                    onGraphModified();
            });
        }
    }
    // 8. /export/wav <filePath> [durationSec]
    else if (address == "/export/wav")
    {
        if (message.size() >= 1 && message[0].isString())
        {
            std::string path = message[0].getString().toStdString();
            float durSec = (message.size() >= 2 && message[1].isFloat32()) ? message[1].getFloat32() : 5.0f;

            juce::MessageManager::callAsync ([this, path, durSec] {
                if (onExportWavRequested)
                    onExportWavRequested (path, durSec);
                if (onOscLogMessage)
                    onOscLogMessage ("OSC: Export WAV requested -> " + path, false);
            });
        }
    }
    // 9. /export/png <filePath>
    else if (address == "/export/png")
    {
        if (message.size() >= 1 && message[0].isString())
        {
            std::string path = message[0].getString().toStdString();

            juce::MessageManager::callAsync ([this, path] {
                if (onExportPngRequested)
                    onExportPngRequested (path);
                if (onOscLogMessage)
                    onOscLogMessage ("OSC: Export PNG requested -> " + path, false);
            });
        }
    }
}

} // namespace time_dilation
