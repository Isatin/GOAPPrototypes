// Copyright 2025 Isaac Hsu

#include <cassert>

#include "Span.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool ESpanType::IsComplement(ESpanType Another) const
{
    if (mValue == Another.mValue)
    {
        return false;
    }

    return (static_cast<signed char>(mValue) & 2) == (static_cast<signed char>(Another.mValue) & 2);
}

const char* ESpanType::GetLeftSymbol() const
{
    switch (mValue)
    {
    case open:                      return "(";
    case closed:                    return "[";
    case approximate:               return "[~";
    case approximationExclusive:    return "(~";
    }

    assert(!"Invalid span type");
    return "";
}

const char* ESpanType::GetRightSymbol() const
{
    switch (mValue)
    {
    case open:                      return ")";
    case closed:                    return "]";
    case approximate:               return "~]";    
    case approximationExclusive:    return "~)";
    }

    assert(!"Invalid span type");
    return "";
}

EComparer ESpanType::GetInfimumComparer() const
{
    switch (mValue)
    {
    case open:                      return EComparer::Greater;
    case closed:                    return EComparer::GreaterOrEqual;
    case approximate:               return EComparer::GreaterOrAlmostEqual;
    case approximationExclusive:    return EComparer::NotLessOrAlmostEqual; // x>a∧x≄a ≡ ¬(x≤a∨x≃a) ≡ ¬x≲a ≡ x≴a
    }

    assert(!"Invalid span type");
    return EComparer::Greater;
}

EComparer ESpanType::GetSuprenumComparer() const
{
    switch (mValue)
    {
    case open:                      return EComparer::Less;
    case closed:                    return EComparer::LessOrEqual;
    case approximate:               return EComparer::LessOrAlmostEqual;
    case approximationExclusive:    return EComparer::NotGreaterOrAlmostEqual; // x<a∧x≄a ≡ ¬(x≥a∨x≃a) ≡ ¬x≳a ≡ x≵a
    }

    assert(!"Invalid span type");
    return EComparer::Less;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const SSpan SSpan::Empty(CNumber::Infinity, -CNumber::Infinity);
const SSpan SSpan::Boundless(-CNumber::Infinity, CNumber::Infinity, ESpanType::closed, ESpanType::closed);
const SSpan SSpan::PositiveInfinityExclusion(-CNumber::Infinity, CNumber::Infinity, ESpanType::closed, ESpanType::open);
const SSpan SSpan::NegativeInfinityExclusion(-CNumber::Infinity, CNumber::Infinity, ESpanType::open, ESpanType::closed);

SSpan::SSpan() 
    : Infimum(Empty.Infimum)
    , Supremum(Empty.Supremum)
    , InfimumType(ESpanType::open)
    , SupremumType(ESpanType::open)
{}

SSpan::SSpan(CNumber Value) 
    : Infimum(Value)
    , Supremum(Value)
    , InfimumType(ESpanType::approximate)
    , SupremumType(ESpanType::approximate)
{}

SSpan::SSpan(CNumber Infimum, CNumber Supremum)
    : Infimum(Infimum)
    , Supremum(Supremum)
    , InfimumType(ESpanType::open)
    , SupremumType(ESpanType::open)
{}

SSpan::SSpan(CNumber Infimum, SApproximateNumber Supremum)
    : Infimum(Infimum)
    , Supremum(Supremum.Subject)
    , InfimumType(ESpanType::open)
    , SupremumType(ESpanType::approximationExclusive)
{}

SSpan::SSpan(SApproximateNumber Infimum, CNumber Supremum)
    : Infimum(Infimum.Subject)
    , Supremum(Supremum)
    , InfimumType(ESpanType::approximationExclusive)
    , SupremumType(ESpanType::open)
{}

SSpan::SSpan(SApproximateNumber Infimum, SApproximateNumber Supremum)
    : Infimum(Infimum.Subject)
    , Supremum(Supremum.Subject)
    , InfimumType(ESpanType::approximationExclusive)
    , SupremumType(ESpanType::approximationExclusive)
{}

SSpan::SSpan(CNumber Value1, CNumber Value2, SAutocorrect)
    : InfimumType(ESpanType::open)
    , SupremumType(ESpanType::open)
{
    if (Value1 < Value2)
    {
        Infimum = Value1;
        Supremum = Value2;
    }
    else
    {
        Infimum = Value2;
        Supremum = Value1;
    }
}

SSpan::SSpan(CNumber Infimum, CNumber Supremum, ESpanType EndpointType)
    : Infimum(Infimum)
    , Supremum(Supremum) 
    , InfimumType(EndpointType)
    , SupremumType(EndpointType)
{}

SSpan::SSpan(CNumber Infimum, CNumber Supremum, ESpanType InfimumType, ESpanType SupremumType)
    : Infimum(Infimum)
    , Supremum(Supremum) 
    , InfimumType(InfimumType)
    , SupremumType(SupremumType)
{}

SSpan::SSpan(CNumber Value1, CNumber Value2, ESpanType Value1Type, ESpanType Value2Type, SAutocorrect)
{
    if (Value1 < Value2)
    {
        Infimum = Value1;
        Supremum = Value2;
        InfimumType = Value1Type;
        SupremumType = Value2Type;
    }
    else
    {
        Infimum = Value2;
        Supremum = Value1;
        InfimumType = Value2Type;
        SupremumType = Value1Type;
    }
}

SSpan::SSpan(EComparer Comparer, CNumber Right)
{
    bool Ok = Set(Comparer, Right);
    assert(Ok);
}

bool SSpan::Set(EComparer Comparer, CNumber Right)
{
    if (Comparer.IsNegative())
    {
        if (Right.IsNaN())
        {
            // No number can satisfy the comparison with NaN, so every number can satisfy the negation.
            *this = SSpan::Boundless;
        }
        else if (Comparer.GetType() == EComparer::almostEqual)
        {
            // The complement of an interval is not an interval in most cases, so it's not supported.
            return false;
        }
        else if (Comparer.GetType() == EComparer::less)
        {
            return Set(EComparer::greaterOrEqual, Right);
        }
        else if (Comparer.GetType() == EComparer::lessOrEqual)
        {
            return Set(EComparer::greater, Right);
        }
        else if (Comparer.GetType() == EComparer::lessOrAlmostEqual)
        {
            // x≴a ≡ ¬(x<a∨x≃a) ≡ x≥a∧x≄a ≡ x∈(~a, +∞]
            Infimum = Right;
            Supremum = CNumber::Infinity;
            InfimumType = ESpanType::approximationExclusive;
            SupremumType = ESpanType::closed;
        }
        else if (Comparer.GetType() == EComparer::greater)
        {
            return Set(EComparer::lessOrEqual, Right);
        }
        else if (Comparer.GetType() == EComparer::greaterOrEqual)
        {
            return Set(EComparer::less, Right);
        }
        else if (Comparer.GetType() == EComparer::greaterOrAlmostEqual)
        {
            // x≵a ≡ ¬(x>a∨x≃a) ≡ x≤a∧x≄a ≡ x∈[-∞, a~)
            Infimum = -CNumber::Infinity;
            Supremum = Right;
            InfimumType = ESpanType::closed;
            SupremumType = ESpanType::approximationExclusive;
        }
        else
        {
            assert(!"Invalid comparer type");
            return false;
        }
    }
    else
    {
        if (Right.IsNaN())
        {            
            *this = SSpan::Empty; // Set to empty because no number can satisfy comparison with NaN.
        }
        else if (Comparer.GetType() == EComparer::almostEqual)
        {
            Infimum = Right;
            Supremum = Right;
            InfimumType = ESpanType::approximate;
            SupremumType = ESpanType::approximate;
        }
        else if (Comparer.GetType() == EComparer::less)
        {
            Infimum = -CNumber::Infinity;
            Supremum = Right;
            InfimumType = ESpanType::closed;
            SupremumType = ESpanType::open;
        }
        else if (Comparer.GetType() == EComparer::lessOrEqual)
        {
            Infimum = -CNumber::Infinity;
            Supremum = Right;
            InfimumType = ESpanType::closed;
            SupremumType = ESpanType::closed;
        }
        else if (Comparer.GetType() == EComparer::lessOrAlmostEqual)
        {
            Infimum = -CNumber::Infinity;
            Supremum = Right;
            InfimumType = ESpanType::closed;
            SupremumType = ESpanType::approximate;
        }
        else if (Comparer.GetType() == EComparer::greater)
        {
            Infimum = Right;
            Supremum = CNumber::Infinity;
            InfimumType = ESpanType::open;
            SupremumType = ESpanType::closed;
        }
        else if (Comparer.GetType() == EComparer::greaterOrEqual)
        {
            Infimum = Right;
            Supremum = CNumber::Infinity;
            InfimumType = ESpanType::closed;
            SupremumType = ESpanType::closed;
        }
        else if (Comparer.GetType() == EComparer::greaterOrAlmostEqual)
        {
            Infimum = Right;
            Supremum = CNumber::Infinity;
            InfimumType = ESpanType::approximate;
            SupremumType = ESpanType::closed;
        }
        else
        {
            assert(!"Invalid comparer type");
            return false;
        }
    }

    return true;
}

SSpan& SSpan::operator =  (CNumber Value) 
{ 
    Infimum = Value; 
    Supremum = Value;
    InfimumType = ESpanType::approximate;
    SupremumType = ESpanType::approximate;
    return *this; 
}

SSpan& SSpan::operator += (CNumber Value) 
{ 
    Infimum += Value; 
    Supremum += Value; 
    return *this; 
}

SSpan& SSpan::operator -= (CNumber Value) 
{ 
    Infimum -= Value; 
    Supremum -= Value; 
    return *this; 
}

SSpan& SSpan::operator *= (CNumber Value) 
{ 
    Infimum *= Value; 
    Supremum *= Value;

    if (Value < 0)
    {
        Flip();
    }

    return *this; 
}

SSpan& SSpan::operator /= (CNumber Value) 
{ 
    Infimum /= Value; 
    Supremum /= Value; 

    if (Value < 0)
    {
        Flip();
    }

    return *this; 
}

void SSpan::Flip()
{
    std::swap(Infimum, Supremum);
    std::swap(InfimumType, SupremumType);
}

bool SSpan::IsEmpty(CNumber Tolerance) const
{
    return !CompareInfimumAndSupremum(Infimum, InfimumType, Supremum, SupremumType, Tolerance);
}

CNumber::BPromoted SSpan::GetTolerancedInfimum(CNumber Infimum, ESpanType InfimumType, CNumber Tolerance)
{
    switch (InfimumType)
    {
    case ESpanType::open:
        return Infimum.GetPromoted();
    case ESpanType::closed:
        return Infimum.GetPromoted();
    case ESpanType::approximate:
        return Infimum.GetPromoted() - Infimum.CalcTolerance(Tolerance);
    case ESpanType::approximationExclusive:
        return Infimum.GetPromoted() + Infimum.CalcTolerance(Tolerance);
    }

    assert(!"Invalid comparer type");
    return 0;
}

CNumber::BPromoted SSpan::GetTolerancedSupremum(CNumber Supremum, ESpanType SupremumType, CNumber Tolerance)
{
    switch (SupremumType)
    {
    case ESpanType::open:
        return Supremum.GetPromoted();
    case ESpanType::closed:
        return Supremum.GetPromoted();
    case ESpanType::approximate:
        return Supremum.GetPromoted() + Supremum.CalcTolerance(Tolerance);
    case ESpanType::approximationExclusive:
        return Supremum.GetPromoted() - Supremum.CalcTolerance(Tolerance);
    }

    assert(!"Invalid comparer type");
    return 0;
}

bool SSpan::CompareInfimumAndSupremum(CNumber Infimum, ESpanType InfimumType, CNumber Supremum, ESpanType SupremumType, CNumber Tolerance)
{
    if (InfimumType == ESpanType::open)
    {
        switch (SupremumType)
        {
        case ESpanType::open:
            return Infimum.Get() < Supremum.Get();
        case ESpanType::closed:
            return Infimum.Get() < Supremum.Get();
        case ESpanType::approximate:
            return Infimum.GetPromoted() < Supremum.GetPromoted() + Supremum.CalcTolerance(Tolerance);
        case ESpanType::approximationExclusive:
            return Infimum.GetPromoted() < Supremum.GetPromoted() - Supremum.CalcTolerance(Tolerance);
        }
    }
    else if (InfimumType == ESpanType::closed)
    {
        switch (SupremumType)
        {
        case ESpanType::open:
            return Infimum.Get() < Supremum.Get();
        case ESpanType::closed:
            return Infimum.Get() <= Supremum.Get();
        case ESpanType::approximate:
            return Infimum.GetPromoted() <= Supremum.GetPromoted() + Supremum.CalcTolerance(Tolerance);
        case ESpanType::approximationExclusive:
            return Infimum.GetPromoted() <  Supremum.GetPromoted() - Supremum.CalcTolerance(Tolerance);
        }
    }
    else if (InfimumType == ESpanType::approximate)
    {
        switch (SupremumType)
        {
        case ESpanType::open:
            return Infimum.GetPromoted() - Infimum.CalcTolerance(Tolerance) <  Supremum.GetPromoted();
        case ESpanType::closed:
            return Infimum.GetPromoted() - Infimum.CalcTolerance(Tolerance) <= Supremum.GetPromoted();
        case ESpanType::approximate:
            return Infimum.GetPromoted() - Infimum.CalcTolerance(Tolerance) <= Supremum.GetPromoted() + Supremum.CalcTolerance(Tolerance);
        case ESpanType::approximationExclusive:
            return Infimum.GetPromoted() - Infimum.CalcTolerance(Tolerance) <  Supremum.GetPromoted() - Supremum.CalcTolerance(Tolerance);
        }
    }
    else if (InfimumType == ESpanType::approximationExclusive)
    {
        switch (SupremumType)
        {
        case ESpanType::open:
            return Infimum.GetPromoted() + Infimum.CalcTolerance(Tolerance) < Supremum.GetPromoted();
        case ESpanType::closed:
            return Infimum.GetPromoted() + Infimum.CalcTolerance(Tolerance) < Supremum.GetPromoted();
        case ESpanType::approximate:
            return Infimum.GetPromoted() + Infimum.CalcTolerance(Tolerance) < Supremum.GetPromoted() + Supremum.CalcTolerance(Tolerance);
        case ESpanType::approximationExclusive:
            return Infimum.GetPromoted() + Infimum.CalcTolerance(Tolerance) < Supremum.GetPromoted() - Supremum.CalcTolerance(Tolerance);
        }
    }

    assert(!"Invalid span type");
    return false;
}

bool SSpan::CompareInfimum(CNumber Left, ESpanType LeftType, CNumber Right, ESpanType RightType, CNumber Tolerance)
{
    CNumber::BPromoted TolerancedLeft = GetTolerancedInfimum(Left, LeftType, Tolerance);
    CNumber::BPromoted TolerancedRight = GetTolerancedInfimum(Right, RightType, Tolerance);
    if (LeftType.IsInclusive() && RightType.IsInclusive())
    {
        return TolerancedLeft <= TolerancedRight;
    }
    else
    {
        return TolerancedLeft < TolerancedRight;
    }
}

bool SSpan::CompareSupremum(CNumber Left, ESpanType LeftType, CNumber Right, ESpanType RightType, CNumber Tolerance)
{
    CNumber::BPromoted TolerancedLeft = GetTolerancedSupremum(Left, LeftType, Tolerance);
    CNumber::BPromoted TolerancedRight = GetTolerancedSupremum(Right, RightType, Tolerance);
    if (LeftType.IsInclusive() && RightType.IsInclusive())
    {
        return TolerancedLeft <= TolerancedRight;
    }
    else
    {
        return TolerancedLeft < TolerancedRight;
    }
}

bool SSpan::IsUnbounded() const
{
    return Infimum.IsNegativeInfinity() && Supremum.IsPositiveInfinity();
}

bool SSpan::IsIdentical(const SSpan& Another) const 
{ 
    return Infimum.IsIdentical(Another.Infimum) && Supremum.IsIdentical(Another.Supremum)
        && InfimumType == Another.InfimumType && SupremumType == Another.SupremumType;
}

bool SSpan::IsEqual(const SSpan& Another) const 
{ 
    return Infimum.IsEqual(Another.Infimum) && Supremum.IsEqual(Another.Supremum)
        && InfimumType == Another.InfimumType && SupremumType == Another.SupremumType;
}

bool SSpan::IsAlmostEqual(const SSpan& Another, CNumber Tolerance) const
{
    return Infimum.IsAlmostEqual(Another.Infimum, Tolerance) && Supremum.IsAlmostEqual(Another.Supremum, Tolerance)
        && InfimumType == Another.InfimumType && SupremumType == Another.SupremumType;
}

bool SSpan::Contain(CNumber Value, CNumber Tolerance) const
{
    switch (InfimumType)
    {
    case ESpanType::open:
        if (!(Infimum.Get() < Value.Get()))
        {
            return false;
        }
        break;
    case ESpanType::closed:
        if (!(Infimum.Get() <= Value.Get()))
        {
            return false;
        }
        break;
    case ESpanType::approximate:
        if (!Infimum.IsLessOrAlmostEqual(Value, Tolerance))
        {
            return false;
        }
        break;
    case ESpanType::approximationExclusive:
        if (Infimum.IsGreaterOrAlmostEqual(Value, Tolerance))
        {
            return false;
        }
        break;
    }

    switch (SupremumType)
    {
    case ESpanType::open:
        if (!(Supremum.Get() > Value.Get()))
        {
            return false;
        }
        break;
    case ESpanType::closed:
        if (!(Supremum.Get() >= Value.Get()))
        {
            return false;
        }
        break;
    case ESpanType::approximate:
        if (!Supremum.IsGreaterOrAlmostEqual(Value, Tolerance))
        {
            return false;
        }
        break;
    case ESpanType::approximationExclusive:
        if (Supremum.IsLessOrAlmostEqual(Value, Tolerance))
        {
            return false;
        }
        break;
    }

    return true;
}

bool SSpan::HasIntersection(const SSpan& Another, CNumber Tolerance) const
{
    if (!CompareInfimumAndSupremum(Infimum, InfimumType, Another.Supremum, Another.SupremumType, Tolerance))
    {
        return false;
    }

    if (!CompareInfimumAndSupremum(Another.Infimum, Another.InfimumType, Supremum, SupremumType, Tolerance))
    {
        return false;
    }

    return true;
}

SSpan SSpan::GetIntersection(const SSpan& Another, CNumber Tolerance) const
{
    SSpan Intersection = *this;
    Intersection.Intersect(Another, Tolerance);
    return Intersection;
}

bool SSpan::Intersect(const SSpan& Another, CNumber Tolerance)
{
    if (!HasIntersection(Another, Tolerance))
    {
        *this = Empty;
        return false;
    }

    SSpan Unselected = Another;
    CNumber::BPromoted ThisTolerancedInfimum = GetTolerancedInfimum(Infimum, InfimumType, Tolerance);
    CNumber::BPromoted ThatTolerancedInfimum = GetTolerancedInfimum(Another.Infimum, Another.InfimumType, Tolerance);
    if (ThisTolerancedInfimum < ThatTolerancedInfimum)
    {
        std::swap(Infimum, Unselected.Infimum);
        std::swap(InfimumType, Unselected.InfimumType);
    }

    if (Infimum.IsEqual(Unselected.Infimum) && Unselected.InfimumType < InfimumType)
    {
        InfimumType = Unselected.InfimumType;
    }

    CNumber::BPromoted ThisTolerancedSupremum = GetTolerancedSupremum(Supremum, SupremumType, Tolerance);
    CNumber::BPromoted ThatTolerancedSupremum = GetTolerancedSupremum(Another.Supremum, Another.SupremumType, Tolerance);
    if (ThisTolerancedSupremum > ThatTolerancedSupremum)
    {
        std::swap(Supremum, Unselected.Supremum);
        std::swap(SupremumType, Unselected.SupremumType);
    }

    if (Supremum.IsEqual(Unselected.Supremum) && Unselected.SupremumType < SupremumType)
    {
        SupremumType = Unselected.SupremumType;
    }

    return true;
}

bool SSpan::Union(const SSpan& Another, CNumber Tolerance)
{
    if (IsEmpty())
    {
        *this = Another;
        return true;
    }

    if (HasIntersection(Another, Tolerance))
    {
        SSpan Unselected = Another;
        CNumber::BPromoted ThisTolerancedInfimum = GetTolerancedInfimum(Infimum, InfimumType, Tolerance);
        CNumber::BPromoted ThatTolerancedInfimum = GetTolerancedInfimum(Another.Infimum, Another.InfimumType, Tolerance);
        if (ThisTolerancedInfimum > ThatTolerancedInfimum)
        {
            std::swap(Infimum, Unselected.Infimum);
            std::swap(InfimumType, Unselected.InfimumType);
        }

        if (Infimum.IsEqual(Unselected.Infimum) && Unselected.InfimumType > InfimumType)
        {
            InfimumType = Unselected.InfimumType;
        }

        CNumber::BPromoted ThisTolerancedSupremum = GetTolerancedSupremum(Supremum, SupremumType, Tolerance);
        CNumber::BPromoted ThatTolerancedSupremum = GetTolerancedSupremum(Another.Supremum, Another.SupremumType, Tolerance);
        if (ThisTolerancedSupremum < ThatTolerancedSupremum)
        {
            std::swap(Supremum, Unselected.Supremum);
            std::swap(SupremumType, Unselected.SupremumType);
        }

        if (Supremum.IsEqual(Unselected.Supremum) && Unselected.SupremumType > SupremumType)
        {
            SupremumType = Unselected.SupremumType;
        }

        return true;
    }
    else // If no intersection, check if the two spans share a complementary boundary.
    {        
        if (Supremum.IsEqual(Another.Infimum) && CompareSupremum(Supremum, SupremumType, Another.Supremum, Another.SupremumType))
        {
            if (SupremumType.IsComplement(Another.InfimumType))
            {
                Supremum = Another.Supremum;
                SupremumType = Another.SupremumType;
                return true;
            }
        }

        if (Infimum.IsEqual(Another.Supremum) && CompareInfimum(Another.Infimum, Another.InfimumType, Infimum, InfimumType))
        {
            if (InfimumType.IsComplement(Another.SupremumType))
            {
                Infimum = Another.Infimum;
                InfimumType = Another.InfimumType;
                return true;
            }
        }

        return false;
    }
}

std::string SSpan::ToString() const
{
    std::string Return;

    if (IsEmpty())
    {
        Return = GetEmptySymbol();
    }
    else
    {
        Return = InfimumType.GetLeftSymbol();
        Return += Infimum.ToString();
        Return += ", ";
        Return += Supremum.ToString();
        Return += SupremumType.GetRightSymbol();
    }

    return Return;
}

std::string SSpan::Stringize(const std::string& Subject) const
{
    std::string Return;

    if (IsEmpty())
    {
        ((Return = Subject) += EComparer::Equal.GetSymbol()) += GetEmptySymbol();
    }
    else if (Infimum.IsIdentical(Supremum) && InfimumType == SupremumType)
    {
        assert(InfimumType.IsInclusive()); // If the endpoints are exclusive, this span is empty and we should be in the above if block instead.

        Return = Subject;
        Return += IsInfimumClosed() ? EComparer::Equal.GetSymbol() : EComparer::AlmostEqual.GetSymbol();
        Return += Infimum.ToString();
    }
    else
    {
        Return = Infimum.ToString();
        Return += InfimumType.GetSuprenumComparer().GetSymbol();
        Return += Subject;
        Return += SupremumType.GetSuprenumComparer().GetSymbol();
        Return += Supremum.ToString();
    }

    return Return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
