// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows three test cases where effects use multiplication.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    const auto& X = *Definition.DefineNumber("X");

    CState StartingState(Definition);
    StartingState.SetProperty(X, 10);

    CState GoalState(Definition);
    GoalState.SetProperty(X >= 100);

    std::vector<CAction> Actions;
    {
        CAction& Double = Actions.emplace_back("*2", Definition);
        Double.SetEffect(X *= 2);
    }
    {
        CAction& Triple = Actions.emplace_back("*3", Definition);
        Triple.SetEffect(X *= 3);
    }
    {
        CAction& Zeroize = Actions.emplace_back("*0", Definition);
        Zeroize.SetEffect(X *= 0);
    }
    {
        CAction& Opposite = Actions.emplace_back("*-1", Definition);
        Opposite.SetEffect(-X);
    }

    RunGOAPs(StartingState, GoalState, Actions);

    GoalState.SetProperty(X == 0);
    RunGOAPs(StartingState, GoalState, Actions);

    GoalState.SetProperty(X <= -100);
    RunGOAPs(StartingState, GoalState, Actions);

    return 0;
}
