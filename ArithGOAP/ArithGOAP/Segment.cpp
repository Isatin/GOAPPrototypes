// Copyright 2024 Isaac Hsu

#include <cassert>
#include <limits>

#include "Segment.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const SSegment SSegment::Unset(CNumber::Null, -CNumber::Null);
const SSegment SSegment::Empty(CNumber::Infinity, -CNumber::Infinity);
const SSegment SSegment::Boundless(-CNumber::Infinity, CNumber::Infinity);

SSegment::SSegment(CNumber Value1, CNumber Value2, SAutocorrect)
{
    if (Value1 < Value2)
    {
        Minimum = Value1;
        Maximum = Value2;
    }
    else
    {
        Minimum = Value2;
        Maximum = Value1;
    }
}

SSegment::SSegment(EComparer Comparer, CNumber Value)
{
    switch (Comparer)
    {
    case EComparer::equal:
        Minimum = Value;
        Maximum = Value;
        break;
    case EComparer::lessOrEqual:
        Minimum = -CNumber::Infinity;
        Maximum = Value;
        break;
    case EComparer::greaterOrEqual:
        Minimum = Value;
        Maximum = CNumber::Infinity;
        break;
    default:
        assert(!"Invalid comparer type");
        *this = Empty;
        break;
    }
}

SSegment& SSegment::operator =  (CNumber Value) 
{ 
    Minimum = Value; 
    Maximum = Value; 
    return *this; 
}

SSegment& SSegment::operator += (CNumber Value) 
{ 
    Minimum += Value; 
    Maximum += Value; 
    return *this; 
}

SSegment& SSegment::operator -= (CNumber Value) 
{ 
    Minimum -= Value; 
    Maximum -= Value; 
    return *this; 
}

SSegment& SSegment::operator *= (CNumber Value) 
{ 
    Minimum *= Value; 
    Maximum *= Value;

    if (Value < 0)
    {
        Flip();
    }

    return *this; 
}

SSegment& SSegment::operator /= (CNumber Value) 
{ 
    Minimum /= Value; 
    Maximum /= Value; 

    if (Value < 0)
    {
        Flip();
    }

    return *this; 
}

void SSegment::Flip()
{
    std::swap(Minimum, Maximum);
}

bool SSegment::IsDegenerate(CNumber Tolerance) const
{
    return Minimum.IsEquivalent(Maximum, Tolerance) && !IsEmpty();
}

bool SSegment::IsAnyBounded() const 
{ 
    return Minimum.IsFinite() || Maximum.IsFinite();
}

bool SSegment::IsIdentical(const SSegment& Another) const 
{ 
    return Minimum.IsIdentical(Another.Minimum) && Maximum.IsIdentical(Another.Maximum); 
}

bool SSegment::IsEqual(const SSegment& Another) const 
{ 
    return Minimum.IsEqual(Another.Minimum) && Maximum.IsEqual(Another.Maximum); 
}

bool SSegment::IsEquivalent(const SSegment& Another, CNumber Tolerance) const
{
    return Minimum.IsEquivalent(Another.Minimum, Tolerance) && Maximum.IsEquivalent(Another.Maximum, Tolerance);
}

bool SSegment::IsAllLessThan(const SSegment& Another) const
{
    if (Maximum.Get() >= Another.Minimum.Get())
    {
        return false;
    }

    if (IsEmpty())
    {
        return false;
    }

    if (Another.IsEmpty())
    {
        return false;
    }

    return true;
}

bool SSegment::IsAllGreaterThan(const SSegment& Another) const 
{ 
    return Another.IsAllLessThan(*this);
}

bool SSegment::Contain(CNumber Number, CNumber Tolerance) const
{ 
    return Minimum.IsLessOrEquivalent(Number, Tolerance) && Number.IsLessOrEquivalent(Maximum, Tolerance);
}

CNumber SSegment::GetGap(const SSegment& Another, CNumber Tolerance) const
{
    if (HasIntersection(Another, Tolerance))
    {
        return 0;
    }
    else
    {
        CNumber DistFromMyMin = std::abs(Minimum - Another.Maximum);
        CNumber DistFromMyMax = std::abs(Maximum - Another.Minimum);
        return std::min(DistFromMyMin, DistFromMyMax);
    }
}

void SSegment::Clamp(CNumber& Number) const
{
    if (Number < Minimum)
    {
        Number = Minimum;
    }

    if (Number > Maximum)
    {
        Number = Maximum;
    }
}

void SSegment::Clamp(SSegment& Another) const
{
    if (IsEmpty())
    {
        Another = Empty;
    }
    else
    {
        Clamp(Another.Minimum);
        Clamp(Another.Maximum);
    }
}

bool SSegment::Unclamp(SSegment& Another, CNumber Tolerance) const
{
    if (HasIntersection(Another, Tolerance))
    {
        if (Another.Contain(Minimum, Tolerance))
        {
            Another.Minimum = -CNumber::Infinity;
        }

        if (Another.Contain(Maximum, Tolerance))
        {
            Another.Maximum = CNumber::Infinity;
        }

        return true;
    }
    else
    {
        Another = SSegment::Empty;
        return false;
    }
}

bool SSegment::HasIntersection(const SSegment& Another, CNumber Tolerance) const
{
    if (!(Minimum.IsLessOrEquivalent(Another.Maximum, Tolerance)))
    {
        return false;
    }

    if (!(Another.Minimum.IsLessOrEquivalent(Maximum, Tolerance)))
    {
        return false;
    }

    if (IsEmpty())
    {
        return false;
    }

    if (Another.IsEmpty())
    {
        return false;
    }

    return true;
}

SSegment SSegment::GetIntersection(const SSegment& Another, CNumber Tolerance) const
{
    SSegment Intersection = *this;
    Intersection.Intersect(Another, Tolerance);
    return Intersection;
}

bool SSegment::Intersect(const SSegment& Another, CNumber Tolerance)
{
    if (HasIntersection(Another, Tolerance))
    {
        Minimum = std::max(Minimum, Another.Minimum);
        Maximum = std::min(Maximum, Another.Maximum);
        return true;
    }
    else
    {
        *this = Empty;
        return false;
    }
}

std::string SSegment::ToString() const
{
    std::string Return;

    if (IsEmpty())
    {
        Return = GetEmptySymbol();
    }
    else
    {
        Return = "[";
        Return += Minimum.ToString();
        Return += ", ";
        Return += Maximum.ToString();
        Return += "]";
    }

    return Return;
}

std::string SSegment::Stringize(const std::string& Subject) const
{
    std::string Return;

    auto StringizeWithMinAndMax = [&]()
    {
        std::string String = Minimum.ToString();
        String += EComparer::LessOrEqual.GetSymbol();
        String += Subject;
        String += EComparer::LessOrEqual.GetSymbol();
        String += Maximum.ToString();
        return String;
    };

    if (IsEmpty())
    {
        ((Return = Subject) += "==") += GetEmptySymbol();
    }
    else if (Minimum.IsIdentical(Maximum))
    {
        Return = Subject;
        Return += EComparer::Equal.GetSymbol();
        Return += Minimum.ToString();
    }
    else if (Minimum.IsNegativeInfinity())
    {
        if (Maximum.IsPositiveInfinity())
        {
            Return = StringizeWithMinAndMax();
        }
        else
        {
            ((Return = Subject) += EComparer::LessOrEqual.GetSymbol()) += Maximum.ToString();
        }
    }
    else
    {
        if (Maximum.IsPositiveInfinity())
        {
            ((Return = Subject) += EComparer::GreaterOrEqual.GetSymbol()) += Minimum.ToString();
        }
        else
        {
            Return = StringizeWithMinAndMax();
        }
    }

    return Return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
