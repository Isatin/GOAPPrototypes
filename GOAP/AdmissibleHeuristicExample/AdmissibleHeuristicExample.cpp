// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a test case with an inadmissible heuristic and another with an admissible one.
// You can see in the first case, the heuristic overestimates the actual costs.
// By setting the action costs to the number of world properties in the second case, these heuristic  
// values are guaranteed to be no greater than the actual costs.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CState StartingState;
    StartingState.SetProperty("A", false);
    StartingState.SetProperty("B", false);
    StartingState.SetProperty("C", false);
    StartingState.SetProperty("D", false);
    StartingState.SetProperty("E", false);
    StartingState.SetProperty("F", false);

    CState GoalState;
    GoalState.SetProperty("A", true);
    GoalState.SetProperty("B", true);
    GoalState.SetProperty("C", true);
    GoalState.SetProperty("D", true);
    GoalState.SetProperty("E", true);
    GoalState.SetProperty("F", true);

    std::vector<CAction> Actions;
    {
        CAction& ActionA = Actions.emplace_back("A");
        ActionA.SetPrecondition("A", false);
        ActionA.SetEffect("A", true);
    }
    {
        CAction& ActionB = Actions.emplace_back("B");
        ActionB.SetPrecondition("B", false);
        ActionB.SetEffect("B", true);
    }
    {
        CAction& ActionC = Actions.emplace_back("C");
        ActionC.SetPrecondition("C", false);
        ActionC.SetEffect("C", true);
    }
    {
        CAction& ActionD = Actions.emplace_back("D");
        ActionD.SetPrecondition("D", false);
        ActionD.SetEffect("D", true);
    }
    {
        CAction& ActionE = Actions.emplace_back("E");
        ActionE.SetPrecondition("E", false);
        ActionE.SetEffect("E", true);
    }
    {
        CAction& ActionF = Actions.emplace_back("F");
        ActionF.SetPrecondition("F", false);
        ActionF.SetEffect("F", true);
    }
    {
        CAction& ActionAB = Actions.emplace_back("AB");
        ActionAB.SetPrecondition("A", false);
        ActionAB.SetPrecondition("B", false);
        ActionAB.SetEffect("A", true);
        ActionAB.SetEffect("B", true);
    }
    {
        CAction& ActionCD = Actions.emplace_back("CD");
        ActionCD.SetPrecondition("C", false);
        ActionCD.SetPrecondition("D", false);
        ActionCD.SetEffect("C", true);
        ActionCD.SetEffect("D", true);
    } 
    {
        CAction& ActionEF = Actions.emplace_back("EF");
        ActionEF.SetPrecondition("E", false);
        ActionEF.SetPrecondition("F", false);
        ActionEF.SetEffect("E", true);
        ActionEF.SetEffect("F", true);
    }

    std::cout << "    INADMISSIBLE HEURISTIC\n";
    RunGOAPs(StartingState, GoalState, Actions);

    std::cout << "    ADMISSIBLE HEURISTIC\n";
    for (CAction& Action : Actions)
    {
        Action.SetBaseCost(static_cast<float>(StartingState.GetPropertyCount()));
    }
    RunGOAPs(StartingState, GoalState, Actions);

    return 0;
}
