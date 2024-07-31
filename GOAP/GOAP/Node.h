// Copyright 2024 Isaac Hsu

#pragma once

#include <memory>
#include <string>


namespace GOAP
{
    class CAction;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SNode
    {
        const CAction* Action = nullptr;// Action taken from the parent node to this node
        // ConstState and MutableState are pointed to the same state except the mutable counterparts of const-only states (starting and goal states) are null.
        const CState* ConstState = nullptr;
        std::unique_ptr<CState> MutableState;

        int Parent = -1;                // Index of parent node
        int Depth = 0;                  // Depth of this node in search tree
        float PreviousCost = 0.f;       // Cost of previous actions
        float CurrentCost = 0.f;        // Cost of current action

        float BasicHeuristicCost = 0.f; // Heuristic cost based on facts of current and goal state
        float ExtraHeuristicCost = 0.f; // Custom heuristic cost

        float GetActualCost() const { return PreviousCost + CurrentCost; }
        float GetHeuristicCost() const { return BasicHeuristicCost + ExtraHeuristicCost; }
        float GetTotalCost() const { return GetActualCost() + GetHeuristicCost(); }

        std::string ToString() const;   // For debug
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
