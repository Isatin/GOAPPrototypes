// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a test case with admissible heuristic and non-admissible one.
// There are four actions with one arithmetic operation and one action with two operations.
// The shortest solution is to apply the two-op action three times instead of the others four times.
// However, the non-admissible heuristic will miss the shortest solution while the admissible one  
// will find it.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
using VAR = ArithGOAP::SVariable;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CStateDefinition Definition;
    auto& X = *Definition.DefineNumber("X");
    auto& Y = *Definition.DefineNumber("Y");

    CState StartingState(Definition);
    StartingState.SetFact(X, 1);
    StartingState.SetFact(Y, 1);

    CState GoalState(Definition);
    GoalState.SetFact(X >= 10);
    GoalState.SetFact(Y >= 100);

    std::vector<CAction> Actions;
    {
        CAction& AddTwo = Actions.emplace_back("X+2", Definition);
        AddTwo.SetEffect(X += 2);
    }
    {
        CAction& AddFive = Actions.emplace_back("X+5", Definition);
        AddFive.SetEffect(X += 5);
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
        std::cout << "    NON-ADMISSIBLE HEURISTIC\n";
        RunGOAPs(StartingState, GoalState, Actions);
    }

    {
        std::cout << "    ADMISSIBLE HEURISTIC\n";
        X.SetDistanceWeight(0.01);
        Y.SetDistanceWeight(0.01);

        for (CAction& Action : Actions)
        {
            Action.SetBaseCost((float) StartingState.GetFactCount());
        }

        RunGOAPs(StartingState, GoalState, Actions);
    }

    return 0;
}
