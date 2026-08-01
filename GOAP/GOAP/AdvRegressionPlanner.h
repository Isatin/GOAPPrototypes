// Copyright 2024 Isaac Hsu

#pragma once

#include <vector>

#include "RegressionPlanner.h"


namespace GOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CAdvRegressionPlanner : public CRegressionPlanner // Improved regressive GOAP with effect lookup tables
    {
        using Super = CRegressionPlanner;
    public:
        // Formulate a plan from the input if possible. Return true if successful.
        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////    
}
