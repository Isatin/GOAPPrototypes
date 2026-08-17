// Copyright 2025 Isaac Hsu

#include <cassert>

#include "Condition.h"
#include "Fact.h"
#include "State.h"


using namespace AlgebGOAP;
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
        mProperties.resize(Size, CNumber::Null);
    }
}

int CStateBase::CountProperties() const
{
    int Count = 0;

    for (CNumber Value : mProperties)
    {
        if (!Value.IsNull())
        {
            Count++;
        }
    }

    return Count;
}

CNumber CStateBase::GetProperty(const CFact& Fact) const
{
    assert(&Fact.GetOwner() == &mDefinition);

    return GetProperty(Fact.GetIndex());
}

CNumber CStateBase::GetProperty(int FactIndex) const
{
    if (FactIndex >= 0 && FactIndex < mProperties.size())
    {
        return mProperties[FactIndex];
    }
    else
    {
        return CNumber::Null;
    }
}

void CStateBase::SetProperty(const CBooleanFact& Fact, bool Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), Value);
}

void CStateBase::SetProperty(const CEnumerationFact& Fact, int Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), Value);
}

void CStateBase::SetProperty(const CNumericFact& Fact, CNumber Value)
{
    assert(&Fact.GetOwner() == &mDefinition);

    SetProperty(Fact.GetIndex(), Value);
}

void CStateBase::SetProperty(int FactIndex, CNumber Value)
{
    if (FactIndex < 0)
    {
        return;
    }

    Expand(FactIndex + 1);
    mProperties[FactIndex] = Value;
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
        CNumber Value = mProperties[FactIndex];
        if (Value.IsNull())
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
        Return += Value.ToString();
    }

    return Return;
}

float CState::GetBaseHeuristicCost(const CCondition& Condition, SDebugInfo* DebugInfo) const
{
    return static_cast<float>(Condition.GetConstraint().GetHeuristicCost(*this, DebugInfo));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
