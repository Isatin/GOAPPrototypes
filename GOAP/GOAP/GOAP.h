// Copyright 2024 Isaac Hsu

#pragma once

#include <vector>


namespace GOAP
{
    class CAction;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Interface functions of GOAP
    bool ForwardSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);
    bool RegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);
    bool LUTRegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);
}