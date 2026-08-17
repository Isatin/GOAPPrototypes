// Copyright 2025 Isaac Hsu

#include <cassert>

#include "BitVector.h"
#include "Fact.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
EFactType& EFactType::operator |= (Type Right) 
{ 
    mValue = static_cast<Type>(mValue | Right);
    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CFact::CFact(CFactDefinition& Owner, int Index, const std::string& Name, EFactType Type)
    : mOwner(Owner)
    , mIndex(Index)
    , mName(Name)
    , mType(Type)
{}
///////////////////////////////////////////////////////////////////////////////////////////////////
CBooleanFact::CBooleanFact(CFactDefinition& Owner, int Index, const std::string& Name)
    : CFact(Owner, Index, Name, EFactType::boolean)
{}

CFactAssignment CBooleanFact::operator = (bool Right) const
{
    return CFactAssignment(*this, Right);
}

CFactAssignment CBooleanFact::operator = (const CBooleanFact& Right) const
{
    return CFactAssignment(*this, Right);
}

CFactAssignment CBooleanFact::operator = (const CBooleanExpression& Right) const
{
    return CFactAssignment(*this, Right);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CEnumerationFact::CEnumerationFact(CFactDefinition& Owner, int Index, const std::string& Name)
    : CFact(Owner, Index, Name, EFactType::enumeration)
{}

CFactAssignment CEnumerationFact::operator = (int Right) const
{
    return CFactAssignment(*this, Right);
}

CFactAssignment CEnumerationFact::operator = (const CEnumerationFact& Right) const
{
    return CFactAssignment(*this, Right);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CNumericFact::CNumericFact(CFactDefinition& Owner, int Index, const std::string& Name, CNumber GapWeight)
    : CFact(Owner, Index, Name, EFactType::number)
    , mGapWeight(GapWeight)
{
    assert(GapWeight > 0.f);
    assert(GapWeight.IsFinite());
}

void CNumericFact::SetGapWeight(CNumber GapWeight) 
{
    assert(GapWeight > 0.f);
    assert(GapWeight.IsFinite());

    mGapWeight = GapWeight; 
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CFactDefinition::CFactDefinition(CNumber BaseRelationCost, CNumber Tolerance, CNumber GapOffset)
    : mBaseRelationCost(BaseRelationCost)
    , mTolerance(Tolerance)
    , mGapOffset(GapOffset)
    , mEmptyState(*this)
{
    assert(BaseRelationCost > 0);
    assert(Tolerance >= 0);
    assert(GapOffset >= 0);
}

bool CFactDefinition::ValidateDefinition(const std::string& Name)
{
    if (mNameMap.find(Name) != mNameMap.end())
    {
        return false; // Redifinition is disallowed.
    }

    return true;
}

CBooleanFact* CFactDefinition::DefineBoolean(const std::string& Name)
{
    return Define<CBooleanFact>(Name);
}

CEnumerationFact* CFactDefinition::DefineEnumeration(const std::string& Name)
{
    return Define<CEnumerationFact>(Name);
}

CNumericFact* CFactDefinition::DefineNumber(const std::string& Name, CNumber GapWeight)
{
    return Define<CNumericFact>(Name, GapWeight);
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

EFactType CFactDefinition::GetFactTypes(const CBitVector& FactBits) const
{
    EFactType FactTypes = EFactType::none;

    for (int FactIndex = 0; FactIndex < FactBits.GetSize(); FactIndex++)
    {
        if (!FactBits[FactIndex])
        {
            continue;
        }

        const CFact* Fact = GetFact(FactIndex);
        if (!Fact)
        {
            continue;
        }

        FactTypes |= Fact->GetType();
    }

    return FactTypes;
}

std::pair<CNumber, CNumber> CFactDefinition::GetHeuristicCost(CNumber Gap, const CBitVector& FactBits) const
{
    assert(!Gap.IsFinite() || Gap >= 0);

    CNumber Cost = 0;
    CNumber Weight = CNumber::Null;
    EFactType FactTypes = GetFactTypes(FactBits);
    if (FactTypes == EFactType::boolean || FactTypes == EFactType::enumeration) // Check if all the facts are either Boolean or enumerations.
    {
        if (Gap != 0)
        {
            Cost = mBaseRelationCost;
        }
    }
    else
    {
        if (Gap.IsFinite())
        {
            Weight = GetMinGapWeight(FactBits);
            if (Weight.IsFinite())
            {
                Cost = Gap * Weight;
            }
            else
            {
                Cost = Gap;
            }
        }
        else
        {
            // Intermediate effects producing NaN values could still lead to a solution in the regressive search, so a base cost is returned instead.
            Cost = mBaseRelationCost;
        }
    }

    return std::make_pair(Cost, Weight);
}

CNumber CFactDefinition::GetMinGapWeight(const CBitVector& FactBits) const
{
    CNumber MinGapWeight = CNumber::Null;

    for (int FactIndex = 0; FactIndex < FactBits.GetSize(); FactIndex++)
    {
        if (!FactBits[FactIndex])
        {
            continue;
        }

        const CFact* Fact = GetFact(FactIndex);
        if (!Fact)
        {
            continue;
        }

        CNumber CurrGapWeight = Fact->GetGapWeight();
        if (MinGapWeight.IsNull() || MinGapWeight > CurrGapWeight)
        {
            MinGapWeight = CurrGapWeight;
        }
    }

    return MinGapWeight;
}

std::string CFactDefinition::StringizeFactBits(const CBitVector& FactBits, const char* Delimiter) const
{
    std::string Return;

    bool Successive = false;
    for (int FactIndex = 0; FactIndex < FactBits.GetSize(); FactIndex++)
    {
        if (!FactBits[FactIndex])
        {
            continue;
        }

        const CFact* Fact = GetFact(FactIndex);
        if (!Fact)
        {
            continue;
        }

        if (Delimiter)
        {
            if (Successive)
            {
                Return += Delimiter;
            }
            else
            {
                Successive = true;
            }
        }

        Return += Fact->GetName();
    }

    return Return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////