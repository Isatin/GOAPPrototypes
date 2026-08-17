// Copyright 2026 Isaac Hsu

#include <cassert>

#include "Condition.h"
#include "Fact.h"
#include "Set.h"
#include "State.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
static_assert(!std::is_polymorphic_v<CStateBase>);

CStateBase::CStateBase(const CFactDefinition& Definition)
    : mDefinition(Definition)
{
    Expand(Definition.GetFactCount());
}

void CStateBase::Expand(int Size)
{
    if (Size > mProperties.size())
    {
        mProperties.resize(Size);
    }
}

int CStateBase::CountProperties() const
{
    int Count = 0;

    for (const CVariant& Property : mProperties)
    {
        if (!Property.IsUnset())
        {
            Count++;
        }
    }

    return Count;
}

const CVariant& CStateBase::GetProperty(const CFact& Fact) const
{
    assert(&Fact.GetOwner() == &mDefinition);

    return GetProperty(Fact.GetIndex());
}

const CVariant& CStateBase::GetProperty(int FactIndex) const
{
    if (FactIndex >= 0 && FactIndex < mProperties.size())
    {
        return mProperties[FactIndex];
    }
    else
    {
        return CVariant::Unset;
    }
}

void CStateBase::SetProperty(const CBooleanFact& Fact, bool Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), CVariant(Value));
}

void CStateBase::SetProperty(const CEnumerationFact& Fact, int Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), CVariant(Value));
}

void CStateBase::SetProperty(const CNumericFact& Fact, CNumber Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), CVariant(Value));
}

void CStateBase::SetProperty(const CMatrixFact& Fact, const CMatrix& Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), CVariant(Value));
}

void CStateBase::SetProperty(const CMatrixFact& Fact, CMatrix&& Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), std::move(Value));
}

void CStateBase::SetProperty(const CSetFact& Fact, const CSet& Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), CVariant(Value));
}

void CStateBase::SetProperty(const CSetFact& Fact, CSet&& Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), std::move(Value));
}

void CStateBase::SetProperty(int FactIndex, CVariant&& Value)
{
    assert(Value.GetVariantType().IsCompatible(mDefinition.GetFact(FactIndex)->GetType()));

    if (FactIndex < 0)
    {
        return;
    }

    Expand(FactIndex + 1);
    mProperties[FactIndex] = std::move(Value);
}

CSet CStateBase::GetUnionSet() const 
{ 
    CSet UniversalSet;

    for (const CVariant& Property : mProperties)
    {
        if (Property.IsSet())
        {
            UniversalSet.Union(Property.GrabSet());
        }
    }

    return UniversalSet;
}

CSet CStateBase::GetUnionSetIC() const
{
    CSet UniversalSet;

    for (const CVariant& Property : mProperties)
    {
        if (Property.IsSet())
        {
            UniversalSet.UnionIC(Property.GrabSet());
        }
    }

    return UniversalSet;
}

void CStateBase::ConvertComplementSets(const CSet& UniversalSet)
{
    for (CVariant& Property : mProperties)
    {
        if (!Property.IsSet())
        {
            continue;
        }

        CSet& Set = Property.GrabSet();
        if (!Set.IsComplement())
        {
            continue;
        }

        Set.SetComplement(false);
        Set = UniversalSet.GetDifference(Set);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CState::CState(const CFactDefinition& Definition)
    : CStateBase(Definition)
{}

std::string CState::ToString() const
{
    std::string Return;
    bool Successive = false;

    for (int FactIndex = 0; FactIndex < mProperties.size(); FactIndex++)
    {
        const CVariant& Property = mProperties[FactIndex];
        if (Property.IsUnset())
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

        Return += GetDefinition().GetFact(FactIndex)->GetName();
        Return += "=";
        Return += Property.ToString();
    }

    return Return;
}

float CState::GetBaseHeuristicCost(const CCondition& Condition, SDebugInfo* DebugInfo) const
{
    return static_cast<float>(Condition.GetConstraint().GetHeuristicCost(*this, DebugInfo));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
