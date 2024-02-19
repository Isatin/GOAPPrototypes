// Copyright 2024 Isaac Hsu

#include <cassert>
#include <sstream>

#include "Fact.h"
#include "Notation.h"
#include "State.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CState::CState(const CStateDefinition& Def)
    : mDefinition(Def)
{ 
    Expand(Def.GetFactCount());
}

CState::CState(const CStateDefinition& Def, std::vector<SInterval>&& Facts)
    : mDefinition(Def)
    , mValues(Facts)
{
}

void CState::Expand(int Size)
{
    if (Size > mValues.size())
    {
        mValues.resize(Size);
    }
}

std::unique_ptr<CState> CState::Clone() const
{ 
    auto Values = mValues;
    return Clone(std::move(Values));
}

std::string CState::ToString() const
{
    std::stringstream Stream;
    bool First = true;

    for (int i = 0; i < mValues.size(); i++)
    {
        const SInterval& Interval = mValues[i];
        if (!Interval)
        {
            continue;
        }

        if (First)
        {
            First = false;
        }
        else
        {
            Stream << ", ";
        }

        Stream << Interval.ToString(GetDefinition().GetFact(i)->GetName());
    }

    return Stream.str();
}

SInterval& CState::GetFact(int Index)
{
    Expand(Index + 1);
    return mValues[Index];
}

SInterval& CState::GetFact(const CFact& Fact)
{
    assert(&Fact.GetOwner() == &mDefinition);

    return GetFact(Fact.GetIndex());
}

const SInterval& CState::GetFact(int Index) const
{
    if (Index < mValues.size())
    {
        return mValues[Index];
    }
    else
    {
        return SInterval::Empty;
    }
}

const SInterval& CState::GetFact(const CFact& Fact) const
{
    assert(&Fact.GetOwner() == &mDefinition);

    return GetFact(Fact.GetIndex());
}

bool CState::SetFact(int Index, const SInterval& Interval)
{
    Expand(Index + 1);
    mValues[Index] = Interval;
    return true;
}

bool CState::SetFact(const CFact& Fact, const SInterval& Interval)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SInterval Intersection = Interval;
    if (!Intersection.Intersect(Fact.GetRange()))
    {
        return false;
    }

    return SetFact(Fact.GetIndex(), Intersection);
}

bool CState::SetFact(const CFact& Fact, SNumber Value)
{
    return SetFact(Fact, SInterval(Value, Value));
}

bool CState::SetFact(const SFactRange& Range)
{
    return SetFact(Range.Subject, SInterval(Range.Minimum, Range.Maximum));
}

bool CState::SetFact(const SNumericFactRange& Range)
{
    return SetFact(Range.Subject, SInterval(Range.Minimum, Range.Maximum));
}

float CState::GetBasicHeuristicCost(const CState& Another) const 
{ 
    return (float) mDefinition.GetHeuristicCost(*this, Another); 
}

bool CState::HasIntersection(const CState& Another) const
{
    assert(&Another.GetDefinition() == &mDefinition);

    for (int i = 0; i < mValues.size(); i++)
    {
        const SInterval& Source = mValues[i];
        if (!Source)
        {
            continue;
        }

        const SInterval& Other = Another.GetFact(i);
        if (!Other)
        {
            return false;
        }

        if (!Source.HasIntersection(Other))
        {
            return false;
        }
    }

    return true;
}

void CState::Clamp() 
{ 
    mDefinition.Clamp(*this); 
}
///////////////////////////////////////////////////////////////////////////////////////////////////
