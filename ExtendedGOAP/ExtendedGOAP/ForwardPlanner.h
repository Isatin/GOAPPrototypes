// Copyright 2026 Isaac Hsu

#pragma once

#include <map>
#include <memory>

#include "BasePlanner.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CForwardPlanner : public CBasePlanner // Forward algebraic GOAP
    {
        using Super = CBasePlanner;

        struct SNode : public SBaseNode
        {
            // ConstState and MutableState point to the same state except that the mutable counterpart of the starting state is null.
            const CState* ConstState = nullptr;
            std::unique_ptr<CState> MutableState;

        public:
            std::string ToString() const; // For debugging
        };

    public:
        using Super::Super;

    protected:
        bool DoPlan(std::vector<const CAction*>& oSteps, const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions, int MaxDepth) override;
        void PostPlan() override;

        // Create a search node for a given action from a given node if feasible.
        void Explore(std::multimap<float, int>& oOpenMap, int NodeIndex, const CAction& Action, const CCondition& GoalCondition);
        // List the actions on the path to a given node.
        void BuildForwardPlan(std::vector<const CAction*>& oSteps, int NodeIndex) const;
        // Return concatenated names of the actions on the path to a given node.
        std::string StringizeForwardPath(int NodeIndex) const;
        // Return a description of listed nodes.
        std::string StringizeNodes(const std::multimap<float, int>& OpenMap, const CCondition& GoalCondition) const;

    protected:
        std::vector<SNode> mNodes;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
