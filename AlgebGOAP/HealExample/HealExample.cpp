// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows that we can make the planners favor one solution over others by adjusting 
// action costs.
// 
// The goal is to restore full health, and the agent has an action to heal a variable amount of  
// health based on a coefficient and a bonus that can be raised by two separate buff actions. 
// In the first run, it uses the heal action twice, which consumes more SP. 
// In the second run, by setting the action costs to their SP costs, it instead applies both buff
// actions and heals once to use less SP. However, this takes more steps.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& MaxHP         = *Definition.DefineNumber("MaxHP");
    auto& HP            = *Definition.DefineNumber("HP", 0.01);
    auto& SP            = *Definition.DefineNumber("SP", 0.01);
    auto& Base          = *Definition.DefineNumber("Base");
    auto& Coef          = *Definition.DefineNumber("Coef");
    auto& Bonus         = *Definition.DefineNumber("Bonus");
    auto& HealSP        = *Definition.DefineNumber("HealSP");
    auto& BuffSP        = *Definition.DefineNumber("BuffSP");
    auto& BuffCoefInc   = *Definition.DefineNumber("CoefInc");
    auto& BuffBonusInc  = *Definition.DefineNumber("BonusInc");

    CState StartingState(Definition);
    StartingState.SetProperty(MaxHP, 100);
    StartingState.SetProperty(HP, 40);
    StartingState.SetProperty(SP, 100);
    StartingState.SetProperty(Base, 30);
    StartingState.SetProperty(Coef, 1);
    StartingState.SetProperty(Bonus, 0);
    StartingState.SetProperty(HealSP, 40);
    StartingState.SetProperty(BuffSP, 10);
    StartingState.SetProperty(BuffCoefInc, 0.5);
    StartingState.SetProperty(BuffBonusInc, 20);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(HP >= MaxHP);

    std::vector<CAction> Actions;
    {
        CAction& Heal = Actions.emplace_back("Heal", Definition);
        Heal.SetPrecondition(HP < MaxHP && SP >= HealSP);
        Heal.AddEffect(HP += Base * Coef + Bonus);
        //Heal.AddEffect(HP = min(HP, MaxHP)); // Cap the healing amount.
        Heal.AddEffect(SP -= HealSP);
    }
    {
        CAction& BuffCoefficient = Actions.emplace_back("Coef+", Definition);
        BuffCoefficient.SetPrecondition(SP >= BuffSP && Coef <= 1);
        BuffCoefficient.AddEffect(SP -= BuffSP);
        BuffCoefficient.AddEffect(Coef += BuffCoefInc);
    }
    {
        CAction& BuffBonus = Actions.emplace_back("Bonus+", Definition);
        BuffBonus.SetPrecondition(SP >= BuffSP && Bonus <= 0);
        BuffBonus.AddEffect(SP -= BuffSP);
        BuffBonus.AddEffect(Bonus += BuffBonusInc);
    }
    std::cout << "    IGNORING SP COSTS\n";
    RunGOAPs(StartingState, GoalCondition, Actions);

    std::cout << "    CONSIDERING SP COSTS\n";
    Actions[0].SetBaseCost(static_cast<float>(StartingState.GetProperty(HealSP)));
    Actions[1].SetBaseCost(static_cast<float>(StartingState.GetProperty(BuffSP)));
    Actions[2].SetBaseCost(static_cast<float>(StartingState.GetProperty(BuffSP)));
    RunGOAPs(StartingState, GoalCondition, Actions);

    return 0;
}

