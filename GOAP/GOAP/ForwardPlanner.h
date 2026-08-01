// Copyright 2024 Isaac Hsu

#pragma once

#include <map>
#include <string>
#include <vector>


namespace GOAP
{
    class CAction;
    class CState;
    struct SNode;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CForwardPlanner // Forward GOAP
    {
    public:
        // Formulate a plan from the input if possible. Return true if successful.
        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);

    protected:
        // Create a search node for a given action from a given node if feasible.
        void Explore(std::multimap<float, int>& oOpenMap, std::vector<SNode>& Nodes, int NodeIndex, const CAction& Action, const CState& GoalState);
        // List the actions on the path to a given node.
        void BuildPlan(std::vector<const CAction*>& oSteps, const std::vector<SNode>& Nodes, int NodeIndex);
        // Return concatenated names of the actions on the path to a given node.
        std::string StringizePath(const std::vector<SNode>& Nodes, int NodeIndex) const;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
