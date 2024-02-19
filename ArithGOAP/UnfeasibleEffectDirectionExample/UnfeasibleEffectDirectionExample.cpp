// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows that if we build lookup tables for world properties whether they could be 
// increased/decreased/both by the effects of all actions, we can use them to rule out some 
// impossible desired values in the iteration of regressive search for performance.
// For instance, the fact of Credit can only decreased and thus it's impposible for desired values 
// to reach the starting value. So the regression from GetLoad won't continue.
// As proof, the numbers of steps and nodes in the last search with lookup tables are less than 
// the second search.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
using VAR = ArithGOAP::SVariable;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CStateDefinition Definition;
    auto& MoneyFact = *Definition.DefineNumber(VAR("Money") >= 0);
    auto& CreditFact = *Definition.DefineNumber("Credit", 0.01);
    auto& FatigueFact = *Definition.DefineNumber(VAR("Fatigue") >= 0);

    CState StartingState(Definition);
    StartingState.SetFact(MoneyFact, 0);
    StartingState.SetFact(CreditFact, 10);
    StartingState.SetFact(FatigueFact, 2);

    CState GoalState(Definition);
    GoalState.SetFact(MoneyFact >= 2);

    std::vector<CAction> Actions;
    {
        CAction& GetLoan = Actions.emplace_back("L", Definition);
        GetLoan.SetPrecondition(CreditFact >= 100);
        GetLoan.SetEffect(CreditFact -= 100);
        GetLoan.SetEffect(MoneyFact += 100);
    }
    {
        CAction& Work = Actions.emplace_back("W", Definition);
        Work.SetPrecondition(FatigueFact <= 0);
        Work.SetEffect(FatigueFact += 1);
        Work.SetEffect(MoneyFact += 1);
    }
    {
        CAction& Rest = Actions.emplace_back("R", Definition);
        Rest.SetPrecondition(FatigueFact >= 1);
        Rest.SetEffect(FatigueFact -= 2);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
