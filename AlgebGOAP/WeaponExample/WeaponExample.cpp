// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is from Dr. Orkin's "Applying Goal-Oriented Action Planning to Games", but an extra 
// precondition of WeaponIsLoaded being false (opposite to its effect) is added to the LoadWeapon 
// action. It demonstrates that the regressive algebraic GOAP can work with actions that have 
// inconsistent preconditions and effects.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& TargetIsDead = *Definition.DefineNumber("TargetIsDead");
    auto& WeaponIsLoaded = *Definition.DefineNumber("WeaponIsLoaded");
    auto& WeaponIsArmed = *Definition.DefineNumber("WeaponIsArmed");

    CState StartingState(Definition);
    StartingState.SetProperty(TargetIsDead, false);
    StartingState.SetProperty(WeaponIsLoaded, false);
    StartingState.SetProperty(WeaponIsArmed, false);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(TargetIsDead == true);  

    std::vector<CAction> Actions;
    {
        CAction& Attack = Actions.emplace_back("A", Definition);
        Attack.SetPrecondition(WeaponIsLoaded == true);
        Attack.AddEffect(TargetIsDead = true);
    }
    {
        CAction& LoadWeapon = Actions.emplace_back("LW", Definition);
        LoadWeapon.SetPrecondition(WeaponIsLoaded == false && WeaponIsArmed == true);
        LoadWeapon.AddEffect(WeaponIsLoaded = true);
    }
    {
        CAction& DrawWeapon = Actions.emplace_back("DW", Definition);
        DrawWeapon.SetPrecondition(WeaponIsArmed == false);
        DrawWeapon.AddEffect(WeaponIsArmed = true);
    }

    RunGOAPs(StartingState, GoalCondition, Actions);
    return 0;
}
