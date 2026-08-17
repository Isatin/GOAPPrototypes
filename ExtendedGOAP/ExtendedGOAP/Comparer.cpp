// Copyright 2026 Isaac Hsu

#include <cassert>

#include "Comparer.h"
#include "Fact.h"
#include "Matrix.h"
#include "Set.h"
#include "TriState.h"
#include "Variant.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const EComparer EComparer::Equal                = equal;
const EComparer EComparer::NotEqual(equal, true);
const EComparer EComparer::AlmostEqual          = almostEqual;
const EComparer EComparer::NotAlmostEqual(almostEqual, true);
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
const EComparer EComparer::SetMember            = setMember;
const EComparer EComparer::SetHasMember         = setHasMember;
const EComparer EComparer::Subset               = subset;
const EComparer EComparer::ProperSubset         = properSubset;
const EComparer EComparer::Superset             = superset;
const EComparer EComparer::ProperSuperset       = properSuperset;

bool EComparer::operator () (CNumber Left, CNumber Right, CNumber Tolerance) const
{
    bool Return = false;

    switch (mType)
    {
    case equal:
        Return = Left.IsEqual(Right);
        break;
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

bool EComparer::operator () (const CVariant& Left, const CVariant& Right, CNumber Tolerance) const
{
    bool Return = false;

    switch (mType)
    {
    case equal:
        Return = Left.IsEqual(Right);
        break;
    case almostEqual:
        Return = Left.IsAlmostEqual(Right, Tolerance);
        break;
    case less:
        Return = Left.GrabScalar() < Right.GrabScalar();
        break;
    case lessOrEqual:
        Return = Left.GrabScalar() <= Right.GrabScalar();
        break;
    case lessOrAlmostEqual:
        Return = Left.GrabScalar().IsLessOrAlmostEqual(Right.GrabScalar(), Tolerance);
        break;
    case greater:
        Return = Left.GrabScalar() > Right.GrabScalar();
        break;
    case greaterOrEqual:
        Return = Left.GrabScalar() >= Right.GrabScalar();
        break;
    case greaterOrAlmostEqual:
        Return = Left.GrabScalar().IsGreaterOrAlmostEqual(Right.GrabScalar(), Tolerance);
        break;
    case setMember:
        Return = Right.GrabSet().Contain(Left.GrabScalar());
        break;
    case setHasMember:
        Return = Left.GrabSet().Contain(Right.GrabScalar());
        break;
    case subset:
        Return = Right.GrabSet().Contain(Left.GrabSet());
        break;
    case properSubset:
        Return = Right.GrabSet().ProperContain(Left.GrabSet());
        break;
    case superset:
        Return = Left.GrabSet().Contain(Right.GrabSet());
        break;
    case properSuperset:
        Return = Left.GrabSet().ProperContain(Right.GrabSet());
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

CNumber EComparer::GetDistanceIN(const CVariant& Left, const CVariant& Right) const
{
    switch (mType)
    {
    case EComparer::setMember:
        return Right.GrabSet().GetMinDistance(Left.GrabScalar());
    case EComparer::setHasMember:
        return Left.GrabSet().GetMinDistance(Right.GrabScalar());        
    }

    assert(Left.GetVariantType() == Right.GetVariantType());

    if (Left.GetVariantType() == EVariantType::unset)
    {
        return 0;
    }
    else if (Left.GetVariantType() == EVariantType::scalar)
    {
        return std::abs(Left.GrabScalar() - Right.GrabScalar());
    }
    else if (Left.GetVariantType() == EVariantType::matrix)
    {
        return Left.GrabMatrix().GetManhattanDistance(Right.GrabMatrix());
    }
    else if (Left.GetVariantType() == EVariantType::set)
    {
        auto FetchProperSupersetDistance = [](const CSet& LeftSet, const CSet& RightSet)
        {
            if (RightSet.IsUniversalSet())
            {
                return CNumber::Null; // Return null because no set can properly contain a universal set.
            }
            else
            {
                size_t Lack = RightSet.GetDifferenceSize(LeftSet);          // How many members in the right are missing from the left?
                size_t Extra = LeftSet.IsDifferenceEmpty(RightSet) ? 1 : 0; // Does the left have an extra member to properly contain the right?
                assert(Lack + Extra <= CNumber::GetOrderedNumberCount());   // Verify that the distance should not be greater than the universal set size.
                return CNumber(Lack + Extra);
            }
        };

        switch (mType)
        {
        case EComparer::equal:          return Left.GrabSet().GetSymmetricDifferenceSize(Right.GrabSet());
        case EComparer::subset:         return Left.GrabSet().GetDifferenceSize(Right.GrabSet());
        case EComparer::properSubset:   return FetchProperSupersetDistance(Right.GrabSet(), Left.GrabSet());
        case EComparer::superset:       return Right.GrabSet().GetDifferenceSize(Left.GrabSet());
        case EComparer::properSuperset: return FetchProperSupersetDistance(Left.GrabSet(), Right.GrabSet());
        }

        assert(!"Invalid comparer type");
    }

    assert(!"Invalid variant type");
    return CNumber::Null;
}

CNumber EComparer::GetGap(const CVariant& Left, const CVariant& Right, const CFactDefinition& Definition) const
{
    return GetGap(Left, Right, Definition.GetTolerance(), Definition.GetGapOffset());
}

CNumber EComparer::GetGap(const CVariant& Left, const CVariant& Right, CNumber Tolerance, CNumber Offset) const
{
    auto FetchNull                  = [&]() { return CNumber::Null; };
    auto FetchDistance              = [&]() { return GetDistanceIN(Left, Right); };    
    auto FetchDistancePlusOffset    = [&]() { return GetDistanceIN(Left, Right) + Offset; };
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

    auto FetchMembershipGap = [&](bool Result, const CSet& Set, CNumber Value)
    {
        if (mNegative)
        {
            if (!Result)
            {
                return 0_n; // Return zero if the comparison matches.
            }

            if (Set.IsComplement())
            {
                if (Set.GetEntryCount() <= 0)
                {
                    return CNumber::Null; // Return null because a universal set can't exclude any members.
                }
            }
            else
            {
                return CNumber::Null; // Return null for an unmet negated membership operation so that the heuristic will use the base relation cost.
            }
        }
        else
        {
            if (Result)
            {
                return 0_n; // Return zero if the comparison matches.
            }

            if (Set.IsComplement())
            {
                if (Set.GetEntryCount() <= 0)
                {
                    return 0_n; // Return zero because a universal set includes every member.
                }

                return CNumber::Null; // S'∋x ≡ S∌x
            }
            else
            {
                if (Set.GetEntryCount() <= 0)
                {
                    return CNumber::Null; // Return null because an empty set includes no members.
                }
            }
        }

        return Set.GetMinDistanceIC(Value);
    };

    switch (mType)
    {
    case equal:
        return FetchGap(Left.IsEqual(Right), FetchDistance, FetchNull);
    case almostEqual:
        return FetchGap(Left.IsAlmostEqual(Right, Tolerance), FetchDistance, FetchNull);
    case less:
        return FetchGap(Left.GrabScalar() < Right.GrabScalar(), FetchDistancePlusOffset, FetchDistance);
    case lessOrEqual:
        return FetchGap(Left.GrabScalar() <= Right.GrabScalar(), FetchDistance, FetchDistancePlusOffset);
    case lessOrAlmostEqual:
        return FetchGap(Left.GrabScalar().IsLessOrAlmostEqual(Right.GrabScalar(), Tolerance), FetchDistance, FetchDistancePlusOffset);
    case greater:
        return FetchGap(Left.GrabScalar() > Right.GrabScalar(), FetchDistancePlusOffset, FetchDistance);
    case greaterOrEqual:
        return FetchGap(Left.GrabScalar() >= Right.GrabScalar(), FetchDistance, FetchDistancePlusOffset);
    case greaterOrAlmostEqual:
        return FetchGap(Left.GrabScalar().IsGreaterOrAlmostEqual(Right.GrabScalar(), Tolerance), FetchDistance, FetchDistancePlusOffset);
    case setMember:
        return FetchMembershipGap(Right.GrabSet().Contain(Left.GrabScalar()), Right.GrabSet(), Left.GrabScalar());
    case setHasMember:
        return FetchMembershipGap(Left.GrabSet().Contain(Right.GrabScalar()), Left.GrabSet(), Right.GrabScalar());
    case subset:
        return FetchGap(Right.GrabSet().Contain(Left.GrabSet()), FetchDistance, FetchNull);
    case properSubset:
        return FetchGap(Right.GrabSet().ProperContain(Left.GrabSet()), FetchDistance, FetchNull);
    case superset:
        return FetchGap(Left.GrabSet().Contain(Right.GrabSet()), FetchDistance, FetchNull);
    case properSuperset:
        return FetchGap(Left.GrabSet().ProperContain(Right.GrabSet()), FetchDistance, FetchNull);
    }

    assert(!"Invalid comparer type");
    return CNumber::Null;
}

const char* EComparer::GetName() const
{
    switch (mType)
    {
    case equal:                 return mNegative ? "NotEqual"                   : "Equal";
    case almostEqual:           return mNegative ? "NotAlmostEqual"             : "AlmostEqual";
    case less:                  return mNegative ? "NotLess"                    : "Less";
    case lessOrEqual:           return mNegative ? "NotLessOrEqual"             : "LessOrEqual";
    case lessOrAlmostEqual:     return mNegative ? "NotLessOrAlmostEqual"       : "LessOrAlmostEqual";
    case greater:               return mNegative ? "NotGreater"                 : "Greater";
    case greaterOrEqual:        return mNegative ? "NotGreaterOrEqual"          : "GreaterOrEqual";
    case greaterOrAlmostEqual:  return mNegative ? "NotGreaterOrAlmostEqual"    : "GreaterOrAlmostEqual";
    case setMember:             return mNegative ? "NotSetMember"               : "SetMember";
    case setHasMember:          return mNegative ? "SetHasNoSuchMember"         : "SetHasMember";
    case subset:                return mNegative ? "NotSubset"                  : "Subset";
    case properSubset:          return mNegative ? "NotProperSubset"            : "ProperSubset";
    case superset:              return mNegative ? "NotSuperset"                : "Superset";
    case properSuperset:        return mNegative ? "NotProperSuperset"          : "ProperSuperset";
    }

    return "UNDEF";
}

const char* EComparer::GetSymbol() const
{
#ifdef USE_UNICODE_SYMBOLS
    switch (mType)
    {
    case equal:                 return mNegative ? "≠" : "==";
    case almostEqual:           return mNegative ? "≄" : "≃";
    case less:                  return mNegative ? "≮" : "<";
    case lessOrEqual:           return mNegative ? "≰" : "≤";
    case lessOrAlmostEqual:     return mNegative ? "≴" : "≲";
    case greater:               return mNegative ? "≯" : ">";
    case greaterOrEqual:        return mNegative ? "≱" : "≥";
    case greaterOrAlmostEqual:  return mNegative ? "≵" : "≳";
    case setMember:             return mNegative ? "∉" : "∈";
    case setHasMember:          return mNegative ? "∌" : "∋";
    case subset:                return mNegative ? "⊈" : "⊆";
    case properSubset:          return mNegative ? "⊄" : "⊂";
    case superset:              return mNegative ? "⊉" : "⊇";
    case properSuperset:        return mNegative ? "⊅" : "⊃";
    }
#else
    switch (mType)
    {
    case equal:                 return mNegative ? "!="  : "==";
    case almostEqual:           return mNegative ? "!~"  : "=~";
    case less:                  return mNegative ? "!<"  : "<";
    case lessOrEqual:           return mNegative ? "!<=" : "<=";
    case lessOrAlmostEqual:     return mNegative ? "!<~" : "<~";
    case greater:               return mNegative ? "!>"  : ">";
    case greaterOrEqual:        return mNegative ? "!>=" : ">=";
    case greaterOrAlmostEqual:  return mNegative ? "!>~" : ">~";
    case setMember:             return mNegative ? "!<<" : "<<";
    case setHasMember:          return mNegative ? "!>>" : ">>";
    case subset:                return mNegative ? "!<=" : "<=";
    case properSubset:          return mNegative ? "!<" : "<"
    case superset:              return mNegative ? "!>=" : ">=";
    case properSuperset:        return mNegative ? "!>"  : ">"
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

bool EComparer::IsForward(Type Type)
{
    switch (Type)
    {
    case equal:                 return false;
    case almostEqual:           return false;
    case less:                  return true;
    case lessOrEqual:           return true;
    case lessOrAlmostEqual:     return true;
    case greater:               return false;
    case greaterOrEqual:        return false;
    case greaterOrAlmostEqual:  return false;
    case setMember:             return true;
    case setHasMember:          return false;
    case subset:                return true;
    case properSubset:          return true;
    case superset:              return false;
    case properSuperset:        return false;
    }

    assert(!"Invalid comparer type");
    return false;
}

bool EComparer::IncludeEquality(Type Type)
{
    switch (Type)
    {
    case equal:                 return true;
    case almostEqual:           return true;
    case lessOrEqual:           return true;
    case lessOrAlmostEqual:     return true;
    case greaterOrEqual:        return true;
    case greaterOrAlmostEqual:  return true;
    case subset:                return true;
    case superset:              return true;
    }

    return false;
}

bool EComparer::IncludeEquality() const
{
    switch (mType)
    {
    case equal:                 return !mNegative;
    case almostEqual:           return !mNegative;
    case less:                  return  mNegative;
    case lessOrEqual:           return !mNegative;
    case lessOrAlmostEqual:     return !mNegative;
    case greater:               return  mNegative;
    case greaterOrEqual:        return !mNegative;
    case greaterOrAlmostEqual:  return !mNegative;
    case subset:                return !mNegative;
    case properSubset:          return  mNegative;
    case superset:              return !mNegative;
    case properSuperset:        return  mNegative;
    }

    return false;
}

bool EComparer::IsOrdered() const
{ 
    switch (mType)
    {
    case equal:                 return true;
    case almostEqual:           return true;
    case less:                  return true;
    case lessOrEqual:           return true;
    case lessOrAlmostEqual:     return true;
    case greater:               return true;
    case greaterOrEqual:        return true;
    case greaterOrAlmostEqual:  return true;
    }

    return false;
}

bool EComparer::IsSetComparison() const 
{ 
    switch (mType)
    {
    case equal:             return true;
    case subset:            return true;
    case properSubset:      return true;
    case superset:          return true;
    case properSuperset:    return true;
    }

    return false;
}

bool EComparer::IsSymmetric() const
{
    switch (mType)
    {
    case equal:         return true;
    case almostEqual:   return true;
    }

    return false;
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
        case equal:                 mNegative = true;                   break;
        case almostEqual:           mNegative = true;                   break;
        case less:                  mType = greaterOrEqual;             break;
        case lessOrEqual:           mType = greater;                    break;
        case lessOrAlmostEqual:     mNegative = true;                   break;
        case greater:               mType = lessOrEqual;                break;
        case greaterOrEqual:        mType = less;                       break;
        case greaterOrAlmostEqual:  mNegative = true;                   break;
        case setHasMember:          mNegative = true;                   break;
        case setMember:             mNegative = true;                   break;
        case superset:              mNegative = true;                   break;
        case properSuperset:        mNegative = true;                   break;
        case subset:                mNegative = true;                   break;
        case properSubset:          mNegative = true;                   break; 
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
    case setHasMember:          return EComparer(setMember, mNegative);
    case setMember:             return EComparer(setHasMember, mNegative);
    case superset:              return EComparer(subset, mNegative);
    case properSuperset:        return EComparer(properSubset, mNegative);
    case subset:                return EComparer(superset, mNegative);
    case properSubset:          return EComparer(properSuperset, mNegative);
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
    case setHasMember:          mType = setMember;              break;
    case setMember:             mType = setHasMember;           break;
    case superset:              mType = subset;                 break;
    case properSuperset:        mType = properSubset;           break;
    case subset:                mType = superset;               break;
    case properSubset:          mType = properSuperset;         break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
