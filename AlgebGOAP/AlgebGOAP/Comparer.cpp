// Copyright 2025 Isaac Hsu

#include <cassert>

#include "Comparer.h"
#include "Fact.h"
#include "TriState.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const CDummyEqualComparer EComparer::Equal;
const EComparer EComparer::NotAlmostEqual(almostEqual, true);
const EComparer EComparer::AlmostEqual          = almostEqual;
const EComparer EComparer::Less                 = less;
const EComparer EComparer::NotLess(less, true);
const EComparer EComparer::LessOrEqual          = lessOrEqual;
const EComparer EComparer::NotLessOrEqual(lessOrEqual, true);
const EComparer EComparer::LessOrAlmostEqual    = lessOrAlmostEqual;
const EComparer EComparer::NotLessOrAlmostEqual(lessOrAlmostEqual, true);
const EComparer EComparer::Greater              = greater;
const EComparer EComparer::NotGreater(greater, true);
const EComparer EComparer::GreaterOrEqual       = greaterOrEqual;
const EComparer EComparer::NotGreaterOrEqual(greaterOrEqual, true);
const EComparer EComparer::GreaterOrAlmostEqual = greaterOrAlmostEqual;
const EComparer EComparer::NotGreaterOrAlmostEqual(greaterOrAlmostEqual, true);

bool EComparer::operator () (CNumber Left, CNumber Right, CNumber Tolerance) const
{
    bool Return = false;

    switch (mType)
    {
    case almostEqual:
        Return = Left.IsAlmostEqual(Right, Tolerance);
        break;
    case less:
        Return = Left.Get() < Right.Get();
        break;
    case lessOrEqual:
        Return = Left.Get() <= Right.Get();
        break;
    case lessOrAlmostEqual:
        Return = Left.IsLessOrAlmostEqual(Right, Tolerance);
        break;
    case greater:
        Return = Left.Get() > Right.Get();
        break;
    case greaterOrEqual:
        Return = Left.Get() >= Right.Get();
        break;
    case greaterOrAlmostEqual:
        Return = Left.IsGreaterOrAlmostEqual(Right, Tolerance);
        break;
    default:
        assert(!"Invalid comparer type");
        return false;
    }

    if (mNegative)
    {
        Return = !Return;
    }

    return Return;
}

bool EComparer::IsForward(Type Type)
{
    switch (Type)
    {
    case almostEqual:           return false;
    case less:                  return true;
    case lessOrEqual:           return true;
    case lessOrAlmostEqual:     return true;
    case greater:               return false;
    case greaterOrEqual:        return false;
    case greaterOrAlmostEqual:  return false;
    }

    assert(!"Invalid comparer type");
    return false;
}

bool EComparer::IncludeEquality() const
{
    switch (mType)
    {
    case almostEqual:           return !mNegative;
    case less:                  return  mNegative;
    case lessOrEqual:           return !mNegative;
    case lessOrAlmostEqual:     return !mNegative;
    case greater:               return  mNegative;
    case greaterOrEqual:        return !mNegative;
    case greaterOrAlmostEqual:  return !mNegative;
    }

    assert(!"Invalid comparer type");        
    return false;
}

bool EComparer::IsSymmetric() const
{
    switch (mType)
    {
    case almostEqual:   return true;
    }

    return false;
}

CNumber EComparer::GetGap(CNumber Left, CNumber Right, const CFactDefinition& Definition) const
{
    return GetGap(Left, Right, Definition.GetTolerance(), Definition.GetGapOffset());
}

CNumber EComparer::GetGap(CNumber Left, CNumber Right, CNumber Tolerance, CNumber Offset) const
{
    auto FetchNull                  = [&]() -> CNumber { return CNumber::Null; };
    auto FetchDistance              = [&]() -> CNumber { return std::abs(Left - Right); };
    auto FetchDistancePlusOffset    = [&]() -> CNumber { return std::abs(Left - Right) + Offset; };
    auto FetchGap = [&]<typename TPositiveGap, typename TNegativeGap>(bool Result, const TPositiveGap& PositiveGap, const TNegativeGap& NegativeGap)
    {
        if (mNegative)
        {
            return !Result ? 0_n : NegativeGap();
        }
        else
        {
            return  Result ? 0_n : PositiveGap();
        }
    };

    switch (mType)
    {
    case almostEqual:
        return FetchGap(Left.IsAlmostEqual(Right, Tolerance), FetchDistance, FetchNull);
    case less:
        return FetchGap(Left.Get() < Right.Get(), FetchDistancePlusOffset, FetchDistance);
    case lessOrEqual:
        return FetchGap(Left.Get() <= Right.Get(), FetchDistance, FetchDistancePlusOffset);
    case lessOrAlmostEqual:
        return FetchGap(Left.IsLessOrAlmostEqual(Right, Tolerance), FetchDistance, FetchDistancePlusOffset);
    case greater:    
        return FetchGap(Left.Get() > Right.Get(), FetchDistancePlusOffset, FetchDistance);
    case greaterOrEqual:
        return FetchGap(Left.Get() >= Right.Get(), FetchDistance, FetchDistancePlusOffset);
    case greaterOrAlmostEqual:
        return FetchGap(Left.IsGreaterOrAlmostEqual(Right, Tolerance), FetchDistance, FetchDistancePlusOffset);
    }

    assert(!"Invalid comparer type");
    return CNumber::Null;
}

const char* EComparer::GetName() const
{
    switch (mType)
    {
    case almostEqual:           return mNegative ? "NotAlmostEqual"             : "AlmostEqual";
    case less:                  return mNegative ? "NotLess"                    : "Less";
    case lessOrEqual:           return mNegative ? "NotLessOrEqual"             : "LessOrEqual";
    case lessOrAlmostEqual:     return mNegative ? "NotLessOrAlmostEqual"       : "LessOrAlmostEqual";
    case greater:               return mNegative ? "NotGreater"                 : "Greater";
    case greaterOrEqual:        return mNegative ? "NotGreaterOrEqual"          : "GreaterOrEqual";
    case greaterOrAlmostEqual:  return mNegative ? "NotGreaterOrAlmostEqual"    : "GreaterOrAlmostEqual";
    }

    return "UNDEF";
}

const char* EComparer::GetSymbol() const
{
#ifdef USE_UNICODE_SYMBOLS
    switch (mType)
    {
    case almostEqual:           return mNegative ? "≄" : "≃";
    case less:                  return mNegative ? "≮" : "<";
    case lessOrEqual:           return mNegative ? "≰" : "≤";
    case lessOrAlmostEqual:     return mNegative ? "≴" : "≲";
    case greater:               return mNegative ? "≯" : ">";
    case greaterOrEqual:        return mNegative ? "≱" : "≥";
    case greaterOrAlmostEqual:  return mNegative ? "≵" : "≳";
    }
#else
    switch (mType)
    {
    case almostEqual:           return mNegative ? "!~"  : "=~";
    case less:                  return mNegative ? "!<"  : "<";
    case lessOrEqual:           return mNegative ? "!<=" : "<=";
    case lessOrAlmostEqual:     return mNegative ? "!<~" : "<~";
    case greater:               return mNegative ? "!>"  : ">";
    case greaterOrEqual:        return mNegative ? "!>=" : ">=";
    case greaterOrAlmostEqual:  return mNegative ? "!>~" : ">~";
    }
#endif // USE_UNICODE_SYMBOLS

    assert(!"Invalid comparer type");
    return nullptr;
}

std::string EComparer::Stringize(const std::string& Left, const std::string& Right) const
{
    std::string Return = Left;
    Return += GetSymbol();
    Return += Right;
    return Return;
}

bool EComparer::IsTautological(CNumber Right) const
{
    if (Right.IsPositiveInfinity())
    {
        if (*this == EComparer::LessOrEqual || *this == EComparer::LessOrAlmostEqual || *this == EComparer::NotGreater)
        {
            return true;
        }
    }
    else if (Right.IsNegativeInfinity())
    {
        if (*this == EComparer::GreaterOrEqual || *this == EComparer::GreaterOrAlmostEqual || *this == EComparer::NotLess)
        {
            return true;
        }
    }
    else if (Right.IsNaN())
    {
        return mNegative; // Any comparison involving NaN is false.
    }

    return false;
}

ETriState EComparer::UnconditionallyEvaluate(CNumber Right) const
{
    if (Right.IsPositiveInfinity())
    {
        if (*this == EComparer::LessOrEqual || *this == EComparer::LessOrAlmostEqual || *this == EComparer::NotGreater)
        {
            return ETriState::yes;
        }

        if (*this == EComparer::Greater || *this == EComparer::NotLessOrEqual || *this == EComparer::NotLessOrAlmostEqual)
        {
            return ETriState::no;
        }
    }
    else if (Right.IsNegativeInfinity())
    {
        if (*this == EComparer::GreaterOrEqual || *this == EComparer::GreaterOrAlmostEqual || *this == EComparer::NotLess)
        {
            return ETriState::yes;
        }

        if (*this == EComparer::Less || *this == EComparer::NotGreaterOrEqual || *this == EComparer::NotGreaterOrAlmostEqual)
        {
            return ETriState::no;
        }
    }
    else if (Right.IsNaN())
    {
        return ETriState(mNegative); // Any comparison involving NaN is false.
    }

    return ETriState::unknown;
}

void EComparer::Negate()
{
    if (mNegative)
    {
        mNegative = false;
    }
    else
    {
        // The C++ operators with equality used to compose CRelation are deemed approximate rather than exact.
        // For example, !(x<=y) is actually equivalent to ¬(x≲y) ≡ ¬(x<y∨x≃y) ≢ ¬(x<y∨x==y) ≡ x>y.
        // Thus, we cannot convert negated inequalities with approximate equality as we do with exact equality. 
        // Instead, we set the negative flag.
        switch (mType)
        {
        case almostEqual:           mNegative = true;                   break;
        case less:                  mType = greaterOrEqual;             break;
        case lessOrEqual:           mType = greater;                    break;
        case lessOrAlmostEqual:     mNegative = true;                   break;
        case greater:               mType = lessOrEqual;                break;
        case greaterOrEqual:        mType = less;                       break;
        case greaterOrAlmostEqual:  mNegative = true;                   break;
        default:                    assert(!"Invalid comparer type");   break;
        }
    } 
}

EComparer EComparer::GetReverse() const
{
    switch (mType)
    {
    case less:                  return EComparer(greater, mNegative);
    case lessOrEqual:           return EComparer(greaterOrEqual, mNegative);
    case lessOrAlmostEqual:     return EComparer(greaterOrAlmostEqual, mNegative);
    case greater:               return EComparer(less, mNegative);
    case greaterOrEqual:        return EComparer(lessOrEqual, mNegative);
    case greaterOrAlmostEqual:  return EComparer(lessOrAlmostEqual, mNegative);
    }

    return *this;
}

void EComparer::Reverse()
{
    switch (mType)
    {
    case less:                  mType = greater;                break;
    case lessOrEqual:           mType = greaterOrEqual;         break;
    case lessOrAlmostEqual:     mType = greaterOrAlmostEqual;   break;
    case greater:               mType = less;                   break;
    case greaterOrEqual:        mType = lessOrEqual;            break;
    case greaterOrAlmostEqual:  mType = lessOrAlmostEqual;      break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
