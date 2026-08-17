// Copyright 2025 Isaac Hsu

#include <algorithm>
#include <cassert>

#include "Range.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CRange::CRange(CNumber Tolerance)
    : mTolerance(Tolerance)
{}

CRange::CRange(const SSpan& Span, CNumber Tolerance)
    : mTolerance(Tolerance)
{
    Initialize(Span);
}

CRange::CRange(EComparer Comparer, CNumber Right, CNumber Tolerance)
    : mTolerance(Tolerance)
{
    if (Right.IsNaN())
    {
        if (Comparer.IsNegative())
        {
            // A comparison with NaN is always false, making the negated condition true for all numbers.
            *this = SSpan::Boundless;
        }
        else
        {
            // Leave it empty because a comparison with NaN is always false.
        }
    }
    else if (Comparer.IsNotAlmostEqual())
    {
        if (Right.IsPositiveInfinity())
        {
            mSpans.push_back(SSpan::PositiveInfinityExclusion);
        }
        else if (Right.IsNegativeInfinity())
        {
            mSpans.push_back(SSpan::NegativeInfinityExclusion);
        }
        else
        {
            mSpans.emplace_back(-CNumber::Infinity, Right, ESpanType::closed, ESpanType::approximationExclusive);
            mSpans.emplace_back(Right, CNumber::Infinity, ESpanType::approximationExclusive, ESpanType::closed);
        }
    }
    else
    {
        Initialize(SSpan(Comparer, Right));
    }
}

CRange::CRange(CNumber Infimum, CNumber Supremum, ESpanType InfimumType, ESpanType SupremumType, CNumber Tolerance)
{ 
    Initialize(SSpan(Infimum, Supremum, InfimumType, SupremumType));
}

void CRange::Initialize(const SSpan& Span)
{
    if (!Span.IsEmpty())
    {
        mSpans.emplace_back(Span);
    }
}

void CRange::SetTo(const SSpan& Span)
{
    if (Span.IsEmpty())
    {
        mSpans.clear();
    }
    else
    {
        mSpans.resize(1);
        mSpans.front() = Span;
    }
}

std::string CRange::ToString() const
{
    std::string Return = "{";
    bool Successive = false;

    for (const SSpan& Span : mSpans)
    {
        if (Successive)
        {
            Return += ", ";
        }
        else
        {
            Successive = true;
        }

        Return += Span.ToString();
    }

    Return += "}";
    return Return;
}

bool CRange::IsEmpty() const
{
    return mSpans.empty();
}

bool CRange::IsFull() const
{
    return IsUnbounded() && mSpans.front().InfimumType.IsInclusive() && mSpans.front().SupremumType.IsInclusive();
}

bool CRange::IsUnbounded() const
{
    return !mSpans.empty() && mSpans.front().IsUnbounded();
}

bool CRange::IsMergeable() const
{
    if (IsEmpty())
    {
        return false;
    }

    SSpan Union = GrabFirstSpan();
    for (int i = 1; i < mSpans.size(); i++)
    {
        if (Union.Union(mSpans[i], mTolerance))
        {
            return true;
        }
    }

    return false;
}

bool CRange::IsEqual(const CRange& Another) const
{
    return mSpans == Another.mSpans;
}

bool CRange::Contain(CNumber Value) const
{
    for (const SSpan& Span : mSpans)
    {
        if (Span.Contain(Value, mTolerance))
        {
            return true;
        }
    }

    return false;
}

void CRange::Clear()
{
    mSpans.clear();
}

bool CRange::IsSorted() const
{
    return std::ranges::is_sorted(mSpans, CompareSpan);
}

void CRange::Sort()
{
    std::ranges::sort(mSpans, CompareSpan);
}

void CRange::Intersect(EComparer Comparer, CNumber Right)
{
    if (Comparer.IsNotAlmostEqual())
    {
        if (Right.IsNaN())
        {
            // The given constraint is unbounded since x≄NaN ≡ -∞≤x≤+∞.
            // So do nothing because the intersection of any number and the extended real numbers remains the same.
        }
        else if (Right.IsPositiveInfinity())
        {
            Intersect(SSpan::PositiveInfinityExclusion);
        }
        else if (Right.IsNegativeInfinity())
        {
            Intersect(SSpan::NegativeInfinityExclusion);
        }
        else
        {
            CRange LeftSpan = GetIntersection(SSpan(-CNumber::Infinity, Right, ESpanType::closed, ESpanType::approximationExclusive));
            CRange RightSpan = GetIntersection(SSpan(Right, CNumber::Infinity, ESpanType::approximationExclusive, ESpanType::closed));
            mSpans.clear();
            mSpans.append_range(LeftSpan.mSpans);
            mSpans.append_range(RightSpan.mSpans);

            assert(!IsMergeable());
        }
    }
    else
    {
        Intersect(SSpan(Comparer, Right));
    }
}

CRange CRange::GetIntersection(const SSpan& Conjunct) const
{
    CRange Intersection = *this;
    Intersection.Intersect(Conjunct);
    return Intersection;
}

void CRange::Intersect(const SSpan& Conjunct)
{
    for (int i = static_cast<int>(mSpans.size()) - 1; i >= 0; i--)
    {
        if (!mSpans[i].Intersect(Conjunct, mTolerance))
        {
            mSpans.erase(mSpans.begin() + i); // Remove the resulting intersection if empty.
        }
    }

    assert(!IsMergeable());
}

CRange CRange::GetUnion(const SSpan& Disjunct) const
{
    CRange Union = *this;
    Union.Union(Disjunct);
    return Union;
}

void CRange::Union(EComparer Comparer, CNumber Right)
{
    if (Comparer.IsNotAlmostEqual())
    {
        if (Right.IsNaN())
        {
            SetTo(SSpan::Boundless);
        }
        else if (Right.IsPositiveInfinity())
        {
            Union(SSpan::PositiveInfinityExclusion);
        }
        else if (Right.IsNegativeInfinity())
        {
            Union(SSpan::NegativeInfinityExclusion);
        }
        else
        {
            Union(SSpan(-CNumber::Infinity, Right, ESpanType::closed, ESpanType::approximationExclusive));
            Union(SSpan(Right, CNumber::Infinity, ESpanType::approximationExclusive, ESpanType::closed));
        }
    }
    else
    {
        Union(SSpan(Comparer, Right));
    }
}

void CRange::Union(const SSpan& Disjunct)
{
    if (mSpans.empty())
    {
        mSpans.push_back(Disjunct);
        return;
    }

    SSpan Union = Disjunct;
    int MergedIndex = InvalidIndex;

    for (int i = static_cast<int>(mSpans.size()) - 1; i >= 0; i--)
    {
        if (Union.Union(mSpans[i], mTolerance))
        {
            mSpans.erase(mSpans.begin() + i);
            MergedIndex = i;
        }
    }

    if (MergedIndex == InvalidIndex)
    {
        mSpans.emplace_back(std::move(Union)); // TODO: Find a proper position to insert.
    }
    else
    {
        mSpans.insert(mSpans.begin() + MergedIndex, std::move(Union));
    }

    assert(!IsMergeable());
}
///////////////////////////////////////////////////////////////////////////////////////////////////