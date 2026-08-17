// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a test case with an inadmissible heuristic and another with an admissible one.
// There are four actions with one arithmetic operation and an action with two operations.
// The simplest solution is to apply the two-op action three times.
// The inadmissible heuristic misses it, while the admissible one finds the shortest path.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& X = *Definition.DefineNumber("X");
    auto& Y = *Definition.DefineNumber("Y");

    CState StartingState(Definition);
    StartingState.SetProperty(X, 1);
    StartingState.SetProperty(Y, 1);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(X > 9 && Y >= 100);

    std::vector<CAction> Actions;
    {
        CAction& PlusTwo = Actions.emplace_back("X+=2", Definition);
        PlusTwo.SetPrecondition(X >= 0);
        PlusTwo.AddEffect(X += 2);
    }
    {
        CAction& PlusFive = Actions.emplace_back("X+=5", Definition);
        PlusFive.SetPrecondition(X >= 0);
        PlusFive.AddEffect(X += 5);
    }
    {
        CAction& Double = Actions.emplace_back("Y*=2", Definition);
        Double.SetPrecondition(Y >= 0);
        Double.AddEffect(Y *= 2);
    }
    {
        CAction& TenTimes = Actions.emplace_back("Y*=10", Definition);
        TenTimes.SetPrecondition(Y >= 0);
        TenTimes.AddEffect(Y *= 10);
    }
    {
        CAction& TwoOps = Actions.emplace_back("X+=3&Y*=5", Definition);
        TwoOps.SetPrecondition(X >= 0 && Y >= 0);
        TwoOps.AddEffect(X += 3);
        TwoOps.AddEffect(Y *= 5);
    }

    {
        std::cout << "    INADMISSIBLE HEURISTIC\n";
        RunGOAPs(StartingState, GoalCondition, Actions);
    }

    {
        std::cout << "    ADMISSIBLE HEURISTIC\n";
        X.SetGapWeight(0.01);
        Y.SetGapWeight(0.01);

        float MaxHeuristic = static_cast<float>(GoalCondition.GetConstraint().CountRelations() + GetMaxPreconditionRelations(Actions));
        MaxHeuristic *= Definition.GetBaseRelationCost();
        for (CAction& Action : Actions)
        {
            Action.SetBaseCost(MaxHeuristic);
        }

        RunGOAPs(StartingState, GoalCondition, Actions);
    }

    return 0;
}
