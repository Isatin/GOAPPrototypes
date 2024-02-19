// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is two test cases of ineffective actions and they will fail.
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
        CAction& PlusZero = Actions.emplace_back("+0", Definition);
        PlusZero.SetEffect(NumberFact += 0);
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
