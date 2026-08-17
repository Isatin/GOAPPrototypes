// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a case in which we can improve search efficiency by identifying inconsistent
// compound inequalities. You can see that CRegressionPlanner searches along the path, W=6⇨Z=W,
// which results in desired constraints: (X×Y≳2)∧(X×Y≲1), whereas CAdvRegressionPlanner avoids that 
// infeasible path and finds a solution in fewer iterations.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& X = *Definition.DefineNumber("X", 0.1);
    auto& Y = *Definition.DefineNumber("Y", 0.1);
    auto& Z = *Definition.DefineNumber("Z", 0.1);
    auto& W = *Definition.DefineNumber("W", 0.1);

    CState StartingState(Definition);
    StartingState.SetProperty(X, 0);
    StartingState.SetProperty(Y, 0);
    StartingState.SetProperty(Z, 1);
    StartingState.SetProperty(W, 1);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(Z >= 5);

    std::vector<CAction> Actions;
    {
        CAction& Action = Actions.emplace_back("X=2", Definition);
        Action.SetPrecondition(X == 0);
        Action.AddEffect(X = 2);
    }
    {
        CAction& Action = Actions.emplace_back("Y=4", Definition);
        Action.SetPrecondition(Y == 0);
        Action.AddEffect(Y = 4);
    }
    {
        CAction& Action = Actions.emplace_back("Z=W", Definition);
        Action.SetPrecondition(X * Y >= 2);
        Action.AddEffect(Z = W);
    }
    {
        CAction& Action = Actions.emplace_back("W=6", Definition);
        Action.SetPrecondition(X * Y <= 1);
        Action.AddEffect(W = 6);
    }

    RunGOAPs(StartingState, GoalCondition, Actions);
    return 0;
}

