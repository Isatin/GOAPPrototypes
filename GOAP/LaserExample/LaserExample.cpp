// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is from Orkin's "Applying Goal-Oriented Action Planning to Games" but simplified.
// It shows that GOAP can not only support Boolean but also enumerations.
// Also, it demonstrates that the improved regressive GOAP can deal with inconsistency between
// preconditions and effects of the same actions (see ActivateGenerator). 
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
        if (std::optional<PFact> MyPlace = GetEffect().GetFact("At"))
        {
            if (std::optional<PFact> CurrentPlace = State.GetFact("At"))
            {
                if (MyPlace == CurrentPlace) // Disallow the same start and destination
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
    StartingState.SetFact("TargetHurt", false);
    StartingState.SetFact("At", EPlace::other);
    StartingState.SetFact("Switch", false);

    CState GoalState;
    GoalState.SetFact("TargetHurt", true);

    std::vector<const CAction*> Actions;

    CGotoAction GotoLaser("GL");
    GotoLaser.SetEffect("At", EPlace::laser);
    Actions.push_back(&GotoLaser);

    CGotoAction GotoGenerator("GG");
    GotoGenerator.SetEffect("At", EPlace::generator);
    Actions.push_back(&GotoGenerator);

    CAction ActivateGenerator("AG");
    ActivateGenerator.SetPrecondition("At", EPlace::generator);
    ActivateGenerator.SetPrecondition("Switch", false);
    ActivateGenerator.SetEffect("Switch", true);
    Actions.push_back(&ActivateGenerator);

    CAction FireLaser("FL");
    FireLaser.SetPrecondition("At", EPlace::laser);
    FireLaser.SetPrecondition("Switch", true);
    FireLaser.SetEffect("TargetHurt", true);
    Actions.push_back(&FireLaser);

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
