// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is two test cases of division.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
using VAR = ArithGOAP::SVariable;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CStateDefinition Definition;
    const auto& NumberFact = *Definition.DefineNumber("Number");

    CState StartingState(Definition);
    StartingState.SetFact(NumberFact, 100);

    CState GoalState(Definition);
    GoalState.SetFact(NumberFact <= 10);

    std::vector<CAction> Actions;
    {
        CAction& DivideByZero = Actions.emplace_back("/0", Definition); // Divide-by-zero should be treated as invalid input.
        DivideByZero.SetEffect(NumberFact /= 0);
    }
    {
        CAction& Half = Actions.emplace_back("/2", Definition);
        Half.SetEffect(NumberFact /= 2);
    }
    {
        CAction& OneThird = Actions.emplace_back("/3", Definition);
        OneThird.SetEffect(NumberFact /= 3);
    }
    RunGOAPs(StartingState, GoalState, Actions);

    {
        CAction& NegQuarter = Actions.emplace_back("/-4", Definition);
        NegQuarter.SetEffect(NumberFact /= -4);
    }
    GoalState.SetFact(NumberFact <= -10);
    RunGOAPs(StartingState, GoalState, Actions);

    return 0;
}
