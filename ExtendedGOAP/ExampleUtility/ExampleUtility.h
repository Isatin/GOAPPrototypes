// Copyright 2026 Isaac Hsu

#pragma once

#include <vector>

#include "ExtendedGOAP/Action.h"
#include "ExtendedGOAP/ExtendedGOAP.h"
#include "ExtendedGOAP/Fact.h"
#include "ExtendedGOAP/Matrix.h"
#include "ExtendedGOAP/MatrixExpression.h"
#include "ExtendedGOAP/Notation.h"
#include "ExtendedGOAP/Set.h"
#include "ExtendedGOAP/SetExpression.h"
#include "ExtendedGOAP/State.h"


namespace ExtendedGOAP
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
void RunGOAPs(const ExtendedGOAP::CState& StartingState, ExtendedGOAP::CCondition& GoalCondition, std::vector<ExtendedGOAP::CAction>& Actions, int MaxDepth = 0, unsigned Flags = ExtendedGOAP::DefaultGOAPFlags);
void RunGOAPs(const ExtendedGOAP::CState& StartingState, ExtendedGOAP::CCondition& GoalCondition, std::vector<ExtendedGOAP::CAction>& Actions, size_t MaxDepth, unsigned Flags = ExtendedGOAP::DefaultGOAPFlags);
void RunGOAPs(const ExtendedGOAP::CState& StartingState, ExtendedGOAP::CCondition& GoalCondition, const std::vector<const ExtendedGOAP::CAction*>& Actions, int MaxDepth = 0, unsigned Flags = ExtendedGOAP::DefaultGOAPFlags);
void RunGOAPs(const ExtendedGOAP::CState& StartingState, ExtendedGOAP::CCondition& GoalCondition, const std::vector<const ExtendedGOAP::CAction*>& Actions, size_t MaxDepth, unsigned Flags = ExtendedGOAP::DefaultGOAPFlags);
