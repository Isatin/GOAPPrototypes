// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is from Orkin's "Applying Goal-Oriented Action Planning to Games" as it's probably
// the most known example of GOAP and it would be a good starting point to learn the improved 
// regressive algorithm by comparing the output of iterations with the original method.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CState StartingState;
    StartingState.SetFact("TargetIsDead", false);
    StartingState.SetFact("WeaponIsLoaded", false);
    StartingState.SetFact("WeaponIsArmed", false);

    CState GoalState;
    GoalState.SetFact("TargetIsDead", true);  

    std::vector<CAction> Actions;
    {
        CAction& Attack = Actions.emplace_back("A");
        Attack.SetPrecondition("WeaponIsLoaded", true);
        Attack.SetEffect("TargetIsDead", true);
    }
    {
        CAction& LoadWeapon = Actions.emplace_back("LW");
        LoadWeapon.SetPrecondition("WeaponIsArmed", true);
        LoadWeapon.SetEffect("WeaponIsLoaded", true);
    }
    {
        CAction& DrawWeapon = Actions.emplace_back("DW");
        DrawWeapon.SetEffect("WeaponIsArmed", true);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
