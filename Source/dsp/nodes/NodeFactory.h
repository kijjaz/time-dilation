#pragma once

#include "../RelativisticNodeGraph.h"
#include "AudioNodeObjects.h"
#include "RelativisticTimeNodeObjects.h"

namespace time_dilation
{

// Forward declaration of NodeFactory
class NodeFactory
{
public:
    static std::shared_ptr<RelativisticNode> createNodeByType (const std::string& type, int id);
    static std::vector<std::string> getAvailableNodeTypes();
};

} // namespace time_dilation
