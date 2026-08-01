// Copyright 2024 Isaac Hsu

#include <iostream>

#include "ArithGOAP/ArithGOAP.h"
#include "ExampleUtility.h"


using namespace ArithGOAP;
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

void RunGOAPs(const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
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
    bool Found1 = ForwardSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
    std::cout << StringizeGOAPResult(Found1, Steps) << std::endl;

    Steps.clear();
    bool Found2 = RegressiveSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
    std::cout << StringizeGOAPResult(Found2, Steps) << std::endl;

    Steps.clear();
    bool Found3 = AdvRegressiveSearch(Steps, StartingState, GoalState, Actions, MaxDepth);
    std::cout << StringizeGOAPResult(Found3, Steps) << std::endl;
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
