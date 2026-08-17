// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows constraints and effects using the three basic logical operations: AND, OR, and NOT.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& X = *Definition.DefineNumber("X");
    auto& Y = *Definition.DefineNumber("Y");
    auto& Z = *Definition.DefineNumber("Z");
    auto& W = *Definition.DefineNumber("W");
    auto& V = *Definition.DefineNumber("V");
    auto& A = *Definition.DefineNumber("A");

    CState StartingState(Definition);
    StartingState.SetProperty(X, 1);
    StartingState.SetProperty(Y, 1);
    StartingState.SetProperty(A, 1);
    StartingState.SetProperty(Z, 1);
    StartingState.SetProperty(W, 1);
    StartingState.SetProperty(V, 1);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(!((X >= 0 || !(Y <= 0)) && A == 1 || Z == W));

    std::vector<CAction> Actions;
    {
        CAction& Action = Actions.emplace_back("X=-1", Definition);
        Action.SetPrecondition(!(X == -1) && (W < 3 || V > 0));
        Action.AddEffect(X = -1);
    }
    {
        CAction& Action = Actions.emplace_back("Y=-1", Definition);
        Action.SetPrecondition(!(Y != 1) && (0.3 * W < 0.6 || 7 * V > 0));
        Action.AddEffect(Y = -1);
    }
    {
        CAction& Action = Actions.emplace_back("Z=-1", Definition);
        Action.SetPrecondition(Z != -1 && (W <= 4 || V > 0 || W < 3 || W != 2)); // The ORed constraints are redundant, so CAdvRegressionPlanner will remove them in preprocessing.
        Action.AddEffect(Z = -1);
    }
    {
        CAction& Action = Actions.emplace_back("W=-1", Definition);
        Action.SetPrecondition(W != -1);
        Action.AddEffect(W = -1);
    }
    {
        CAction& Action = Actions.emplace_back("V=-1", Definition);
        Action.SetPrecondition(V != -1);
        Action.AddEffect(V = -1);
    }

    RunGOAPs(StartingState, GoalCondition, Actions);
    return 0;
}

