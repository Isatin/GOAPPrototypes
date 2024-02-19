// Copyright 2024 Isaac Hsu

#pragma once

#include <map>
#include <string>
#include <vector>


namespace ArithGOAP
{
    class CAction;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CRegressivePlanner // Regressive GOAP
    {
    public:
        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);

    protected:
        struct SNode
        {
            const CAction* Action = nullptr;// Action taken from the parent node to this node
            // ConstState and MutableState are pointed to the same state except the mutable counterparts of const-only states (starting and goal states) are null.
            const CState* ConstState = nullptr;
            std::unique_ptr<CState> MutableState;

            int Parent = -1;                // Index of parent node
            int Depth = 0;                  // Depth of this node in search tree
            float CurrentCost = 0.f;        // Cost of current action
            float PreviousCost = 0.f;       // Cost of previous actions
            float BasicHeuristicCost = 0.f; // Heuristic cost based on facts of current and goal state
            float ExtraHeuristicCost = 0.f; // Custom heuristic cost

            float GetActualCost() const { return CurrentCost + PreviousCost; }
            float GetHeuristicCost() const { return BasicHeuristicCost + ExtraHeuristicCost; }
            float GetTotalCost() const { return GetActualCost() + GetHeuristicCost(); }

            std::string ToString() const;   // For debug
        };

        // Create a search node for the action and current node if the action is feasible
        void Explore(std::multimap<float, int>& oOpenMap, std::vector<SNode>& Nodes, int NodeIndex, const CAction& Action, const CState& StartingState);
        // List the actions on the path
        void BuildPlan(std::vector<const CAction*>& oSteps, const std::vector<SNode>& Nodes, int NodeIndex);
        // Return concatenated action names on the path
        std::string GetPathName(const std::vector<SNode>& Nodes, int NodeIndex);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
