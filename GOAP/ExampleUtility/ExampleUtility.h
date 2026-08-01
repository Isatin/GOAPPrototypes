// Copyright 2024 Isaac Hsu

#pragma once

#include "GOAP/Action.h"


namespace GOAP
{
    enum EGOAPType : unsigned
    {
        ForwardGOAP         = 1 << 0,
        BackwardGOAP        = 1 << 1,
        RegressiveGOAP      = 1 << 2,
        AdvRegressiveGOAP   = 1 << 3,
    };

    constexpr unsigned DefaultGOAPs = ForwardGOAP | RegressiveGOAP | AdvRegressiveGOAP;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions to run GOAP planners
void RunGOAPs(const GOAP::CState& StartingState, const GOAP::CState& GoalState, const std::vector<GOAP::CAction>& Actions, int MaxDepth = 0, unsigned GOAPTypes = GOAP::DefaultGOAPs);
void RunGOAPs(const GOAP::CState& StartingState, const GOAP::CState& GoalState, const std::vector<const GOAP::CAction*>& Actions, int MaxDepth = 0, unsigned GOAPTypes = GOAP::DefaultGOAPs);
