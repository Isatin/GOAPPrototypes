// Copyright 2024 Isaac Hsu

#pragma once

#include <vector>

#include "ArithGOAP/Action.h"
#include "ArithGOAP/Fact.h"
#include "ArithGOAP/Notation.h"


// Utility functions to run all GOAP planners
void RunGOAPs(const ArithGOAP::CState& StartingState, const ArithGOAP::CState& GoalState, const std::vector<ArithGOAP::CAction>& Actions, int MaxDepth = 0);
void RunGOAPs(const ArithGOAP::CState& StartingState, const ArithGOAP::CState& GoalState, const std::vector<const ArithGOAP::CAction*>& Actions, int MaxDepth = 0);
