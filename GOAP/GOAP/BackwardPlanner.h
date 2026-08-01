// Copyright 2024 Isaac Hsu

#pragma once

#include <vector>


namespace GOAP
{
    class CAction;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Modified version of the original regressive GOAP with an extra check to avoid sequences of conflicting actions, namely SNode::CheckPostcondition.
    class CBackwardPlanner
    {
        struct SNode
        {
            const CAction* Action = nullptr;        // Action taken from the parent node to this node
            std::unique_ptr<CState> DesiredState;   // Desired world state of this node
            std::unique_ptr<CState> CurrentState;   // Current world state of this node

            int Parent = -1;                // Index of the parent node
            int Depth = 0;                  // Depth of this node in the search tree
            float PreviousCost = 0.f;       // Cost of the previous actions
            float CurrentCost = 0.f;        // Cost of the current action

            float BaseHeuristicCost = 0.f;  // Heuristic cost based on property comparisons
            float ExtraHeuristicCost = 0.f; // Custom heuristic cost

        public:
            float GetActualCost() const     { return PreviousCost + CurrentCost; }
            float GetHeuristicCost() const  { return BaseHeuristicCost + ExtraHeuristicCost; }
            float GetTotalCost() const      { return GetActualCost() + GetHeuristicCost(); }

            bool IsSatisfied() const;
            bool CheckPostcondition(const CAction& Action) const;
            std::string ToString() const; // For debugging
        };

    public:
        // Formulate a plan from the input if possible. Return true if successful.
        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);

    private:
        // Create a search node for a given action from a given node if feasible.
        void Explore(std::multimap<float, int>& oOpenMap, std::vector<SNode>& Nodes, int NodeIndex, const CAction& Action, const CState& StartingState);
        // List the actions on the path from a given node.
        void BuildPlan(std::vector<const CAction*>& oSteps, const std::vector<SNode>& Nodes, int NodeIndex);
        // Return concatenated names of the actions on the path from a given node.
        std::string StringizePath(const std::vector<SNode>& Nodes, int NodeIndex) const;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
