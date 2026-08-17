// Copyright 2026 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows effects using matrix operations: addition, subtraction, and multiplication.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& X = *Definition.DefineMatrix("X", 0.001);
    auto& Y = *Definition.DefineMatrix("Y");
    auto& Z = *Definition.DefineMatrix("Z");
    auto& W = *Definition.DefineMatrix("W");
    auto& G = *Definition.DefineMatrix("G");
    auto& C = *Definition.DefineNumber("C");

    CState StartingState(Definition);
    StartingState.SetProperty(X, {{0, 0}, {0, 0}});
    StartingState.SetProperty(Y, {{1,3}, {13,15}});
    StartingState.SetProperty(Z, {{1,0,-1}, {1,2,5}});
    StartingState.SetProperty(W, {{2,4}, {4,4}, {2,2}});
    StartingState.SetProperty(G, {{1, 2}, {3, 4}});
    StartingState.SetProperty(C, 0.5);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(X == G);

    std::vector<CAction> Actions;
    {
        CAction& Action = Actions.emplace_back("X+=Y", Definition);
        Action.AddEffect(X += Y);
    }
    {
        CAction& Action = Actions.emplace_back("Y-=Z", Definition);
        Action.AddEffect(Y -= Z);
    }
    {
        CAction& Action = Actions.emplace_back("Z*=W", Definition);
        Action.AddEffect(Z *= W);
    }
    {
        CAction& Action = Actions.emplace_back("W*=C", Definition);
        Action.SetPrecondition(G == G);
        Action.AddEffect(W *= C);
    }
    {
        CAction& Action = Actions.emplace_back("X=X+Y-Z*W*C", Definition);
        Action.SetPrecondition(G != G); // This impossible precondition makes this action infeasible.
        Action.AddEffect(X = X+Y-Z*W*C);
    }

    RunGOAPs(StartingState, GoalCondition, Actions);
    return 0;
}

