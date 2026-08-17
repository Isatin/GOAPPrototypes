// Copyright 2025 Isaac Hsu

#pragma once

#include <vector>

#include "AlgebGOAP/Action.h"
#include "AlgebGOAP/AlgebGOAP.h"
#include "AlgebGOAP/Fact.h"
#include "AlgebGOAP/Notation.h"
#include "AlgebGOAP/State.h"


namespace AlgebGOAP
{
    enum EGOAPType : unsigned
    {
        ForwardGOAP         = ExtraGOAPFlag << 0,
        BackwardGOAP        = ExtraGOAPFlag << 1,
        RegressiveGOAP      = ExtraGOAPFlag << 2,
        AdvRegressiveGOAP   = ExtraGOAPFlag << 3,
    };

    constexpr unsigned DefaultGOAPFlags = ShowMethodName | ShowStartAndGoal | ShowAlteredActions | ShowSteps | ShowExcessiveHeuristic | ShowResult | ForwardGOAP /*| BackwardGOAP*/ | RegressiveGOAP | AdvRegressiveGOAP;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions to run GOAP planners
void RunGOAPs(const AlgebGOAP::CState& StartingState, AlgebGOAP::CCondition& GoalCondition, std::vector<AlgebGOAP::CAction>& Actions, int MaxDepth = 0, unsigned Flags = AlgebGOAP::DefaultGOAPFlags);
void RunGOAPs(const AlgebGOAP::CState& StartingState, AlgebGOAP::CCondition& GoalCondition, std::vector<AlgebGOAP::CAction>& Actions, size_t MaxDepth, unsigned Flags = AlgebGOAP::DefaultGOAPFlags);
void RunGOAPs(const AlgebGOAP::CState& StartingState, AlgebGOAP::CCondition& GoalCondition, const std::vector<const AlgebGOAP::CAction*>& Actions, int MaxDepth = 0, unsigned Flags = AlgebGOAP::DefaultGOAPFlags);
void RunGOAPs(const AlgebGOAP::CState& StartingState, AlgebGOAP::CCondition& GoalCondition, const std::vector<const AlgebGOAP::CAction*>& Actions, size_t MaxDepth, unsigned Flags = AlgebGOAP::DefaultGOAPFlags);
