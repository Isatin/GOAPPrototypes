// Copyright 2024 Isaac Hsu

#include <cassert>

#include "Fact.h"
#include "Notation.h"
#include "State.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CFact::CFact(CFactDefinition& Owner, int Index, const std::string& Name, EFactType Type, const SSegment& Range, const CHeuristicFunctor& HeuristicFunctor)
    : mOwner(Owner)
    , mIndex(Index)
    , mName(Name)
    , mType(Type)
    , mRange(Range)
    , mHeuristicFunctor(HeuristicFunctor)
{}

void CFact::SetGapWeight(CNumber GapWeight)
{
    if (CHeuristicFunctor HeuristicFunctor = mOwner.GenerateHeuristicFunctor(mType, GapWeight))
    {
        SetHeuristicFunctor(HeuristicFunctor);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CBooleanFact::CBooleanFact(CFactDefinition& Owner, int Index, const std::string& Name, const SSegment& Range, const CHeuristicFunctor& HeuristicFunctor)
    : CFact(Owner, Index, Name, EFactType::boolean, Range, HeuristicFunctor)
{}

SFactOperation CBooleanFact::operator = (bool Value) const
{
    return SFactOperation(*this, EOperator::assignment, Value);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CEnumerationFact::CEnumerationFact(CFactDefinition& Owner, int Index, const std::string& Name, const SSegment& Range, const CHeuristicFunctor& HeuristicFunctor)
    : CFact(Owner, Index, Name, EFactType::enumeration, Range, HeuristicFunctor)
{}

SFactOperation CEnumerationFact::operator = (int Value) const
{
    return SFactOperation(*this, EOperator::assignment, Value);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CNumericFact::CNumericFact(CFactDefinition& Owner, int Index, const std::string& Name, const SSegment& Range, const CHeuristicFunctor& HeuristicFunctor)
    : CFact(Owner, Index, Name, EFactType::number, Range, HeuristicFunctor)
{}

SFactOperation CNumericFact::operator = (CNumber Value) const
{
    return {*this, EOperator::assignment, Value};
}

SFactOperation CNumericFact::operator += (CNumber Value) const
{
    return {*this, EOperator::addition, Value};
}

SFactOperation CNumericFact::operator -= (CNumber Value) const
{
    return {*this, EOperator::addition, -Value};
}

SFactOperation CNumericFact::operator *= (CNumber Value) const
{
    return {*this, EOperator::multiplication, Value};
}

SFactOperation CNumericFact::operator /= (CNumber Value) const
{
    return {*this, EOperator::multiplication, 1 / Value};
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CFactDefinition::CFactDefinition(CNumber BaseRelationCost, CNumber Tolerance)
    : mBaseRelationCost(BaseRelationCost)
    , mTolerance(Tolerance)
{
    assert(BaseRelationCost > 0);
    assert(Tolerance >= 0);
}

bool CFactDefinition::HasAnyRange() const
{
    for (const auto& Fact : mFacts)
    {
        if (Fact->GetRange().IsAnyBounded())
        {
            return true;
        }
    }

    return false;
}

std::string CFactDefinition::StringizeBoundedRanges() const
{
    std::string Return;
    bool Successive = false;

    for (const auto& Fact : mFacts)
    {
        if (!Fact->GetRange().IsAnyBounded())
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

        Return += Fact->GetRange().Stringize(Fact->GetName());
    }

    return Return;
}

bool CFactDefinition::ValidateDefinitionParameters(const std::string& Name, EFactType Type, SSegment& Range, CHeuristicFunctor& HeuristicFunctor)
{
    if (mNameMap.find(Name) != mNameMap.end())
    {
        return false; // Redifinition is disallowed.
    }

    if (Range.IsUnset())
    {
        Range = SSegment::Boundless; // Replace the unset range with an unbounded segment.
    }

    if (Range.IsEmpty())
    {
        return false; // Invalid range
    }

    if (!HeuristicFunctor)
    {
        HeuristicFunctor = GenerateHeuristicFunctor(Type, 1); // Replace the unset distance functor with a default one.
    }

    return true;
}

CBooleanFact* CFactDefinition::DefineBoolean(const std::string& Name)
{
    return Define<CBooleanFact>(Name);
}

CEnumerationFact* CFactDefinition::DefineEnumeration(const std::string& Name, const SSegment& Range)
{
    return Define<CEnumerationFact>(Name, Range);
}

CNumericFact* CFactDefinition::DefineNumber(const std::string& Name, const SSegment& Range, CNumber GapWeight)
{
    CHeuristicFunctor HeuristicFunctor = GenerateHeuristicFunctor(EFactType::number, GapWeight);
    return Define<CNumericFact>(Name, Range, HeuristicFunctor);
}

CNumericFact* CFactDefinition::DefineNumber(const std::string& Name, CNumber GapWeight, const SSegment& Range)
{
    return DefineNumber(Name, Range, GapWeight);
}

CNumericFact* CFactDefinition::DefineNumber(const SVariableRange& Range, CNumber GapWeight)
{
    return DefineNumber(Range.Subject.Name, SSegment(Range.Minimum, Range.Maximum), GapWeight);
}

CNumericFact* CFactDefinition::DefineNumber(const SVariableRange& Range, const CHeuristicFunctor& HeuristicFunctor)
{
    return Define<CNumericFact>(Range.Subject.Name, SSegment(Range.Minimum, Range.Maximum), HeuristicFunctor);
}

CHeuristicFunctor CFactDefinition::GenerateHeuristicFunctor(EFactType Type, CNumber GapWeight)
{
    assert(GapWeight > 0.f);
    assert(GapWeight.IsFinite());

    switch (Type)
    {
    case EFactType::boolean:
    case EFactType::enumeration:
        return [&](const SSegment& Source, const SSegment& Target)
        {
            return Source.HasIntersection(Target, mTolerance) ? 0_n : mBaseRelationCost;
        };

    case EFactType::number:
        return [&, GapWeight](const SSegment& Source, const SSegment& Target)
        { 
            CNumber Gap = Source.GetGap(Target, mTolerance);
            if (Gap.IsFinite())
            {
                return Gap * GapWeight;
            }
            else
            {
                return mBaseRelationCost;
            }
        };
    }

    assert(!"Invalid fact type");
    return CHeuristicFunctor();
}

const CFact* CFactDefinition::GetFact(int Index) const
{
    if (Index >= 0 && Index < mFacts.size())
    {
        return mFacts[Index].get();
    }
    else
    {
        return nullptr;
    }
}

const CFact* CFactDefinition::GetFact(const std::string& Name) const
{
    auto it = mNameMap.find(Name);
    if (it == mNameMap.end())
    {
        return nullptr;
    }
    else
    {
        return GetFact(it->second);
    }
}

CNumber CFactDefinition::GetHeuristicCost(const CState& SourceState, const CState& DesiredState) const
{
    assert(&SourceState.GetDefinition() == this);
    assert(&DesiredState.GetDefinition() == this);

    CNumber Return = 0;

    for (const auto& Fact : mFacts)
    {
        const int FactIndex = Fact->GetIndex();
        const SSegment& Target = DesiredState.GetProperty(FactIndex);
        if (Target.IsUnset())
        {
            continue;
        }

        const SSegment& Source = SourceState.GetProperty(FactIndex);
        if (Source.IsSet())
        {
            Return += Fact->GetHeuristicFunctor()(Source, Target);
        }
        else
        {
            Return += mBaseRelationCost;
        }
    }

    return Return;
}

void CFactDefinition::Clamp(CState& State) const
{
    for (const auto& Fact : mFacts)
    {
        SSegment& Target = State.GetProperty(Fact->GetIndex());
        if (Target.IsSet())
        {
            Fact->GetRange().Clamp(Target);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////