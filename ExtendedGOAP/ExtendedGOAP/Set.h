// Copyright 2026 Isaac Hsu

#pragma once

#include <initializer_list>

#include "Variant.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CSet : public CVariantBase // Ordered set of numbers
    {
    public:
        static const CSet Empty;
        static const CSet InvalidType;

        static constexpr size_t GetMaxSize() { return CNumber::GetOrderedNumberCount(); }
        static constexpr const char* GetEmptySymbol();
        static constexpr const char* GetUniversalSetSymbol();
        static constexpr const char* GetComplementSymbol();

    public:
        ~CSet();
        CSet(EVariantError Error = EVariantError::none);
        explicit CSet(int Capacity);
        CSet(const CSet& Another);
        CSet(CSet&& Another) noexcept;
        CSet(std::initializer_list<CNumber> Source);

        CSet& operator = (const CSet& Another);
        CSet& operator = (CSet&& Another) noexcept;
        // NOTE: If this is a complement set, the excluded members are returned instead of the included ones.
        CNumber& operator [] (int Index);
        CNumber  operator [] (int Index) const;

        // NOTE: If this is a complement set, the excluded members are returned instead of the included ones.
        CNumber* begin();
        CNumber* end();
        const CNumber* begin() const    { return const_cast<CSet*>(this)->begin(); }
        const CNumber* end() const      { return const_cast<CSet*>(this)->end(); }

        bool IsComplement() const { return mComplement; }
        void SetComplement(bool Value) { mComplement = Value; }

        std::string ToString() const;
        bool IsEmpty() const { return mEntryCount <= 0 && !mComplement; }
        bool IsUniversalSet() const { return mEntryCount <= 0 && mComplement; }
        bool IsEqual(const CSet& Another) const;
        bool IsAlmostEqual(const CSet& Another, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        bool Contain(CNumber Testee) const;
        bool Contain(const CSet& Another) const;
        bool ProperContain(const CSet& Another) const;
        bool IsDifferenceEmpty(const CSet& Another) const;
        CNumber GetMinDistance(CNumber Testee) const;
        CNumber GetMinDistanceIC(CNumber Testee) const;
        CSet GetIntersection(const CSet& Another) const;
        CSet GetUnion(const CSet& Another) const;
        CSet& Union(const CSet& Another);
        CSet& UnionIC(const CSet& Another);
        CSet GetDifference(const CSet& Another) const;
        CSet GetSymmetricDifference(const CSet& Another) const;
        int GetEntryCount() const { return mEntryCount; }
        size_t GetSize() const;
        size_t GetComplementSize() const;
        size_t GetDifferenceSize(const CSet& Another) const;
        size_t GetSymmetricDifferenceSize(const CSet& Another) const;

    private:
        // Are there any members in this set not in anthoer given set?
        // PREREQUISITE: This set has to be a complement, while the given set has to be a non-complement. This isn't really necessary, but there are better ways to handle the other cases.
        // WARNING: This function may take a while.
        bool HasAnyNotIn(const CSet& Another) const;
        // IC stands for "ignore complement".
        bool ContainIC(const CSet& Another) const;
        bool ProperContainIC(const CSet& Another) const;
        CSet GetIntersectionIC(const CSet& Another) const;
        CSet GetUnionIC(const CSet& Another) const;
        CSet GetDifferenceIC(const CSet& Another) const;
        CSet GetSymmetricDifferenceIC(const CSet& Another) const;
        int GetIntersectionSizeIC(const CSet& Another) const;
        int GetUnionSizeIC(const CSet& Another) const;
        int GetDifferenceSizeIC(const CSet& Another) const;
        int GetSymmetricDifferenceSizeIC(const CSet& Another) const;
        bool IsIntersectionEmptyIC(const CSet& Another) const;
        bool IsDifferenceEmptyIC(const CSet& Another) const;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////    
    CSet operator ~ (const CSet& Argument); // Return a complement set.
    CSet operator ~ (CSet&& Argument);      // Return a complement set.

    inline bool operator == (const CSet& Left, const CSet& Right) { return Left.IsEqual(Right); }

    constexpr const char* CSet::GetEmptySymbol()
    {
#ifdef USE_UNICODE_SYMBOLS
        return "∅";
#else
        return "{}";
#endif
    }

    constexpr const char* CSet::GetUniversalSetSymbol()
    {
#ifdef USE_UNICODE_SYMBOLS
        return "𝙐";
#else
        return "U";
#endif
    }

    constexpr const char* CSet::GetComplementSymbol()
    {
#ifdef USE_UNICODE_SYMBOLS
        return "ᶜ";
#else
        return "′";
#endif
    }
}