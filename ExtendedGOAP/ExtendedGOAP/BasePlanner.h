// Copyright 2026 Isaac Hsu

#pragma once

#include <functional>

#include "Common.h"
#include "Debug.h"


namespace ExtendedGOAP
{
    class CAction;
    class CCondition;
    class CFactDefinition;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CBasePlanner // Base class for GOAP planners
    {
    protected:
        struct SBaseNode
        {            
            const CAction* Action = nullptr;// Action taken from the parent node to this node
            int Parent = InvalidIndex;      // Index of the parent node
            int Depth = 0;                  // Depth of this node in the search tree
            float PreviousCost = 0.f;       // Cost of the previous actions
            float CurrentCost = 0.f;        // Cost of the current action
            float BaseHeuristicCost = 0.f;  // Heuristic cost based on property comparisons
            float ExtraHeuristicCost = 0.f; // Custom heuristic cost

        public:
            float GetActualCost() const     { return PreviousCost + CurrentCost; }
            float GetHeuristicCost() const  { return BaseHeuristicCost + ExtraHeuristicCost; }
            float GetTotalCost() const      { return GetActualCost() + GetHeuristicCost(); }
        };

    public:
        explicit CBasePlanner(unsigned Flags = 0) : mFlags(Flags) {}
        virtual ~CBasePlanner() {}

        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition, const std::vector<const CAction*>& Actions, int MaxDepth);

    protected:
        virtual void PrePlan(const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions);
        virtual bool DoPlan(std::vector<const CAction*>& oSteps, const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions, int MaxDepth) = 0;
        virtual void PostPlan() {}

        std::string StringizeSteps(const std::vector<const CAction*>& Steps) const;
        bool ShouldPrintExcessiveHeuristic() const { return mFlags & ShowExcessiveHeuristic; }
        void PrintExcessiveHeuristic(const CFactDefinition& Definition) const;

        // List the actions on the path from a given node to the root node.
        template <typename TNode>
        void BuildBackwardPlan(std::vector<const CAction*>& oSteps, const std::vector<TNode>& Nodes, int NodeIndex) const;
        // Return concatenated names of the actions on the path from a given node.
        template <typename TNode>
        std::string StringizeBackwardPath(const std::vector<TNode>& Nodes, int NodeIndex, bool Reached) const;
        // Return a fact-based heuristic cost from a given state to a desired condition.
        float GetBaseHeuristicCost(const CState& State, const CCondition& Condition, int NodeIndex);

    protected:
        unsigned mFlags = 0;
        SDebugInfo mDebugInfo;
        std::function<std::string (int NodeIndex)> mPathGetter;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TNode>
    void CBasePlanner::BuildBackwardPlan(std::vector<const CAction*>& oSteps, const std::vector<TNode>& Nodes, int NodeIndex) const
    {
        while (NodeIndex >= 0)
        {
            const TNode& CurrNode = Nodes[NodeIndex];
            if (CurrNode.Action)
            {
                oSteps.push_back(CurrNode.Action);
            }

            NodeIndex = CurrNode.Parent;
        }
    }

    template <typename TNode>
    std::string CBasePlanner::StringizeBackwardPath(const std::vector<TNode>& Nodes, int NodeIndex, bool Reached) const
    {
        std::string Path = "|";
        Path += (Reached || NodeIndex == 0) ? "" : "? ";

        bool Successive = false;
        while (NodeIndex >= 0)
        {
            const TNode& CurrNode = Nodes[NodeIndex];
            if (CurrNode.Action)
            {
                if (Successive)
                {
                    Path += ' ';
                }
                else
                {
                    Successive = true;
                }

                Path += CurrNode.Action->GetName();
            }

            NodeIndex = CurrNode.Parent;
        }

        Path += "|";
        return Path;
    }
}
