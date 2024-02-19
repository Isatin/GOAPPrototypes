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
        // Formulate a plan according to the input if possible. Otherwise, return false.
        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);

    protected:
        // Create a search node for the action and current node if the action is feasible
        void Explore(std::multimap<float, int>& oOpenMap, std::vector<SNode>& Nodes, int NodeIndex, const CAction& Action, const CState& GoalState);
        // List the actions on the path
        void BuildPlan(std::vector<const CAction*>& oSteps, const std::vector<SNode>& Nodes, int NodeIndex);
        // Return concatenated action names on the path
        std::string GetPathName(const std::vector<SNode>& Nodes, int NodeIndex);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
