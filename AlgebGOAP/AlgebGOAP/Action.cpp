// Copyright 2025 Isaac Hsu

#include <cassert>

#include "Action.h"
#include "Fact.h"
#include "State.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CAction::CAction(const std::string& Name, const CFactDefinition& Definition, float BaseCost)
    : mName(Name)
    , mBaseCost(BaseCost)
    , mPrecondition(Definition)
    , mEffect(Definition)
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

float CAction::GetCost(const CState& CurrentState, const CState& NextState) const 
{ 
    return mBaseCost + GetCustomCost(CurrentState, NextState); 
}

float CAction::GetCost(const CCondition& CurrentCondition, const CCondition& NextCondition) const 
{ 
    return mBaseCost + GetCustomCost(CurrentCondition, NextCondition); 
}

void CAction::SetBaseCost(float Value) 
{
    assert(Value > 0);

    mBaseCost = Value;
}

bool CAction::CheckPrecondition(const CState& State) const
{
    if (!mPrecondition.IsSatisfiedBy(State))
    {
        return false; // The precondition is unmet.
    }

    if (mEffect.IsEmpty())
    {
        return false; // This action is ineffective.
    }

    return true;
}

void CAction::Rearrange()
{
    mPrecondition.Rearrange();
    mEffect.Rearrange();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
