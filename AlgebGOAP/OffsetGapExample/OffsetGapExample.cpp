// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows two cases about an issue of zero heuristic for unsatisfied strict inequalities.
// In the first run, the cost values of the "X+=1" and "X+=2" actions are the same. However, the
// former action doesn't satisfy the goal constraint X > 1, whereas the latter does.
// In the second run, the hiccup is fixed by adding a small heuristic offset for unsatisfied strict 
// inequalities. You can see the first selected action is "X+=2" rather than "X+=1", and the searches 
// take fewer iterations.
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
    StartingState.SetProperty(X, 0);
    StartingState.SetProperty(Y, 0);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(X > 1 && Y < -1);

    std::vector<CAction> Actions;
    {
        CAction& Action = Actions.emplace_back("X+=1", Definition);
        Action.AddEffect(X += 1);
    }
    {
        CAction& Action = Actions.emplace_back("X+=2", Definition);
        Action.AddEffect(X += 2);
    }
    {
        CAction& Action = Actions.emplace_back("Y-=1", Definition);
        Action.AddEffect(Y -= 1);
    }
    {
        CAction& Action = Actions.emplace_back("Y-=2", Definition);
        Action.AddEffect(Y -= 2);
    }

    {
        std::cout << "    NO OFFSET\n";
        RunGOAPs(StartingState, GoalCondition, Actions);
    }

    {
        std::cout << "    WITH an OFFSET\n";
        Definition.SetGapOffset(0.0001_n);
        X.SetGapWeight(0.1);
        Y.SetGapWeight(0.1);

        RunGOAPs(StartingState, GoalCondition, Actions);
    }

    return 0;
}

