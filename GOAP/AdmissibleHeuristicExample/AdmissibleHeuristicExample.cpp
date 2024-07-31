// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a test case with admissible heuristic and non-admissible one.
// By setting the action costs to the number of world properties in the second run, the heuristic is 
// guaranteed to be no greater than the costs.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CState StartingState;
    StartingState.SetFact("A", false);
    StartingState.SetFact("B", false);
    StartingState.SetFact("C", false);
    StartingState.SetFact("D", false);
    StartingState.SetFact("E", false);
    StartingState.SetFact("F", false);

    CState GoalState;
    GoalState.SetFact("A", true);
    GoalState.SetFact("B", true);
    GoalState.SetFact("C", true);
    GoalState.SetFact("D", true);
    GoalState.SetFact("E", true);
    GoalState.SetFact("F", true);

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

    std::cout << "    NON-ADMISSIBLE HEURISTIC\n";
    RunGOAPs(StartingState, GoalState, Actions);

    std::cout << "    ADMISSIBLE HEURISTIC\n";
    for (CAction& Action : Actions)
    {
        Action.SetBaseCost((float) StartingState.GetFactCount());
    }
    RunGOAPs(StartingState, GoalState, Actions);

    return 0;
}
