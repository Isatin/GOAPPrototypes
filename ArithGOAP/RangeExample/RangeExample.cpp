// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example demonstrates that range limits on world properties could be useful in some cases. 
// The goal is to earn enough Money by taking the Work action, which requires zero Fatigue. 
// In addition, Work increases Fatigue, while Rest decreases it.
// 
// The planners run without range limits first, and then with range limits, to show the differences. 
// You can see that, in the first run, the Fatigue values are negative in some iterations. However, 
// they are not allowed to drop below zero in the game world. By constraining the property to be 
// non-negative, the planners can find the correct solution in the second run.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& Money     = *Definition.DefineNumber("Money");
    auto& Fatigue   = *Definition.DefineNumber("Fatigue");

    CState StartingState(Definition);
    StartingState.SetProperty(Money, 0);
    StartingState.SetProperty(Fatigue, 4);

    CState GoalState(Definition);
    GoalState.SetProperty(Money >= 8);

    std::vector<CAction> Actions;
    {
        CAction& Work = Actions.emplace_back("W", Definition);
        Work.SetPrecondition(Fatigue <= 0);
        Work.SetEffect(Money += 5);
        Work.SetEffect(Fatigue += 2);
    }
    {
        CAction& Rest = Actions.emplace_back("R", Definition);
        Rest.SetPrecondition(Fatigue >= 1);
        Rest.SetEffect(Fatigue -= 3);
    }

    const int MaxSteps = 5;
    {
        std::cout << "    NO RANGE LIMITS\n";
        RunGOAPs(StartingState, GoalState, Actions, MaxSteps);
    }
    
    {
        std::cout << "    WITH RANGE LIMITS\n";
        Money.SetRange(SSegment(EComparer::GreaterOrEqual, 0));
        Fatigue.SetRange(SSegment(EComparer::GreaterOrEqual, 0));
        RunGOAPs(StartingState, GoalState, Actions, MaxSteps);
    }

    return 0;
}
