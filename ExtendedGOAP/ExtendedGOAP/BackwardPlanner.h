// Copyright 2026 Isaac Hsu

#pragma once

#include <map>
#include <memory>

#include "BasePlanner.h"
#include "BitVector.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // This is an unfinished experimental planning algorithm, just for reference.
    // Unlike ExtendedGOAP::CRegressionPlanner, it is not a full regression method.
    // It looks for candidate effects backwards from a goal, but still evaluates world states forwards to check for completion.
    class CBackwardPlanner : public CBasePlanner
    {
        using Super = CBasePlanner;

        struct SNode : public SBaseNode
        {
            // ConstState and MutableState point to the same state except that the mutable counterpart of the starting state is null.
            const CState* ConstState = nullptr;
            std::unique_ptr<CState> MutableState;
            CBitVector DesiredFactBits;

        public:
            std::string ToString() const; // For debugging
        };

    public:
        using Super::Super;

    protected:
        bool DoPlan(std::vector<const CAction*>& oSteps, const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions, int MaxDepth) override;
        void PostPlan() override;

        // Create a search node for a given action from a given node if feasible.
        void Explore(std::multimap<float, int>& oOpenMap, int NodeIndex, const CAction& Action, const CState& StartingState, const CCondition& GoalCondition);
        // Apply the effects on the path from a given state.
        bool ApplyEffectsOnPath(CState& oState, int NodeIndex);
        // Return a description of listed nodes.
        std::string StringizeNodes(const std::multimap<float, int>& OpenMap, const CCondition& GoalCondition) const;

    protected:
        std::vector<SNode> mNodes;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
