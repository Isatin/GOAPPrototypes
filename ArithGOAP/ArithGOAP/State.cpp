// Copyright 2024 Isaac Hsu

#include <cassert>
#include <memory>

#include "Fact.h"
#include "Notation.h"
#include "State.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CState::CState(const CFactDefinition& Definition)
    : mDefinition(Definition)
{ 
    Expand(Definition.GetFactCount());
}

void CState::Expand(int Size)
{
    if (Size > mProperties.size())
    {
        mProperties.resize(Size);
    }
}

std::string CState::ToString() const
{
    std::string Return;
    bool Successive = false;

    for (int FactIndex = 0; FactIndex < mProperties.size(); FactIndex++)
    {
        const SSegment& Segment = mProperties[FactIndex];
        if (Segment.IsUnset())
        {
            continue;
        }

        if (Successive)
        {
            Return += ", ";
        }
        else
        {
            Successive = true;
        }

        Return += Segment.Stringize(GetDefinition().GetFact(FactIndex)->GetName());
    }

    return Return;
}

int CState::CountProperties() const
{
    int Count = 0;

    for (const SSegment& Segment : mProperties)
    {
        if (Segment.IsSet())
        {
            Count++;
        }
    }

    return Count;
}

SSegment& CState::GetProperty(int FactIndex)
{
    Expand(FactIndex + 1);
    return mProperties[FactIndex];
}

SSegment& CState::GetProperty(const CFact& Fact)
{
    assert(&Fact.GetOwner() == &mDefinition);

    return GetProperty(Fact.GetIndex());
}

const SSegment& CState::GetProperty(int FactIndex) const
{
    if (FactIndex >= 0 && FactIndex < mProperties.size())
    {
        return mProperties[FactIndex];
    }
    else
    {
        return SSegment::Empty;
    }
}

const SSegment& CState::GetProperty(const CFact& Fact) const
{
    assert(&Fact.GetOwner() == &mDefinition);

    return GetProperty(Fact.GetIndex());
}

bool CState::SetProperty(int FactIndex, const SSegment& Segment)
{
    Expand(FactIndex + 1);
    mProperties[FactIndex] = Segment;
    return true;
}

bool CState::SetProperty(const CFact& Fact, const SSegment& Segment)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SSegment Intersection = Segment;
    if (!Intersection.Intersect(Fact.GetRange(), mDefinition.GetTolerance()))
    {
        return false;
    }

    return SetProperty(Fact.GetIndex(), Intersection);
}

bool CState::SetProperty(const CFact& Fact, CNumber Value)
{
    return SetProperty(Fact, SSegment(Value, Value));
}

bool CState::SetProperty(const SFactEquation& Equation)
{
    return SetProperty(Equation.Subject, SSegment(Equation.Value, Equation.Value));
}

bool CState::SetProperty(const SNumericFactRange& Range)
{
    return SetProperty(Range.Subject, SSegment(Range.Minimum, Range.Maximum));
}

float CState::GetBaseHeuristicCost(const CState& Another) const 
{ 
    return static_cast<float>(mDefinition.GetHeuristicCost(*this, Another));
}

bool CState::IsContradictory(const CState& Another) const
{
    assert(&Another.GetDefinition() == &mDefinition);

    const CNumber Tolerance = GetDefinition().GetTolerance();
    for (int FactIndex = 0; FactIndex < mProperties.size(); FactIndex++)
    {
        const SSegment& Source = mProperties[FactIndex];
        if (Source.IsUnset())
        {
            continue;
        }

        const SSegment& Other = Another.GetProperty(FactIndex);
        if (Other.IsUnset())
        {
            return true;
        }

        if (!Source.HasIntersection(Other, Tolerance))
        {
            return true;
        }
    }

    return false;
}

void CState::Clamp() 
{ 
    mDefinition.Clamp(*this); 
}
///////////////////////////////////////////////////////////////////////////////////////////////////
