// Copyright 2024 Isaac Hsu

#include "Action.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CAction::CAction(const std::string& Name)
    : mName(Name)
{}

std::string CAction::ToString() const
{
    std::string Return = mName;
    Return += "{ ";

    if (!mPrecondition.IsEmpty())
    {
        Return += mPrecondition.ToString();
        Return += ' ';
    }

    Return += "->";

    std::string EffectText = mEffect.ToString();
    if (!EffectText.empty())
    {
        Return += ' ';
        Return += EffectText;
    }

    Return += " }";
    return Return;
}

bool CAction::CheckPrecondition(const CState& State) const
{
    if (!mPrecondition.IsSatisfiedBy(State))
    {
        return false; // The precondition is unmet.
    }

    if (mEffect.GetPropertyCount() <= 0)
    {
        return false; // This action is ineffective.
    }

    return true;
}

bool CAction::CheckPostcondition(const CState& State) const
{
    // This action is considered a candidate in the regressive search if it has at least one desired effect,
    // and its effects and preconditions have no conflicts with the desired state.
    bool AnySatisfaction = false;

    for (auto& [Name, Target] : State)
    {
        std::optional<BProperty> Effect = mEffect.GetProperty(Name);
        if (Effect == Target)
        {
            AnySatisfaction = true;
            continue; // This property will be removed as it's satisfied by the effect, making it unnecessary to check against the precondition.
        }
        else if (Effect)
        {
            return false; // Any mismatched effects are not allowed.
        }

        std::optional<BProperty> Constraint = mPrecondition.GetProperty(Name);
        if (Constraint && Constraint != Target)
        {
            return false; // The precondition conflicts with the desired state, making the action infeasible.
        }
    }

    return AnySatisfaction;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
