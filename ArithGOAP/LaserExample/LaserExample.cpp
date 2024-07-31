// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is from Orkin's "Applying Goal-Oriented Action Planning to Games" but simplified. 
// The action of activating generator is replaced with toggling generator using Boolean negation.
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
            if (const CTransform& MyTransform = GetEffect().GetTransform(*AtFact))
            {
                if (const SInterval& CurrentPlace = State.GetFact(*AtFact))
                {
                    if (CurrentPlace.Contain(MyTransform.GetOperand())) // Disallow actions from and to the same place
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
    CStateDefinition Definition;
    auto& TargetHurtFact    = *Definition.DefineBoolean("TargetHurt");
    auto& AtFact            = *Definition.DefineEnumeration("At");
    auto& SwitchFact        = *Definition.DefineBoolean("Switch");

    CState StartingState(Definition);
    StartingState.SetFact(TargetHurtFact == false);
    StartingState.SetFact(AtFact == EPlace::other);
    StartingState.SetFact(SwitchFact == false);

    CState GoalState(Definition);
    GoalState.SetFact(TargetHurtFact == true);

    std::vector<const CAction*> Actions;

    CGotoAction GotoLaser("GL", Definition);
    Actions.push_back(&GotoLaser);
    GotoLaser.SetEffect(AtFact = EPlace::laser);

    CGotoAction GotoGenerator("GG", Definition);
    Actions.push_back(&GotoGenerator);
    GotoGenerator.SetEffect(AtFact = EPlace::generator);

    CAction ToggleGenerator("TG", Definition);
    Actions.push_back(&ToggleGenerator);
    ToggleGenerator.SetPrecondition(AtFact == EPlace::generator);
    ToggleGenerator.SetEffect(!SwitchFact);    
 
    CAction FireLaser("FL", Definition);
    Actions.push_back(&FireLaser);
    FireLaser.SetPrecondition(AtFact == EPlace::laser);
    FireLaser.SetPrecondition(SwitchFact == true);
    FireLaser.SetEffect(TargetHurtFact = true);

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
