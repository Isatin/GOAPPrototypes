// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This test case fails the goal with ineffective and unsatisfactory actions.
// Advanced regressive GOAP can rule out these actions with lookup tables during search.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    const auto& X = *Definition.DefineNumber("X");

    CState StartingState(Definition);
    StartingState.SetProperty(X, 1);

    CState GoalState(Definition);
    GoalState.SetProperty(X >= 2);

    std::vector<CAction> Actions;
    // An ineffective action with an effect equal to the additive identity
    {
        CAction& PlusZero = Actions.emplace_back("+0", Definition);
        PlusZero.SetEffect(X += 0);
    }
    // An ineffective action with an effect equal to the multiplicative identity
    {
        CAction& TimesOne = Actions.emplace_back("*1", Definition);
        TimesOne.SetEffect(X *= 1);
    }  
    // An unsatisfactory action with an effect of multiplying by zero
    {
        CAction& TimesZero = Actions.emplace_back("*0", Definition);
        TimesZero.SetEffect(X *= 0);
    }
    //// A satisfactory action
    //{
    //    CAction& Two = Actions.emplace_back("=2", Definition);
    //    Two.SetEffect(X = 2);
    //}
    RunGOAPs(StartingState, GoalState, Actions);

    return 0;
}
