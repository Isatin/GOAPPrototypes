// Copyright 2026 Isaac Hsu

#include <cassert>

#include "BitVector.h"
#include "Condition.h"
#include "State.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CCondition::CCondition(const CFactDefinition& Definition)
    : mDefinition(Definition)
{}

CBitVector CCondition::GetUsedFactBits() const
{
    return mSimultaneousFormulas.GetUsedFactBits();
}

bool CCondition::IsSatisfiedBy(const CState& State) const
{
    assert(&State.GetDefinition() == &mDefinition);

    if (mSimultaneousFormulas.IsEmpty())
    {
        return true;
    }

    return mSimultaneousFormulas.Evaluate(State) == ETriState::yes;
}

void CCondition::Rearrange()
{ 
    mSimultaneousFormulas.Rearrange(mDefinition); 
}
///////////////////////////////////////////////////////////////////////////////////////////////////

