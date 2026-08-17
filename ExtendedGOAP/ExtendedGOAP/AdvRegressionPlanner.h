// Copyright 2026 Isaac Hsu

#pragma once

#include "RegressionPlanner.h"


namespace ExtendedGOAP
{
    /////////////////////////////////////////////////////////////////////////////////////////////////    
    class CAdvRegressionPlanner : public CRegressionPlanner // Regressive algebraic GOAP using simultaneous logical formulas
    {
        using Super = CRegressionPlanner;
    public:
        using Super::Super;
         
    protected:
        void PrePlan(const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions) override;
        void ConjoinCondition(CCondition& oLeft, const CCondition& Right) const override;
        std::string StringizeNode(const SNode& Node) const override;

        CState ReplaceFixedFacts(const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions);
    };
    /////////////////////////////////////////////////////////////////////////////////////////////////
}
