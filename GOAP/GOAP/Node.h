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
        // Action taken from the parent node to this node
        const CAction* Action = nullptr;
        // ConstState and MutableState point to the same state except that the mutable counterparts of const-only states (the starting and goal states) are null.
        const CState* ConstState = nullptr;
        std::unique_ptr<CState> MutableState;

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

        std::string ToString() const;   // For debugging
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
