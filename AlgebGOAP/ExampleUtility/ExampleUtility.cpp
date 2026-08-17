// Copyright 2025 Isaac Hsu

#include <iostream>
#include <sstream>

#include "ExampleUtility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
void RunGOAPs(const AlgebGOAP::CState& StartingState, AlgebGOAP::CCondition& GoalCondition, std::vector<AlgebGOAP::CAction>& Actions, size_t MaxDepth, unsigned Flags)
{
    return RunGOAPs(StartingState, GoalCondition, Actions, static_cast<int>(MaxDepth), Flags);
}

void RunGOAPs(const AlgebGOAP::CState& StartingState, AlgebGOAP::CCondition& GoalCondition, std::vector<AlgebGOAP::CAction>& Actions, int MaxDepth, unsigned Flags)
{
    std::vector<const CAction*> ActionPtrs;
    ActionPtrs.reserve(Actions.size());
    for (const CAction& Action : Actions)
    {
        ActionPtrs.push_back(&Action);
    }

    RunGOAPs(StartingState, GoalCondition, ActionPtrs, MaxDepth, Flags);
}

void RunGOAPs(const AlgebGOAP::CState& StartingState, AlgebGOAP::CCondition& GoalCondition, const std::vector<const AlgebGOAP::CAction*>& Actions, size_t MaxDepth, unsigned Flags)
{
    return RunGOAPs(StartingState, GoalCondition, Actions, static_cast<int>(MaxDepth), Flags);
}

void RunGOAPs(const AlgebGOAP::CState& StartingState, AlgebGOAP::CCondition& GoalCondition, const std::vector<const AlgebGOAP::CAction*>& Actions, int MaxDepth, unsigned Flags)
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
