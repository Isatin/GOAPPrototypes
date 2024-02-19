// Copyright 2024 Isaac Hsu

#include <sstream>

#include "Action.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CAction::CAction(const std::string& Name)
    : mName(Name)
{
}

std::string CAction::ToString() const
{
    std::stringstream Stream;
    Stream << mName << "{ ";

    std::string PreconditionStr = mPrecondition.ToString();
    if (!PreconditionStr.empty())
    {
        Stream << PreconditionStr << " ";
    }

    Stream << "=>";

    std::string EffectStr = mEffect.ToString();
    if (!EffectStr.empty())
    {
        Stream << " " << EffectStr;
    }

    Stream << " }";
    return Stream.str();
}

bool CAction::CheckPrecondition(const CState& State) const
{
    if (!mPrecondition.IsSatisfiedBy(State))
    {
        return false;
    }

    if (mEffect.GetFactCount() <= 0)
    {
        return false; // Ineffective action
    }

    return true;
}

bool CAction::CheckPostcondition(const CState& State) const
{
    // This action is considered as a candidate in regressive search if it has at least one desired effect
    // and the current desired state has no conflicts with its effects and preconditions.
    bool AnySatisfaction = false;

    for (auto& [Key, Aim] : State)
    {
        std::optional<PFact> Effect = mEffect.GetFact(Key);
        if (Effect == Aim)
        {
            AnySatisfaction = true;
            continue; // This fact will be removed as it's satisfied by the effect so it's needless to check conflict with the precondition.
        }
        else if (Effect)
        {
            return false; // Any mismatched effects are not allowed.
        }

        std::optional<PFact> Requirement = mPrecondition.GetFact(Key);
        if (Requirement && Requirement != Aim)
        {
            return false; // Preconditions will become desired facts but this precondition conflicts with current state.
        }
    }

    return AnySatisfaction;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
