// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a common scenario in games where an agent must gather resources using various 
// methods for a task. In this case, it has three actions to earn enough money to buy a house.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
using VAR = ArithGOAP::SVariable;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    const auto& Balance = *Definition.DefineNumber(VAR("Balance") >= 0, 0.001);
    const auto& Credit  = *Definition.DefineNumber("Credit", 0.01);
    const auto& House   = *Definition.DefineNumber("House");

    CState StartingState(Definition);
    StartingState.SetProperty(Balance == 10);
    StartingState.SetProperty(Credit == 0);
    StartingState.SetProperty(House == 0);
    
    CState GoalState(Definition);
    GoalState.SetProperty(House >= 1);

    std::vector<CAction> Actions;
    {
        CAction& BuyHouse = Actions.emplace_back("BH", Definition);
        BuyHouse.SetPrecondition(Balance >= 1000);
        BuyHouse.SetEffect(Balance -= 1000);
        BuyHouse.SetEffect(House += 1);
    }
    {
        CAction& Work = Actions.emplace_back("W", Definition);
        Work.SetEffect(Balance += 100);
        Work.SetEffect(Credit += 10);
    }
    {
        CAction& RunShop = Actions.emplace_back("RS", Definition);
        RunShop.SetPrecondition(Credit >= 0);
        RunShop.SetEffect(Balance += 300);
    }
    {
        CAction& GetLoan = Actions.emplace_back("GL", Definition);
        GetLoan.SetPrecondition(Credit >= 0);
        GetLoan.SetEffect(Credit -= 70);
        GetLoan.SetEffect(Balance += 700);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
