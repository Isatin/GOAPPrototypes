// Copyright 2024 Isaac Hsu

#include <cmath>
#include <iostream>

#include "ExampleUtility.h"
#include "GOAP/GOAP.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string StringizeGOAPResult(bool Found, const std::vector<const CAction*>& Steps)
{
    std::string Return;

    if (Found)
    {
        Return += "SUCCEEDED |";

        bool Successive = false;

        for (int i = 0; i < Steps.size(); i++)
        {
            const CAction* Action = Steps[i];
            if (!Action)
            {
                break;
            }

            if (Successive)
            {
                Return += ' ';
            }
            else
            {
                Successive = true;
            }

            Return += Action->GetName();
        }

        Return += "|\n";
    }
    else
    {
        Return += "FAILED\n";
    }

    return Return;
}

void RunGOAPs(const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth, unsigned GOAPTypes)
{
    if (MaxDepth == 0)
    {
        const int MinDepth = 4; // GOAP plan length is usually no more than 4 (https://www.youtube.com/watch?v=gm7K68663rA&t=2712s). 
        MaxDepth = std::max({MinDepth, StartingState.GetPropertyCount(), static_cast<int>(Actions.size())});
    }

    for (const CAction* Action : Actions)
    {
        std::cout << Action->ToString() << "\n";
    }
    std::cout << "\n";

    std::vector<const CAction*> Steps;

    if (GOAPTypes & ForwardGOAP)
    {
        bool Found = ForwardSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
        std::cout << StringizeGOAPResult(Found, Steps) << "\n";
    }

    if (GOAPTypes & BackwardGOAP)
    {
        Steps.clear();
        bool Found = BackwardSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
        std::cout << StringizeGOAPResult(Found, Steps) << "\n";
    }

    if (GOAPTypes & RegressiveGOAP)
    {
        Steps.clear();
        bool Found = RegressiveSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
        std::cout << StringizeGOAPResult(Found, Steps) << "\n";
    }

    if (GOAPTypes & AdvRegressiveGOAP)
    {
        Steps.clear();
        bool Found = AdvRegressiveSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
        std::cout << StringizeGOAPResult(Found, Steps) << "\n";
    }
}

void RunGOAPs(const CState& StartingState, const CState& GoalState, const std::vector<CAction>& Actions, int MaxDepth, unsigned GOAPTypes)
{
    std::vector<const CAction*> ActionPtrs;
    ActionPtrs.reserve(Actions.size());
    for (const CAction& Action : Actions)
    {
        ActionPtrs.emplace_back(&Action);
    }

    RunGOAPs(StartingState, GoalState, ActionPtrs, MaxDepth, GOAPTypes);
}
