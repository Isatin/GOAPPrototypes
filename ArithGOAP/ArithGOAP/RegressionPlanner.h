// Copyright 2024 Isaac Hsu

#pragma once

#include <map>
#include <vector>

#include "Node.h"


namespace ArithGOAP
{
    class CAction;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CRegressionPlanner // Regressive arithmetic GOAP
    {
    public:
        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);

    protected:
        // Create a search node for a given action from a given node if feasible.
        void Explore(std::multimap<float, int>& oOpenMap, std::vector<SNode>& Nodes, int NodeIndex, const CAction& Action, const CState& StartingState);
        // List the actions on the path from a given node.
        void BuildPlan(std::vector<const CAction*>& oSteps, const std::vector<SNode>& Nodes, int NodeIndex);
        // Return concatenated names of the actions on the path from a given node.
        std::string StringizePath(const std::vector<SNode>& Nodes, int NodeIndex) const;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
