// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows constraints and effects using the three basic Boolean operations: AND, OR, and NOT.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& X = *Definition.DefineBoolean("X");
    auto& Y = *Definition.DefineBoolean("Y");
    auto& Z = *Definition.DefineBoolean("Z");
    auto& W = *Definition.DefineBoolean("W");

    CState StartingState(Definition);
    StartingState.SetProperty(X, true);
    StartingState.SetProperty(Y, false);
    StartingState.SetProperty(Z, false);
    StartingState.SetProperty(W, false);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint((!X && Y) != (Z || W));

    std::vector<CAction> Actions;
    {
        CAction& Action = Actions.emplace_back("X=Y", Definition);
        Action.SetPrecondition(X == !Y);
        Action.AddEffect(X = Y);
    }
    {
        CAction& Action = Actions.emplace_back("X=Y=Z", Definition);
        Action.AddEffect(Y = Z);
        Action.AddEffect(X = Y);
    }
    {
        CAction& Action = Actions.emplace_back("Y=!(Z&&W)", Definition);
        Action.AddEffect(Y = !(Z && W));
    }

    RunGOAPs(StartingState, GoalCondition, Actions);
    return 0;
}

