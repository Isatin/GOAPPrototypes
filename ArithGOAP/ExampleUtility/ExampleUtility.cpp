// Copyright 2024 Isaac Hsu

#include <iostream>

#include "ArithGOAP/ArithGOAP.h"
#include "ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
void RunGOAPs(const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    if (MaxDepth == 0)
    {
        const int MinDepth = 4; // GOAP plan length is usually no more than 4. (https://www.youtube.com/watch?v=gm7K68663rA&t=2712s)
        MaxDepth = std::max<int>({MinDepth, StartingState.GetFactCount(), (int) Actions.size()});
    }

    for (const CAction* Action : Actions)
    {
        std::cout << Action->ToString() << "\n";
    }

    std::cout << "\n";

    std::vector<const CAction*> Steps;
    bool Found1 = ForwardSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
    std::cout << (Found1 ? "SUCCEEDED" : "FAILED") << "\n\n";
    bool Found2 = RegressiveSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
    std::cout << (Found2 ? "SUCCEEDED" : "FAILED") << "\n\n";
    bool Found3 = LUTRegressiveSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
    std::cout << (Found3 ? "SUCCEEDED" : "FAILED") << "\n\n";
}

void RunGOAPs(const CState& StartingState, const CState& GoalState, const std::vector<CAction>& Actions, int MaxDepth)
{
    std::vector<const CAction*> ActionPtrs;
    ActionPtrs.reserve(Actions.size());
    for (const CAction& Action : Actions)
    {
        ActionPtrs.push_back(&Action);
    }

    RunGOAPs(StartingState, GoalState, ActionPtrs, MaxDepth);
}
