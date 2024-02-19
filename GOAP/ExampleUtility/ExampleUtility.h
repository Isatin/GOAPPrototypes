// Copyright 2024 Isaac Hsu

#pragma once

#include "GOAP/Action.h"


// Utility functions to run all GOAP planners
void RunGOAPs(const GOAP::CState& StartingState, const GOAP::CState& GoalState, const std::vector<GOAP::CAction>& Actions, int MaxDepth = 0);
void RunGOAPs(const GOAP::CState& StartingState, const GOAP::CState& GoalState, const std::vector<const GOAP::CAction*>& Actions, int MaxDepth = 0);
