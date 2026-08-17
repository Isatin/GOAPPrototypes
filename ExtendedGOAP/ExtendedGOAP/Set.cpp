// Copyright 2026 Isaac Hsu

#include <algorithm>
#include <cassert>
#include <span>

#include "Set.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const CSet CSet::Empty;
const CSet CSet::InvalidType(EVariantError::invalidType);

CSet::~CSet()
{
    DestructEntries();
}

CSet::CSet(EVariantError Error)
{
    mType = EVariantType::set;
    mError = Error;
    mEntryCount = 0;
    mComplement = false;
    ConstructEntries();
}

CSet::CSet(int Capacity)
{
    assert(Capacity >= 0 && Capacity <= std::numeric_limits<BEntrySize>::max());

    mType = EVariantType::set;
    mError = EVariantError::none;
    mEntryCount = 0;
    mComplement = false;
    ConstructEntries();
    ResizeEntries(Capacity);
}

CSet::CSet(const CSet& Another)
{
    mType = Another.mType;
    mError = Another.mError;
    mEntryCount = Another.mEntryCount;
    mComplement = Another.mComplement;

    ConstructEntries();
    ResizeEntries(Another.mCapacity);
    std::ranges::copy(std::span(Another.mEntries.get(), Another.mEntryCount), mEntries.get());
}

CSet::CSet(CSet&& Another) noexcept
{
    mType = Another.mType;
    mError = Another.mError;
    mEntryCount = Another.mEntryCount;
    mComplement = Another.mComplement;

    ConstructEntries();
    mEntries = std::move(Another.mEntries);
    mCapacity = Another.mCapacity;
}

CSet::CSet(std::initializer_list<CNumber> Source)
{
    // NaN is unordered, so it's not allowed to be in the ordered set.
    assert(std::ranges::all_of(Source, [](CNumber Value){ return !Value.IsNaN(); }));

    mType = EVariantType::set;
    mError = EVariantError::none;
    mEntryCount = static_cast<BEntrySize>(Source.size());
    mComplement = false;

    ConstructEntries();
    ResizeEntries(mEntryCount);
    std::ranges::copy(Source, mEntries.get());

    // Sort the numbers and remove duplicates.
    std::ranges::sort(std::span(mEntries.get(), mEntryCount));
    auto itEnd = std::unique(mEntries.get(), mEntries.get() + mEntryCount);
    mEntryCount = static_cast<int>(itEnd - mEntries.get());
    assert(mEntryCount >= 0);
}

CSet& CSet::operator = (const CSet& Another)
{
    mType = Another.mType;
    mError = Another.mError;
    mEntryCount = Another.mEntryCount;
    mComplement = Another.mComplement;

    ConstructEntries();
    ResizeEntries(Another.mCapacity);
    std::ranges::copy(std::span(Another.mEntries.get(), Another.mEntryCount), mEntries.get());
    return *this;
}

CSet& CSet::operator = (CSet&& Another) noexcept
{
    mType = Another.mType;
    mError = Another.mError;
    mEntryCount = Another.mEntryCount;
    mComplement = Another.mComplement;

    ConstructEntries();
    mEntries = std::move(Another.mEntries);
    mCapacity = Another.mCapacity;
    return *this;
}

CSet ExtendedGOAP::operator ~ (const CSet& Argument) 
{
    CSet Complement = Argument;
    Complement.SetComplement(!Complement.IsComplement());
    return Complement;
}

CSet ExtendedGOAP::operator ~ (CSet&& Argument)
{
    Argument.SetComplement(!Argument.IsComplement());
    return std::move(Argument);
}

CNumber& CSet::operator [] (int Index)
{    
    assert(Index >= 0 && Index < mEntryCount);

    return mEntries[Index];
}

CNumber CSet::operator [] (int Index) const
{
    assert(Index >= 0 && Index < mEntryCount);

    return mEntries[Index];
}

CNumber* CSet::begin()
{
    if (mCapacity <= 0)
    {
        return nullptr;
    }
    else
    {
        return mEntries.get();
    }
}

CNumber* CSet::end()
{
    if (mCapacity <= 0)
    {
        return nullptr;
    }
    else
    {
        assert(mEntryCount <= mCapacity);
        return mEntries.get() + mEntryCount;
    }
}

std::string CSet::ToString() const
{
    if (IsEmpty())
    {
        return GetEmptySymbol();
    }
    else if (IsUniversalSet())
    {
        return GetUniversalSetSymbol();
    }

    std::string Return = "{";

    bool Successive = false;
    for (CNumber Entry : *this)
    {
        if (Successive)
        {
            Return += ",";
        }
        else
        {
            Successive = true;
        }

        Return += Entry.ToString();
    }

    Return += "}";

    if (mComplement)
    {
#ifdef USE_UNICODE_SYMBOLS
        Return += "ᶜ";
#else
        Return += "′";
#endif
    }
    
    return Return;
}

bool CSet::IsEqual(const CSet& Another) const
{
    if (IsComplement() != Another.IsComplement())
    {
        return false;
    }

    return std::ranges::equal(*this, Another, CNumber::IsSame);
}

bool CSet::IsAlmostEqual(const CSet& Another, CNumber Tolerance) const
{
    if (IsComplement() != Another.IsComplement())
    {
        return false;
    }

    return std::ranges::equal(*this, Another, [Tolerance](CNumber Left, CNumber Right){ return Left.IsAlmostEqual(Right, Tolerance); });
}

size_t CSet::GetSize() const
{
    if (mComplement)
    {
        assert(GetMaxSize() >= mEntryCount);        
        return GetMaxSize() - mEntryCount;
    }
    else
    {
        return mEntryCount;
    }
}

size_t CSet::GetComplementSize() const
{
    if (mComplement)
    {
        return mEntryCount;
    }
    else
    {
        assert(GetMaxSize() >= mEntryCount);
        return GetMaxSize() - mEntryCount;
    }
}

CNumber CSet::GetMinDistance(CNumber Testee) const
{
    CNumber MinDist = GetMinDistanceIC(Testee);
    if (IsComplement())
    {
        return MinDist.Get() > 0 ? 0_n : CNumber::Null;
    }
    else
    {
        return MinDist;
    }
};

CNumber CSet::GetMinDistanceIC(CNumber Testee) const
{
    CNumber MinDist = CNumber::Null;

    for (CNumber Entry : *this)
    {
        CNumber Dist = std::abs(Entry - Testee);
        if (MinDist.IsNull() || MinDist > Dist)
        {
            MinDist = Dist;
        }
    }

    return MinDist;
}

bool CSet::Contain(CNumber Testee) const
{
    bool Found = std::ranges::binary_search(*this, Testee);
    if (IsComplement())
    {
        return !Found;
    }
    else
    {
        return Found;
    }
}

bool CSet::Contain(const CSet& Another) const
{
    if (IsComplement())
    {
        if (Another.IsComplement())
        {
            return Another.ContainIC(*this); // S'⊇T' ⇔ S⊆T
        }
        else
        {
            return GetIntersectionSizeIC(Another) == 0; // S'⊇T ⇐ S∩T==∅
        }
    }
    else
    {
        if (Another.IsComplement())
        {
            if (GetSize() < Another.GetSize()) // S⊉T' ⇐ |S|≱|T'|
            {
                return false;
            }

            // The following function takes a long time. The preceding if block should return false in most cases.
            return !Another.HasAnyNotIn(*this); // S⊉T' ⇐ ∃x∈T'∧x∉S
        }
        else
        {
            return ContainIC(Another);
        }
    }
}

bool CSet::ContainIC(const CSet& Another) const
{
    return std::ranges::contains_subrange(*this, Another);
}

bool CSet::HasAnyNotIn(const CSet& Another) const
{
    assert(IsComplement());
    assert(!Another.IsComplement());

    for (CNumber Current = -CNumber::Infinity; Current <= CNumber::Infinity; Current = std::nextafter(Current, CNumber::Infinity))
    {
        if (Contain(Current) && !Another.Contain(Current))
        {
            return true;
        }
    }

    return false;
}

bool CSet::ProperContain(const CSet& Another) const
{
    if (IsComplement())
    {
        if (Another.IsComplement())
        {
            return Another.ProperContainIC(*this); // S'⊃T' ⇔ S⊂T
        }
        else
        {
            return GetIntersectionSizeIC(Another) == 0 && GetSize() > Another.GetSize(); // S'⊃T ⇐ S∩T==∅∧|S'|>|T|
        }
    }
    else
    {
        if (Another.IsComplement())
        {
            if (GetSize() <= Another.GetSize()) // S⊅T' ⇐ |S|≯|T'|
            {
                return false;
            }

            // The following function takes a long time. The preceding if block should return false in most cases.
            return !Another.HasAnyNotIn(*this); // S⊅T' ⇐ ∃x∈T'∧x∉S
        }
        else
        {
            return ProperContainIC(Another);
        }
    }
}

bool CSet::ProperContainIC(const CSet& Another) const
{
    return std::ranges::contains_subrange(*this, Another) && mEntryCount > Another.mEntryCount;
}

CSet CSet::GetIntersection(const CSet& Another) const
{
    if (IsComplement())
    {
        if (Another.IsComplement())
        {
            CSet UnionComplement = GetUnionIC(Another); // S'∩T' = (S∪T)'
            UnionComplement.SetComplement(true);
            return UnionComplement;
        }
        else
        {
            return Another.GetDifferenceIC(*this); // S'∩T = T∖S
        }
    }
    else
    {
        if (Another.IsComplement())
        {
            return GetDifferenceIC(Another); // S∩T' = S∖T
        }
        else
        {
            return GetIntersectionIC(Another);
        }
    }
}

CSet CSet::GetIntersectionIC(const CSet& Another) const
{
    CSet Intersection(mEntryCount);
    auto itEnd = std::set_intersection(begin(), end(), Another.begin(), Another.end(), Intersection.begin());
    Intersection.mEntryCount = static_cast<BEntrySize>(itEnd - Intersection.begin());
    assert(Intersection.mEntryCount >= 0);
    return Intersection;
}

int CSet::GetIntersectionSizeIC(const CSet& Another) const
{
    int Count = 0;
    for (int ThisIndex = 0, ThatIndex = 0; ThisIndex < mEntryCount && ThatIndex < Another.mEntryCount;)
    {
        CNumber ThisEntry = mEntries[ThisIndex];
        CNumber ThatEntry = Another.mEntries[ThatIndex];
        if (ThisEntry < ThatEntry)
        {
            ThisIndex++;
        }
        else if (ThisEntry == ThatEntry)
        {
            Count++;
            ThisIndex++;
            ThatIndex++;
        }
        else
        {
            ThatIndex++;
        }
    }
    
    return Count;
}

bool CSet::IsIntersectionEmptyIC(const CSet& Another) const
{
    for (int ThisIndex = 0, ThatIndex = 0; ThisIndex < mEntryCount && ThatIndex < Another.mEntryCount;)
    {
        CNumber ThisEntry = mEntries[ThisIndex];
        CNumber ThatEntry = Another.mEntries[ThatIndex];
        if (ThisEntry < ThatEntry)
        {
            ThisIndex++;
        }
        else if (ThisEntry == ThatEntry)
        {
            return false;
        }
        else
        {
            ThatIndex++;
        }
    }

    return true;
}

CSet CSet::GetUnion(const CSet& Another) const
{
    if (IsComplement())
    {
        if (Another.IsComplement())
        {
            CSet IntersectionComplement = GetIntersectionIC(Another); // S'∪T' = (S∩T)'
            IntersectionComplement.SetComplement(true);
            return IntersectionComplement;
        }
        else
        {
            CSet DifferenceComplement = GetDifferenceIC(Another); // S'∪T = (S∖T)'
            DifferenceComplement.SetComplement(true);
            return DifferenceComplement;
        }
    }
    else
    {
        if (Another.IsComplement())
        {
            CSet DifferenceComplement = Another.GetDifferenceIC(*this); // S∪T' = (T∖S)'
            DifferenceComplement.SetComplement(true);
            return DifferenceComplement;
        }
        else
        {
            return GetUnionIC(Another);
        }
    }
}

CSet CSet::GetUnionIC(const CSet& Another) const
{
    CSet Union(mEntryCount + Another.mEntryCount);
    auto itEnd = std::set_union(begin(), end(), Another.begin(), Another.end(), Union.begin());
    Union.mEntryCount = static_cast<BEntrySize>(itEnd - Union.begin());
    assert(Union.mEntryCount >= 0);
    return Union;
}

CSet& CSet::Union(const CSet& Another)
{
    return *this = GetUnion(Another);
}

CSet& CSet::UnionIC(const CSet& Another)
{
    return *this = GetUnionIC(Another);
}

int CSet::GetUnionSizeIC(const CSet& Another) const
{
    int Count = 0;
    int ThisIndex = 0, ThatIndex = 0;
    for (; ThisIndex < mEntryCount && ThatIndex < Another.mEntryCount;)
    {
        CNumber ThisEntry = mEntries[ThisIndex];
        CNumber ThatEntry = Another.mEntries[ThatIndex];
        if (ThisEntry < ThatEntry)
        {
            Count++;
            ThisIndex++;
        }
        else if (ThisEntry == ThatEntry)
        {
            Count++;
            ThisIndex++;
            ThatIndex++;
        }
        else
        {
            Count++;
            ThatIndex++;
        }
    }

    for (; ThisIndex < mEntryCount; ThisIndex++)
    {
        Count++;
    }

    for (; ThatIndex < Another.mEntryCount; ThatIndex++)
    {
        Count++;
    }

    return Count;
}

CSet CSet::GetDifference(const CSet& Another) const
{
    if (IsComplement())
    {
        if (Another.IsComplement())
        {
            return Another.GetDifferenceIC(*this); // S'∖T' = T∖S
        }
        else
        {
            CSet UnionComplement = GetUnionIC(Another); // S'∖T = (S∪T)'
            UnionComplement.SetComplement(true);
            return UnionComplement;
        }
    }
    else
    {
        if (Another.IsComplement())
        {
            return GetIntersectionIC(Another); // S∖T' = S∩T
        }
        else
        {
            return GetDifferenceIC(Another);
        }
    }
}

CSet CSet::GetDifferenceIC(const CSet& Another) const
{
    CSet Difference(mEntryCount);
    auto itEnd = std::set_difference(begin(), end(), Another.begin(), Another.end(), Difference.begin());
    Difference.mEntryCount = static_cast<BEntrySize>(itEnd - Difference.begin());
    assert(Difference.mEntryCount >= 0);
    return Difference;
}

size_t CSet::GetDifferenceSize(const CSet& Another) const
{
    if (IsComplement())
    {
        if (Another.IsComplement())
        {
            return Another.GetDifferenceSizeIC(*this); // |S'∖T'| = |T∖S|
        }
        else
        {
            int UnionSize = GetUnionSizeIC(Another); // |S'∖T| = |(S∪T)'|
            assert(GetMaxSize() >= UnionSize);
            return GetMaxSize() - UnionSize;
        }
    }
    else
    {
        if (Another.IsComplement())
        {
            return GetIntersectionSizeIC(Another); // |S∖T'| = |S∩T|
        }
        else
        {
            return GetDifferenceSizeIC(Another);
        }
    }
}

int CSet::GetDifferenceSizeIC(const CSet& Another) const
{
    int Count = 0;
    int ThisIndex = 0, ThatIndex = 0;
    for (; ThisIndex < mEntryCount && ThatIndex < Another.mEntryCount;)
    {
        CNumber ThisEntry = mEntries[ThisIndex];
        CNumber ThatEntry = Another.mEntries[ThatIndex];
        if (ThisEntry < ThatEntry)
        {
            Count++;
            ThisIndex++;
        }
        else if (ThisEntry == ThatEntry)
        {
            ThisIndex++;
            ThatIndex++;
        }
        else
        {
            ThatIndex++;
        }
    }

    for (; ThisIndex < mEntryCount; ThisIndex++)
    {
        Count++;
    }

    return Count;
}

bool CSet::IsDifferenceEmpty(const CSet& Another) const
{
    if (IsComplement())
    {
        if (Another.IsComplement())
        {
            return Another.IsDifferenceEmptyIC(*this); // |S'∖T'| = |T∖S|
        }
        else
        {
            int UnionSize = GetUnionSizeIC(Another); // |S'∖T| = |(S∪T)'|
            assert(GetMaxSize() >= UnionSize);
            return GetMaxSize() - UnionSize <= 0;
        }
    }
    else
    {
        if (Another.IsComplement())
        {
            return IsIntersectionEmptyIC(Another); // |S∖T'| = |S∩T|
        }
        else
        {
            return IsDifferenceEmptyIC(Another);
        }
    }
}

bool CSet::IsDifferenceEmptyIC(const CSet& Another) const
{
    int ThisIndex = 0, ThatIndex = 0;
    for (; ThisIndex < mEntryCount && ThatIndex < Another.mEntryCount;)
    {
        CNumber ThisEntry = mEntries[ThisIndex];
        CNumber ThatEntry = Another.mEntries[ThatIndex];
        if (ThisEntry < ThatEntry)
        {
            return false;
        }
        else if (ThisEntry == ThatEntry)
        {
            ThisIndex++;
            ThatIndex++;
        }
        else
        {
            ThatIndex++;
        }
    }

    for (; ThisIndex < mEntryCount; ThisIndex++)
    {
        return false;
    }

    return true;
}

CSet CSet::GetSymmetricDifference(const CSet& Another) const
{
    CSet SymmetricDifference = GetSymmetricDifferenceIC(Another);

    if (IsComplement() == Another.IsComplement())
    {
        return SymmetricDifference; // S'△T' = S△T
    }
    else
    {
        SymmetricDifference.SetComplement(true); // S'△T = (S△T)' = S△T'
        return SymmetricDifference;
    }
}

CSet CSet::GetSymmetricDifferenceIC(const CSet& Another) const
{
    CSet Difference(mEntryCount + Another.mEntryCount);
    auto itEnd = std::set_symmetric_difference(begin(), end(), Another.begin(), Another.end(), Difference.begin());
    Difference.mEntryCount = static_cast<BEntrySize>(itEnd - Difference.begin());
    assert(Difference.mEntryCount >= 0);
    return Difference;
}

size_t CSet::GetSymmetricDifferenceSize(const CSet& Another) const
{
    int DifferenceSize = GetSymmetricDifferenceSizeIC(Another);

    if (IsComplement() == Another.IsComplement())
    {
        return DifferenceSize; // |S'△T'| = |S△T|
    }
    else
    {
        assert(GetMaxSize() >= DifferenceSize);
        return GetMaxSize() - DifferenceSize; // |S'△T| = |(S△T)'| = |S△T'|
    }
}

int CSet::GetSymmetricDifferenceSizeIC(const CSet& Another) const
{
    int Count = 0;
    int ThisIndex = 0, ThatIndex = 0;
    for (; ThisIndex < mEntryCount && ThatIndex < Another.mEntryCount;)
    {
        CNumber ThisEntry = mEntries[ThisIndex];
        CNumber ThatEntry = Another.mEntries[ThatIndex];
        if (ThisEntry < ThatEntry)
        {
            Count++;
            ThisIndex++;
        }
        else if (ThisEntry == ThatEntry)
        {
            ThisIndex++;
            ThatIndex++;
        }
        else
        {
            Count++;
            ThatIndex++;
        }
    }

    for (; ThisIndex < mEntryCount; ThisIndex++)
    {
        Count++;
    }

    for (; ThatIndex < Another.mEntryCount; ThatIndex++)
    {
        Count++;
    }

    return Count;
}
///////////////////////////////////////////////////////////////////////////////////////////////////