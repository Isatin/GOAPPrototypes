// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is from Dr. Orkin's "Applying Goal-Oriented Action Planning to Games", but simplified. 
// You can see that logical AND, Boolean NOT, and not-equal are also supported in the algebraic GOAP.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace EPlace
{
    enum Type
    {
        other,
        laser,
        generator,
    };
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& TargetHurt    = *Definition.DefineBoolean("TargetHurt");
    auto& At            = *Definition.DefineEnumeration("At");
    auto& Switch        = *Definition.DefineBoolean("Switch");

    CState StartingState(Definition);
    StartingState.SetProperty(TargetHurt, false);
    StartingState.SetProperty(At, EPlace::other);
    StartingState.SetProperty(Switch, false);

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(TargetHurt == true);

    std::vector<CAction> Actions;
    {
        CAction& GotoLaser = Actions.emplace_back("GL", Definition);
        GotoLaser.SetPrecondition(At != EPlace::laser);
        GotoLaser.AddEffect(At = EPlace::laser);
    }
    {
        CAction& GotoGenerator = Actions.emplace_back("GG", Definition);
        GotoGenerator.SetPrecondition(At != EPlace::generator);
        GotoGenerator.AddEffect(At = EPlace::generator);
    }
    {
        CAction& ToggleGenerator = Actions.emplace_back("TG", Definition);
        ToggleGenerator.SetPrecondition(At == EPlace::generator);
        ToggleGenerator.AddEffect(Switch = !Switch);
    }
    {
        CAction& FireLaser = Actions.emplace_back("FL", Definition);
        FireLaser.SetPrecondition(At == EPlace::laser && Switch == true);
        FireLaser.AddEffect(TargetHurt = true);
    }

    RunGOAPs(StartingState, GoalCondition, Actions);
    return 0;
}
