// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example simply shows that the heuristic function based on the number of unsatisfied 
// world properties could be non-admissible (heuristic cost > actual cost).
// As proof, the heuristic cost of the root is 6 while the actual cost is 3 upon success.
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
        CAction& A = Actions.emplace_back("A");
        A.SetPrecondition("A", false);
        A.SetEffect("A", true);
    }
    {
        CAction& B = Actions.emplace_back("B");
        B.SetPrecondition("B", false);
        B.SetEffect("B", true);
    }
    {
        CAction& C = Actions.emplace_back("C");
        C.SetPrecondition("C", false);
        C.SetEffect("C", true);
    }
    {
        CAction& D = Actions.emplace_back("D");
        D.SetPrecondition("D", false);
        D.SetEffect("D", true);
    }
    {
        CAction& E = Actions.emplace_back("E");
        E.SetPrecondition("E", false);
        E.SetEffect("E", true);
    }
    {
        CAction& F = Actions.emplace_back("F");
        F.SetPrecondition("F", false);
        F.SetEffect("F", true);
    }
    {
        CAction& NoEffect = Actions.emplace_back("N");
    }
    {
        CAction& AB = Actions.emplace_back("AB");
        AB.SetPrecondition("A", false);
        AB.SetPrecondition("B", false);
        AB.SetEffect("A", true);
        AB.SetEffect("B", true);
    }
    {
        CAction& CD = Actions.emplace_back("CD");
        CD.SetPrecondition("C", false);
        CD.SetPrecondition("D", false);
        CD.SetEffect("C", true);
        CD.SetEffect("D", true);
    }
    {
        CAction& EF = Actions.emplace_back("EF");
        EF.SetPrecondition("E", false);
        EF.SetPrecondition("F", false);
        EF.SetEffect("E", true);
        EF.SetEffect("F", true);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
