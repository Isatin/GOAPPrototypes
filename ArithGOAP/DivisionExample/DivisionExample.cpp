// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a case with positive divisors and another case with extra negative divisors.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    const auto& X = *Definition.DefineNumber("X");

    CState StartingState(Definition);
    StartingState.SetProperty(X, 100);

    CState GoalState(Definition);
    GoalState.SetProperty(X <= 10);

    std::vector<CAction> Actions;
    {
        CAction& DivideByZero = Actions.emplace_back("/0", Definition); // Division by zero should be treated as invalid input.
        DivideByZero.SetEffect(X /= 0);
    }
    {
        CAction& Half = Actions.emplace_back("/2", Definition);
        Half.SetEffect(X /= 2);
    }
    {
        CAction& OneThird = Actions.emplace_back("/3", Definition);
        OneThird.SetEffect(X /= 3);
    }
    RunGOAPs(StartingState, GoalState, Actions);

    GoalState.SetProperty(X <= -20);
    {
        CAction& NegHalf = Actions.emplace_back("/-2", Definition);
        NegHalf.SetEffect(X /= -2);
    }
    {
        CAction& NegQuarter = Actions.emplace_back("/-4", Definition);
        NegQuarter.SetEffect(X /= -4);
    }
    RunGOAPs(StartingState, GoalState, Actions);

    return 0;
}
