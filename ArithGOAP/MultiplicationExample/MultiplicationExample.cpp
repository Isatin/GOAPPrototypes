// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is three test cases of multiplication.
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
    StartingState.SetFact(NumberFact, 10);

    CState GoalState(Definition);
    GoalState.SetFact(NumberFact >= 100);

    std::vector<CAction> Actions;
    {
        CAction& Double = Actions.emplace_back("*2", Definition);
        Double.SetEffect(NumberFact *= 2);
    }
    {
        CAction& Triple = Actions.emplace_back("*3", Definition);
        Triple.SetEffect(NumberFact *= 3);
    }
    {
        CAction& Zeroize = Actions.emplace_back("*0", Definition);
        Zeroize.SetEffect(NumberFact *= 0);
    }
    {
        CAction& Negation = Actions.emplace_back("*-1", Definition);
        Negation.SetEffect(-NumberFact);
    }

    RunGOAPs(StartingState, GoalState, Actions);

    GoalState.SetFact(NumberFact == 0);
    RunGOAPs(StartingState, GoalState, Actions);

    GoalState.SetFact(NumberFact <= -100);
    RunGOAPs(StartingState, GoalState, Actions);

    return 0;
}
