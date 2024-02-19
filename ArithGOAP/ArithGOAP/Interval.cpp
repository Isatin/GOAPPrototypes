// Copyright 2024 Isaac Hsu

#include <cassert>
#include <limits>
#include <sstream>

#include "Interval.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const SInterval SInterval::Boundless(-std::numeric_limits<PValue>::infinity(), std::numeric_limits<PValue>::infinity());
const SInterval SInterval::Unset(Boundless.Minimum, Boundless.Minimum);
const SInterval SInterval::Empty(Boundless.Maximum, Boundless.Maximum);

SInterval::SInterval(SNumber Value1, SNumber Value2)
{
    if (Value1 < Value2)
    {
        Minimum = Value1;
        Maximum = Value2;
    }
    else
    {
        Minimum = Value1;
        Maximum = Value2;
    }
}

SInterval::SInterval(EComparer Cmp, SNumber Value)
{
    switch (Cmp)
    {
    case EComparer::equal:
        Minimum = Value;
        Maximum = Value;
        break;
    case EComparer::greaterOrEqual:
        Minimum = Value;
        Maximum = SNumber::Infinity;
        break;
    case EComparer::lessOrEqual:
        Minimum = -SNumber::Infinity;
        Maximum = Value;
        break;
    default:
        Minimum = Value;
        Maximum = Value;
        break;
    }
}

bool SInterval::operator == (const SInterval& Another) const 
{ 
    if (IsIdentical(Another))
    {
        return true;
    }
    
    if (Minimum == Another.Minimum && Maximum == Another.Maximum)
    {
        return true;
    }

    return false;
}

std::partial_ordering SInterval::operator <=> (const SInterval& Another) const
{
    if (*this == Another)
    {
        return std::partial_ordering::equivalent;
    }
    else if (IsAllGreaterThan(Another))
    {
        return std::partial_ordering::greater;
    }
    else  if (IsAllLessThan(Another))
    {
        return std::partial_ordering::less;
    }
    else
    {
        return std::partial_ordering::unordered;
    }
}

void SInterval::Flip()
{
    std::swap(Minimum, Maximum);
}

bool SInterval::IsDegenerate() const
{
    return Minimum == Maximum && std::isfinite(Minimum);
}

SNumber SInterval::GetGap(const SInterval& Another) const
{
    if (HasIntersection(Another))
    {
        return 0;
    }
    else
    {
        SNumber DistFromMyMin = std::abs(Minimum - Another.Maximum);
        SNumber DistFromMyMax = std::abs(Maximum - Another.Minimum);
        return std::min(DistFromMyMin, DistFromMyMax);
    }
}

void SInterval::Clamp(SNumber& Number) const
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

void SInterval::Clamp(SInterval& Another) const
{
    if (Another.Minimum == Another.Maximum)
    {
        Clamp(Another.Maximum);
        Another.Minimum = Another.Maximum;
    }
    else
    {
        Clamp(Another.Minimum);
        Clamp(Another.Maximum);
    }
}

bool SInterval::Unclamp(SInterval& Another) const
{
    if (HasIntersection(Another))
    {
        if (Another.Contain(Minimum))
        {
            Another.Minimum = -SNumber::Infinity;
        }

        if (Another.Contain(Maximum))
        {
            Another.Maximum = SNumber::Infinity;
        }

        return true;
    }
    else
    {
        Another = SInterval::Empty;
        return false;
    }
}


bool SInterval::HasIntersection(const SInterval& Another) const
{
    if (!(Minimum <= Another.Maximum)) // using NOT less or equal for infinity vs infinity to return false
    {
        return false;
    }

    if (!(Another.Minimum <= Maximum)) // using NOT less or equal for infinity vs infinity to return false
    {
        return false;
    }

    return true;
}

bool SInterval::Intersect(const SInterval& Another)
{
    if (HasIntersection(Another))
    {
        Minimum = std::max(Minimum, Another.Minimum);
        Maximum = std::min(Maximum, Another.Maximum);
        return true;
    }
    else
    {
        return false;
    }
}

std::string SInterval::ToString() const
{
    std::stringstream Stream;

    if (IsEmpty())
    {
        Stream << "[]";
    }
    else
    {
        Stream << (Minimum.IsInfinity() ? "(" : "[") << Minimum << ", " << Maximum << (Maximum.IsInfinity() ? ")" : "]");
    }

    return Stream.str();
}

std::string SInterval::ToString(const std::string& Subject) const
{
    std::stringstream Stream;

    if (IsEmpty())
    {
        Stream << Subject << "=" << ToString();
    }
    else if (Minimum.IsInfinity())
    {
        if (Maximum.IsInfinity())
        {
            Stream << "-inf<" << Subject << "<inf";
        }
        else
        {
            Stream << Subject << EComparer::LessOrEqual.GetSymbol() << Maximum;
        }
    }
    else
    {
        if (Maximum.IsInfinity())
        {
            Stream << Subject << EComparer::GreaterOrEqual.GetSymbol() << Minimum;
        }
        else if (Minimum == Maximum)
        {
            Stream << Subject << EComparer::Equal.GetSymbol() << Minimum;
        }
        else
        {
            Stream << Minimum << "<=" << Subject << "<=" << Maximum;
        }
    }

    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////