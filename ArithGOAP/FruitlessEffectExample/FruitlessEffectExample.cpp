// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is two test cases with ineffective/unsatisfying actions and they will fail.
// Regressive GOAP with lookup tables will rule out these actions during search.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CStateDefinition Definition;
    const auto& NumberFact = *Definition.DefineNumber("Number");

    CState StartingState(Definition);
    StartingState.SetFact(NumberFact, 1);

    CState GoalState(Definition);
    GoalState.SetFact(NumberFact >= 2);

    std::vector<CAction> Actions;
    {
        CAction& AddZero = Actions.emplace_back("+0", Definition);
        AddZero.SetEffect(NumberFact += 0);
    }
    {
        CAction& TimesOne = Actions.emplace_back("*1", Definition);
        TimesOne.SetEffect(NumberFact *= 1);
    }  
    // Only ineffective actions
    RunGOAPs(StartingState, GoalState, Actions);

    // Unsatisfying effect 
    {
        CAction& TimesZero = Actions.emplace_back("*0", Definition);
        TimesZero.SetEffect(NumberFact *= 0);
    }
    //// Satisfying effect 
    //{
    //    CAction& Two = Actions.emplace_back("=2", Definition);
    //    Two.SetEffect(NumberFact = 2);
    //}
    RunGOAPs(StartingState, GoalState, Actions);

    return 0;
}
