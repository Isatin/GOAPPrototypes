// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows four effects corresponding to the basic arithmetic operations.
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

    CState StartingState(Definition);
    StartingState.SetProperty(X, 8);
    StartingState.SetProperty(Y, 9);
    StartingState.SetProperty(Z, 78);
    StartingState.SetProperty(W, -17);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(Y == 87);

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
        CAction& Action = Actions.emplace_back("W/=X", Definition);
        Action.AddEffect(W /= X);
    }

    RunGOAPs(StartingState, GoalCondition, Actions);
    return 0;
}

