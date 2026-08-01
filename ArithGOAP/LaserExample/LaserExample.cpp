// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is from Dr. Orkin's "Applying Goal-Oriented Action Planning to Games", but simplified. 
// The action of activating the generator is replaced with toggling the generator using Boolean negation.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
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
class CGotoAction : public CAction
{
    using Super = CAction;
public:
    using Super::Super;

protected:
    bool CheckPrecondition(const CState& State) const override
    {
        if (const CFact* AtFact = GetDefinition().GetFact("At"))
        {
            const CTransform& Transform = GetEffect().GetTransform(*AtFact);
            if (!Transform.IsNil())
            {
                const SSegment& CurrPlace = State.GetProperty(*AtFact);
                if (CurrPlace.IsSet())
                {
                    if (CurrPlace.Contain(Transform.GetOperand(), State.GetDefinition().GetTolerance())) // Disallow actions that go from and to the same place.
                    {
                        return false;
                    }
                }
            }
        }

        return Super::CheckPrecondition(State);
    }
};
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& TargetHurt    = *Definition.DefineBoolean("TargetHurt");
    auto& At            = *Definition.DefineEnumeration("At");
    auto& Switch        = *Definition.DefineBoolean("Switch");

    CState StartingState(Definition);
    StartingState.SetProperty(TargetHurt == false);
    StartingState.SetProperty(At == EPlace::other);
    StartingState.SetProperty(Switch == false);

    CState GoalState(Definition);
    GoalState.SetProperty(TargetHurt == true);

    std::vector<const CAction*> Actions;

    CGotoAction GotoLaser("GL", Definition);
    Actions.push_back(&GotoLaser);
    GotoLaser.SetEffect(At = EPlace::laser);

    CGotoAction GotoGenerator("GG", Definition);
    Actions.push_back(&GotoGenerator);
    GotoGenerator.SetEffect(At = EPlace::generator);

    CAction ToggleGenerator("TG", Definition);
    Actions.push_back(&ToggleGenerator);
    ToggleGenerator.SetPrecondition(At == EPlace::generator);
    ToggleGenerator.SetEffect(!Switch);    
 
    CAction FireLaser("FL", Definition);
    Actions.push_back(&FireLaser);
    FireLaser.SetPrecondition(At == EPlace::laser);
    FireLaser.SetPrecondition(Switch == true);
    FireLaser.SetEffect(TargetHurt = true);

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
