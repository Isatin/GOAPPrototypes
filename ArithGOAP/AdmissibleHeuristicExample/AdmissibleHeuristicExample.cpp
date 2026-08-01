// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a test case with an inadmissible heuristic and another with an admissible one.
// There are four actions with one arithmetic operation and an action with two operations.
// The simplest solution is to apply the two-op action three times.
// The inadmissible heuristic misses it, while the admissible one finds the shortest path.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& X = *Definition.DefineNumber("X");
    auto& Y = *Definition.DefineNumber("Y");

    CState StartingState(Definition);
    StartingState.SetProperty(X, 1);
    StartingState.SetProperty(Y, 1);

    CState GoalState(Definition);
    GoalState.SetProperty(X >= 10);
    GoalState.SetProperty(Y >= 100);

    std::vector<CAction> Actions;
    {
        CAction& PlusTwo = Actions.emplace_back("X+2", Definition);
        PlusTwo.SetEffect(X += 2);
    }
    {
        CAction& PlusFive = Actions.emplace_back("X+5", Definition);
        PlusFive.SetEffect(X += 5);
    }
    {
        CAction& Double = Actions.emplace_back("Y*2", Definition);
        Double.SetEffect(Y *= 2);
    }
    {
        CAction& TenTimes = Actions.emplace_back("Y*10", Definition);
        TenTimes.SetEffect(Y *= 10);
    }
    {
        CAction& TwoOps = Actions.emplace_back("X+3&Y*5", Definition);
        TwoOps.SetEffect(X += 3);
        TwoOps.SetEffect(Y *= 5);
    }

    {
        std::cout << "    INADMISSIBLE HEURISTIC\n";
        RunGOAPs(StartingState, GoalState, Actions);
    }

    {
        std::cout << "    ADMISSIBLE HEURISTIC\n";
        X.SetGapWeight(0.01);
        Y.SetGapWeight(0.01);

        const float BaseCost = static_cast<float>(StartingState.CountProperties() * Definition.GetBaseRelationCost());
        for (CAction& Action : Actions)
        {
            Action.SetBaseCost(BaseCost);
        }

        RunGOAPs(StartingState, GoalState, Actions);
    }

    return 0;
}
