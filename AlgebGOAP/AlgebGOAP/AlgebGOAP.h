// Copyright 2025 Isaac Hsu

#pragma once

#include <vector>


namespace AlgebGOAP
{
    class CAction;
    class CCondition;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    int GetMaxPreconditionRelations(const std::vector<CAction>& Actions);
    int GetMaxPreconditionRelations(const std::vector<const CAction*>& Actions);

    // Interface functions of GOAP
    bool ForwardSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition, const std::vector<const CAction*>& Actions, int MaxDepth, unsigned Flags);
    bool BackwardSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition, const std::vector<const CAction*>& Actions, int MaxDepth, unsigned Flags);
    bool RegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition, const std::vector<const CAction*>& Actions, int MaxDepth, unsigned Flags);
    bool AdvRegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition, const std::vector<const CAction*>& Actions, int MaxDepth, unsigned Flags);
}
