// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is from Dr. Orkin's "Applying Goal-Oriented Action Planning to Games", but simplified.
// It demonstrates that the improved regressive GOAP can handle actions with inconsistent 
// preconditions and effects (see the Switch settings of the ActivateGenerator action), 
// while the original regressive GOAP (see the result of CBackwardPlanner) cannot. 
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace EPlace
{
    enum
    {
        other,
        laser,
        generator,
    };
}
///////////////////////////////////////////////////////////////////////////////////////////////////
class CGotoAction : public CAction
{
    using Super = CAction;
public:
    using Super::Super;

protected:
    bool CheckPrecondition(const CState& State) const override
    {
        if (std::optional<BProperty> NewPlace = GetEffect().GetProperty("At"))
        {
            if (std::optional<BProperty> CurrPlace = State.GetProperty("At"))
            {
                if (NewPlace == CurrPlace) // Disallow actions that go from and to the same place.
                {
                    return false;
                }
            }
        }

        return Super::CheckPrecondition(State);
    }
};
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CState StartingState;
    StartingState.SetProperty("TargetHurt", false);
    StartingState.SetProperty("At", EPlace::other);
    StartingState.SetProperty("Switch", false);

    CState GoalState;
    GoalState.SetProperty("TargetHurt", true);

    std::vector<const CAction*> Actions;

    CGotoAction GotoLaser("GL");
    GotoLaser.SetEffect("At", EPlace::laser);
    Actions.push_back(&GotoLaser);

    CGotoAction GotoGenerator("GG");
    GotoGenerator.SetEffect("At", EPlace::generator);
    Actions.push_back(&GotoGenerator);

    CAction ActivateGenerator("AG");
    ActivateGenerator.SetPrecondition("At", EPlace::generator);
    ActivateGenerator.SetPrecondition("Switch", false); // This line makes the backward GOAP not work.
    ActivateGenerator.SetEffect("Switch", true);
    Actions.push_back(&ActivateGenerator);

    CAction FireLaser("FL");
    FireLaser.SetPrecondition("At", EPlace::laser);
    FireLaser.SetPrecondition("Switch", true);
    FireLaser.SetEffect("TargetHurt", true);
    Actions.push_back(&FireLaser);

    RunGOAPs(StartingState, GoalState, Actions, 0, DefaultGOAPs | BackwardGOAP);
    return 0;
}
