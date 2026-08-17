// Copyright 2025 Isaac Hsu

#pragma once

#include <map>
#include <memory>

#include "BasePlanner.h"


namespace AlgebGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CRegressionPlanner : public CBasePlanner // Regressive algebraic GOAP
    {
        using Super = CBasePlanner;
    protected:
        struct SNode : public SBaseNode
        {
            // ConstState and MutableState point to the same state except that the mutable counterpart of the goal condition is null.
            const CCondition* ConstCondition = nullptr;
            std::unique_ptr<CCondition> MutableCondition;
        };

    public:
        using Super::Super;

    protected:
        bool DoPlan(std::vector<const CAction*>& oSteps, const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions, int MaxDepth) override;
        void PostPlan() override;

        virtual void ConjoinCondition(CCondition& oLeft, const CCondition& Right) const;
        virtual std::string StringizeNode(const SNode& Node) const;

        // Create a search node for a given action from a given node if feasible.
        void Explore(std::multimap<float, int>& oOpenMap, int NodeIndex, const CAction& Action, const CState& StartingState);
        // Reverse a given action from a given postcondition and return the desired condition, if feasible.
        std::unique_ptr<CCondition> CheckPostcondition(const CCondition& Postcondition, const CAction& Action) const;
        // Return a description of listed nodes.
        std::string StringizeNodes(const std::multimap<float, int>& OpenMap, const CState& StartingState) const;

    protected:
        std::vector<SNode> mNodes;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
