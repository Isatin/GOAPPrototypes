// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows that we can improve search efficiency by building a lookup table of effect 
// directions for each numeric property to spot unsatisfiable desired constraints and prune their 
// search branches. For instance, the advanced regression planner excludes further search from the 
// GetLoad action because it requires a higher Credit than its starting value, which is infeasible, 
// as the table indicates Credit can only be decreased by these actions.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
using VAR = ArithGOAP::SVariable;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& Money     = *Definition.DefineNumber(VAR("Money") >= 0, 0.1);
    auto& Credit    = *Definition.DefineNumber("Credit", 0.1);
    auto& Fatigue   = *Definition.DefineNumber(VAR("Fatigue") >= 0, 0.1);

    CState StartingState(Definition);
    StartingState.SetProperty(Money, 0);
    StartingState.SetProperty(Credit, 1);
    StartingState.SetProperty(Fatigue, 2);

    CState GoalState(Definition);
    GoalState.SetProperty(Money >= 2);

    std::vector<CAction> Actions;
    {
        CAction& GetLoan = Actions.emplace_back("GL", Definition);
        GetLoan.SetPrecondition(Credit >= 10);
        GetLoan.SetEffect(Credit -= 10);
        GetLoan.SetEffect(Money += 10);
    }
    {
        CAction& Work = Actions.emplace_back("W", Definition);
        Work.SetPrecondition(Fatigue <= 0);
        Work.SetEffect(Fatigue += 1);
        Work.SetEffect(Money += 1);
    }
    {
        CAction& Rest = Actions.emplace_back("R", Definition);
        Rest.SetPrecondition(Fatigue >= 1);
        Rest.SetEffect(Fatigue -= 2);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
