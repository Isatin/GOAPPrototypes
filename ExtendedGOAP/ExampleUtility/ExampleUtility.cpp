// Copyright 2026 Isaac Hsu

#include <iostream>
#include <sstream>

#include "ExampleUtility.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
void RunGOAPs(const ExtendedGOAP::CState& StartingState, ExtendedGOAP::CCondition& GoalCondition, std::vector<ExtendedGOAP::CAction>& Actions, size_t MaxDepth, unsigned Flags)
{
    return RunGOAPs(StartingState, GoalCondition, Actions, static_cast<int>(MaxDepth), Flags);
}

void RunGOAPs(const ExtendedGOAP::CState& StartingState, ExtendedGOAP::CCondition& GoalCondition, std::vector<ExtendedGOAP::CAction>& Actions, int MaxDepth, unsigned Flags)
{
    std::vector<const CAction*> ActionPtrs;
    ActionPtrs.reserve(Actions.size());
    for (const CAction& Action : Actions)
    {
        ActionPtrs.push_back(&Action);
    }

    RunGOAPs(StartingState, GoalCondition, ActionPtrs, MaxDepth, Flags);
}

void RunGOAPs(const ExtendedGOAP::CState& StartingState, ExtendedGOAP::CCondition& GoalCondition, const std::vector<const ExtendedGOAP::CAction*>& Actions, size_t MaxDepth, unsigned Flags)
{
    return RunGOAPs(StartingState, GoalCondition, Actions, static_cast<int>(MaxDepth), Flags);
}

void RunGOAPs(const ExtendedGOAP::CState& StartingState, ExtendedGOAP::CCondition& GoalCondition, const std::vector<const ExtendedGOAP::CAction*>& Actions, int MaxDepth, unsigned Flags)
{
    if (MaxDepth == 0)
    {
        const int MinDepth = 4; // GOAP plan length is usually no more than 4 (https://www.youtube.com/watch?v=gm7K68663rA&t=2712s). 
        MaxDepth = std::max({MinDepth, StartingState.CountProperties(), static_cast<int>(Actions.size())});
    }

    for (const CAction* Action : Actions)
    {
        std::cout << Action->ToString() << std::endl;
    }
    std::cout << std::endl;

    std::vector<const CAction*> Steps;

    if (Flags & ForwardGOAP)
    {
        ForwardSearch(Steps, StartingState, GoalCondition, Actions, MaxDepth, Flags);
        std::cout << std::endl;
    }

    if (Flags & BackwardGOAP)
    {
        Steps.clear();
        BackwardSearch(Steps, StartingState, GoalCondition, Actions, MaxDepth, Flags);
        std::cout << std::endl;
    }

    if (Flags & RegressiveGOAP)
    {
        Steps.clear();
        RegressiveSearch(Steps, StartingState, GoalCondition, Actions, MaxDepth, Flags);
        std::cout << std::endl;
    }

    if (Flags & AdvRegressiveGOAP)
    {
        Steps.clear();
        AdvRegressiveSearch(Steps, StartingState, GoalCondition, Actions, MaxDepth, Flags);
        std::cout << std::endl;
    }
}
