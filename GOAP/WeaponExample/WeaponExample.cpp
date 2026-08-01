// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is from Dr. Orkin's "Applying Goal-Oriented Action Planning to Games". As it's 
// probably the best-known example of GOAP, it serves as a good starting point to see how the  
// improved regressive algorithm works compared to the original method (see CBackwardPlanner).
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CState StartingState;
    StartingState.SetProperty("TargetIsDead", false);
    StartingState.SetProperty("WeaponIsLoaded", false);
    StartingState.SetProperty("WeaponIsArmed", false);

    CState GoalState;
    GoalState.SetProperty("TargetIsDead", true);  

    std::vector<CAction> Actions;
    {
        CAction& Attack = Actions.emplace_back("A");
        Attack.SetPrecondition("WeaponIsLoaded", true);
        Attack.SetEffect("TargetIsDead", true);
    }
    {
        CAction& LoadWeapon = Actions.emplace_back("LW");
        LoadWeapon.SetPrecondition("WeaponIsArmed", true);
        //LoadWeapon.SetPrecondition("WeaponIsLoaded", false); // This line makes the backward GOAP not work.
        LoadWeapon.SetEffect("WeaponIsLoaded", true);
    }
    {
        CAction& DrawWeapon = Actions.emplace_back("DW");
        DrawWeapon.SetEffect("WeaponIsArmed", true);
    }

    RunGOAPs(StartingState, GoalState, Actions, 0, DefaultGOAPs | BackwardGOAP);
    return 0;
}
