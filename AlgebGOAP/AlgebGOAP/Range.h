// Copyright 2025 Isaac Hsu

#pragma once

#include <vector>

#include "Span.h"


namespace AlgebGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CRange // Union of intervals for the range of compound inequalities
    {
    private:
        // These spans do not overlap, so just compare one side.
        static bool CompareSpan(const SSpan& Left, const SSpan& Right) { return Left.Infimum < Right.Infimum; }

    public:
        CRange(CNumber Tolerance = CNumber::GetDefaultTolerance());
        CRange(const SSpan& Span, CNumber Tolerance = CNumber::GetDefaultTolerance());
        CRange(EComparer Comparer, CNumber Right, CNumber Tolerance = CNumber::GetDefaultTolerance());
        CRange(CNumber Infimum, CNumber Supremum, ESpanType InfimumType, ESpanType SupremumType, CNumber Tolerance = CNumber::GetDefaultTolerance());

        int GetSpanCount() const { return static_cast<int>(mSpans.size()); }
        const std::vector<SSpan>& GetSpans() const { return mSpans; }
        // PREREQUISITE: The given index is valid.
        const SSpan& GetSpan(int SpanIndex) const { return mSpans[SpanIndex]; }
        // PREREQUISITE: This range not is empty.
        const SSpan& GrabFirstSpan() const { return mSpans.front(); }
        // PREREQUISITE: This range not is empty.
        const SSpan& GrabLastSpan() const { return mSpans.back(); }

        std::string ToString() const;
        bool IsEmpty() const;
        bool IsFull() const;
        bool IsUnbounded() const;
        bool IsMergeable() const;
        bool IsEqual(const CRange& Another) const;
        bool IsSorted() const;
        bool Contain(CNumber Value) const;
        CRange GetIntersection(const SSpan& Conjunct) const;
        CRange GetUnion(const SSpan& Disjunct) const;

        void Clear();
        void Sort();
        void SetTo(const SSpan& Span);
        void Intersect(EComparer Comparer, CNumber Right);
        void Intersect(const SSpan& Conjunct);
        void Union(EComparer Comparer, CNumber Right);
        void Union(const SSpan& Disjunct);

    private:
        void Initialize(const SSpan& Span);

    private:
        std::vector<SSpan> mSpans; // These spans do not overlap.
        CNumber mTolerance = CNumber::GetDefaultTolerance();
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}