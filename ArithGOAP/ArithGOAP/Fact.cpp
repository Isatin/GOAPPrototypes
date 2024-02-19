// Copyright 2024 Isaac Hsu

#include <cassert>
#include <sstream>

#include "Fact.h"
#include "Notation.h"
#include "State.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CFact::CFact(CStateDefinition& Owner, int Index, const std::string& Name, EFactType Type, const SInterval& Range, const CHeuristicFunctor& HeuristicFunctor)
    : mOwner(Owner)
    , mIndex(Index)
    , mName(Name)
    , mType(Type)
    , mRange(Range)
    , mHeuristicFunctor(HeuristicFunctor)
{}

void CFact::SetDistanceWeight(SNumber DistanceWeight)
{
    if (CHeuristicFunctor HeuristicFunctor = mOwner.GenerateHeuristicFunctor(mType, DistanceWeight))
    {
        SetHeuristicFunctor(HeuristicFunctor);
    }
}

SFactRange CFact::operator == (SNumber Value) const
{
    return {*this, Value, Value};
}

SFactOperation CFact::operator = (SNumber Value) const
{
    return {*this, EOperator::Assign, Value};
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CBooleanFact::CBooleanFact(CStateDefinition& Owner, int Index, const std::string& Name, EFactType Type, const SInterval& Range, const CHeuristicFunctor& HeuristicFunctor)
    : CFact(Owner, Index, Name, Type, Range, HeuristicFunctor)
{}

SBooleanFactOperation CBooleanFact::operator = (SNumber Value) const
{
    return {*this, EOperator::Assign, Value};
}

SBooleanFactOperation CBooleanFact::operator ! () const
{
    return {*this, EOperator::Negation, 0};
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CNumericFact::CNumericFact(CStateDefinition& Owner, int Index, const std::string& Name, EFactType Type, const SInterval& Range, const CHeuristicFunctor& HeuristicFunctor)
    : CFact(Owner, Index, Name, Type, Range, HeuristicFunctor)
{}

SNumericFactRange CNumericFact::operator == (SNumber Value) const
{
    return {*this, Value, Value};
}

SNumericFactRange CNumericFact::operator <= (SNumber Value) const
{
    return {*this, -SNumber::Infinity, Value};
}

SNumericFactRange CNumericFact::operator >= (SNumber Value) const
{
    return {*this, Value, SNumber::Infinity};
}

SNumericFactRange ArithGOAP::operator <= (SNumber Value, const CNumericFact& Fact)
{
    return {Fact, Value, SNumber::Infinity};
}

SNumericFactRange ArithGOAP::operator >= (SNumber Value, const CNumericFact& Fact)
{
    return {Fact, -SNumber::Infinity, Value};
}

SNumericFactOperation CNumericFact::operator = (SNumber Value) const
{
    return {*this, EOperator::Assign, Value};
}

SNumericFactOperation CNumericFact::operator += (SNumber Value) const
{
    return {*this, EOperator::Addition, Value};
}

SNumericFactOperation CNumericFact::operator -= (SNumber Value) const
{
    return {*this, EOperator::Addition, -Value};
}

SNumericFactOperation CNumericFact::operator *= (SNumber Value) const
{
    return {*this, EOperator::Multiplication, Value};
}

SNumericFactOperation CNumericFact::operator /= (SNumber Value) const
{
    return {*this, EOperator::Multiplication, 1 / Value};
}

SNumericFactOperation CNumericFact::operator - () const
{
    return {*this, EOperator::Multiplication, -1};
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string CStateDefinition::StringizeBoundedRanges() const
{
    std::stringstream Stream;
    bool First = true;

    for (const auto& Fact : mFacts)
    {
        if (!Fact->GetRange().IsBounded())
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

        Stream << Fact->GetRange().ToString(Fact->GetName());
    }

    return Stream.str();
}

bool CStateDefinition::ValidateDefinitionParameters(const std::string& Name, EFactType Type, SInterval& Range, CHeuristicFunctor& HeuristicFunctor)
{
    assert(Type == EFactType::boolean || Type == EFactType::enumeration || Type == EFactType::number);

    if (mNameMap.find(Name) != mNameMap.end()) // Redifinition is disallowed.
    {
        return false;
    }

    if (!Range)
    {
        Range = SInterval::Boundless; // Replace unset range with boundless interval
    }

    if (Range.IsEmpty())
    {
        return false; // Invalid range
    }

    if (!HeuristicFunctor)
    {
        HeuristicFunctor = GenerateHeuristicFunctor(Type, 1); // Replace unset distance functor with default one
    }

    return true;
}

CBooleanFact* CStateDefinition::DefineBoolean(const std::string& Name)
{
    return Define<CBooleanFact>(Name, EFactType::boolean);
}

CFact* CStateDefinition::DefineEnumeration(const std::string& Name, const SInterval& Range)
{
    return Define<CFact>(Name, EFactType::enumeration, Range);
}

CNumericFact* CStateDefinition::DefineNumber(const std::string& Name, const SInterval& Range, SNumber DistanceWeight)
{
    CHeuristicFunctor HeuristicFunctor = GenerateHeuristicFunctor(EFactType::number, DistanceWeight);
    return Define<CNumericFact>(Name, EFactType::number, Range, HeuristicFunctor);
}

CNumericFact* CStateDefinition::DefineNumber(const std::string& Name, SNumber DistanceWeight, const SInterval& Range)
{
    return DefineNumber(Name, Range, DistanceWeight);
}

CNumericFact* CStateDefinition::DefineNumber(const SVariableRange& Range, SNumber DistanceWeight)
{
    return DefineNumber(Range.Subject.Name, SInterval(Range.Minimum, Range.Maximum), DistanceWeight);
}

CNumericFact* CStateDefinition::DefineNumber(const SVariableRange& Range, const CHeuristicFunctor& HeuristicFunctor)
{
    return Define<CNumericFact>(Range.Subject.Name, EFactType::number, SInterval(Range.Minimum, Range.Maximum), HeuristicFunctor);
}

CHeuristicFunctor CStateDefinition::GenerateHeuristicFunctor(EFactType Type, SNumber DistanceWeight)
{
    const SNumber BaseCost = mBaseCost;

    switch (Type)
    {
    case EFactType::boolean:
    case EFactType::enumeration:
        return [BaseCost](const SInterval& Source, const SInterval& Aim) 
        {
            return Source.HasIntersection(Aim) ? SNumber(0) : BaseCost; 
        };

    case EFactType::number:
        return [BaseCost, DistanceWeight](const SInterval& Source, const SInterval& Aim)
        { 
            SNumber Result = Source.GetGap(Aim);
            if (Result != 0)
            {
                Result *= DistanceWeight;
                Result += BaseCost;
            }
            return Result;
        };
    }

    return CHeuristicFunctor();
}

const CFact* CStateDefinition::GetFact(int Index) const
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

const CFact* CStateDefinition::GetFact(const std::string& Name) const
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

SNumber CStateDefinition::GetHeuristicCost(const CState& SourceState, const CState& DesiredState) const
{
    assert(&SourceState.GetDefinition() == this);
    assert(&DesiredState.GetDefinition() == this);

    SNumber Result(0);

    for (const auto& Fact : mFacts)
    {
        const int Index = Fact->GetIndex();
        const SInterval& Source = SourceState.GetFact(Index);
        if (!Source)
        {
            continue;
        }

        if (const SInterval& Aim = DesiredState.GetFact(Index))
        {
            Result += Fact->GetHeuristicFunctor()(Source, Aim);
        }
        else
        {
            Result += mBaseCost;
        }
    }

    return Result;
}

void CStateDefinition::Clamp(CState& State) const
{
    for (const auto& Fact : mFacts)
    {
        if (SInterval& Target = State.GetFact(Fact->GetIndex()))
        {
            Fact->GetRange().Clamp(Target);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////