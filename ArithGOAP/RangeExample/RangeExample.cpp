// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows that range limits of world properties could sometimes be useful.
// The three GOAP algorithms run twice: 
// The first run is without range limits and the second run is with range limits.
// 
// You can see that in the first run Fatigue is negative in some iterations.
// Suppose that Fatigue shouldn't be negative in the simulation, then the resulting plans are wrong as well.
// To fix that, we can put range limits on the facts and they'll result in correct plans in the second run.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CStateDefinition Definition;
    auto& MoneyFact = *Definition.DefineNumber("Money");
    auto& FatigueFact = *Definition.DefineNumber("Fatigue");

    CState StartingState(Definition);
    StartingState.SetFact(MoneyFact, 0);
    StartingState.SetFact(FatigueFact, 4);

    CState GoalState(Definition);
    GoalState.SetFact(MoneyFact >= 8);

    std::vector<CAction> Actions;
    {
        CAction& Work = Actions.emplace_back("W", Definition);
        Work.SetPrecondition(FatigueFact <= 0);
        Work.SetEffect(MoneyFact += 5);
        Work.SetEffect(FatigueFact += 2);
    }
    {
        CAction& Rest = Actions.emplace_back("R", Definition);
        Rest.SetPrecondition(FatigueFact >= 1);
        Rest.SetEffect(FatigueFact -= 3);
    }

    // GOAP without ranges
    RunGOAPs(StartingState, GoalState, Actions, 5);
    
    // GOAP with ranges
    MoneyFact.SetRange(SInterval(0, SNumber::Infinity));
    FatigueFact.SetRange(SInterval(0, SNumber::Infinity));
    RunGOAPs(StartingState, GoalState, Actions, 5);

    return 0;
}
