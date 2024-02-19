// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a common scenario in game where the agent needs to get enough resources 
// in several ways for something. In this case, it can take three actions to make money for a house.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
using VAR = ArithGOAP::SVariable;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CStateDefinition Definition;
    const auto& BalanceFact = *Definition.DefineNumber(VAR("Balance") >= 0, 0.001);
    const auto& CreditFact  = *Definition.DefineNumber("Credit", 0.01);
    const auto& HouseFact   = *Definition.DefineNumber("House");

    CState StartingState(Definition);
    StartingState.SetFact(BalanceFact == 10);
    StartingState.SetFact(CreditFact == 0);
    StartingState.SetFact(HouseFact == 0);
    
    CState GoalState(Definition);
    GoalState.SetFact(HouseFact >= 1);

    std::vector<CAction> Actions;
    {
        CAction& BuyHouse = Actions.emplace_back("BH", Definition);
        BuyHouse.SetPrecondition(BalanceFact >= 1000);
        BuyHouse.SetEffect(BalanceFact -= 1000);
        BuyHouse.SetEffect(HouseFact += 1);
    }
    {
        CAction& Work = Actions.emplace_back("W", Definition);
        Work.SetEffect(BalanceFact += 100);
        Work.SetEffect(CreditFact += 10);
    }
    {
        CAction& RunShop = Actions.emplace_back("RS", Definition);
        RunShop.SetPrecondition(CreditFact >= 0);
        RunShop.SetEffect(BalanceFact += 300);
    }
    {
        CAction& GetLoan = Actions.emplace_back("GL", Definition);
        GetLoan.SetPrecondition(CreditFact >= 0);
        GetLoan.SetEffect(CreditFact -= 70);
        GetLoan.SetEffect(BalanceFact += 700);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
