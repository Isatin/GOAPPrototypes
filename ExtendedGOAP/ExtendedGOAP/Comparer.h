// Copyright 2026 Isaac Hsu

#pragma once

#include "Number.h"


namespace ExtendedGOAP
{
    class CFactDefinition;
    class CVariant;
    class ETriState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EComparer // Mock enum of comparative operators with a negative flag
    {
    public:
        enum class Type : unsigned char
        {
            equal,
            almostEqual,
            less,
            lessOrEqual,
            lessOrAlmostEqual,
            greater,
            greaterOrEqual,
            greaterOrAlmostEqual,
            setMember,
            setHasMember,
            subset,
            properSubset,
            superset,
            properSuperset,
        };

        static constexpr Type equal                 = Type::equal;
        static constexpr Type almostEqual           = Type::almostEqual;
        static constexpr Type less                  = Type::less;
        static constexpr Type lessOrEqual           = Type::lessOrEqual;
        static constexpr Type lessOrAlmostEqual     = Type::lessOrAlmostEqual;
        static constexpr Type greater               = Type::greater;
        static constexpr Type greaterOrEqual        = Type::greaterOrEqual;
        static constexpr Type greaterOrAlmostEqual  = Type::greaterOrAlmostEqual;
        static constexpr Type setMember             = Type::setMember;
        static constexpr Type setHasMember          = Type::setHasMember;
        static constexpr Type subset                = Type::subset;
        static constexpr Type properSubset          = Type::properSubset;
        static constexpr Type superset              = Type::superset;
        static constexpr Type properSuperset        = Type::properSuperset;

        static const EComparer Equal;
        static const EComparer NotEqual;
        static const EComparer AlmostEqual;
        static const EComparer NotAlmostEqual;
        static const EComparer Less;
        static const EComparer NotLess;
        static const EComparer LessOrEqual;
        static const EComparer NotLessOrEqual;
        static const EComparer LessOrAlmostEqual;
        static const EComparer NotLessOrAlmostEqual;
        static const EComparer Greater;
        static const EComparer NotGreater;
        static const EComparer GreaterOrEqual;
        static const EComparer NotGreaterOrEqual;
        static const EComparer GreaterOrAlmostEqual;
        static const EComparer NotGreaterOrAlmostEqual;
        static const EComparer SetMember;
        static const EComparer SetHasMember;
        static const EComparer Subset;
        static const EComparer ProperSubset;
        static const EComparer Superset;
        static const EComparer ProperSuperset;

        static bool IsForward(Type Type);
        static bool IncludeEquality(Type Type);

    public:
        EComparer() : mType(almostEqual), mNegative(false) {}
        EComparer(Type Value, bool Negative = false) : mType(Value), mNegative(Negative) {}

        EComparer& operator = (Type Value)          { mType = Value; return *this; }

        bool operator == (Type Value) const         { return mType == Value && !mNegative; }
        bool operator == (EComparer Another) const  { return mData == Another.mData; }
        bool operator () (CNumber Left, CNumber Right, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        bool operator () (const CVariant& Left, const CVariant& Right, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        operator Type() const { return mType; }

        Type GetType() const { return mType; }
        void SetType(Type Type) { mType = Type; }

        bool IsNegative() const { return mNegative; }
        void SetNegative(bool Value) { mNegative = Value; }
        // Negate this comparer.
        void Negate();
        // Reverse the direction of this comparer.
        void Reverse();
        // Return a reversed comparer.
        EComparer GetReverse() const;

        bool IsEqual() const            { return mType == EComparer::equal && !mNegative; }
        bool IsNotEqual() const         { return mType == EComparer::equal &&  mNegative; }
        bool IsAlmostEqual() const      { return mType == EComparer::almostEqual && !mNegative; }
        bool IsNotAlmostEqual() const   { return mType == EComparer::almostEqual &&  mNegative; }
        bool IncludeEquality() const;
        bool IsOrdered() const;
        bool IsSetComparison() const;
        // Return whether it doesn't matter if the left and right sides are swapped.
        bool IsSymmetric() const;
        // Is this comparison with a given number always true for all numbers?
        bool IsTautological(CNumber Right) const;
        // If this comparison with a given number always evaluates to the same result for all numbers, return the result. Otherwise, return unknown.
        ETriState UnconditionallyEvaluate(CNumber Right) const;
        size_t GetHash() const { return mData; }
        int GetArity() const { return 2; }
        const char* GetName() const;
        const char* GetSymbol() const;
        std::string Stringize(const std::string& Left, const std::string& Right) const;
        // Return the difference value between two given variants, plus an optional offset for strict inequality.
        // Always return 0 if this comparison holds true. 
        // This function may handle infinity and NaN incorrectly, but the base relation cost will be used for the heuristic anyway.
        CNumber GetGap(const CVariant& Left, const CVariant& Right, CNumber Tolerance = CNumber::GetDefaultTolerance(), CNumber Offset = 0) const;
        CNumber GetGap(const CVariant& Left, const CVariant& Right, const CFactDefinition& Definition) const;

    private:
        // Distance between two given variants. IN stands for "ignore negative".
        CNumber GetDistanceIN(const CVariant& Left, const CVariant& Right) const;

    private:
        union
        {
            struct // Anonymous structs are not standard, but MSVC, GCC, and Clang support this extension.
            {
                Type mType;
                unsigned char mNegative;
            };
            SSizeToUInt<sizeof(mType) + sizeof(mNegative)>::Type mData = 0;

            static_assert(sizeof(mData) == sizeof(mType) + sizeof(mNegative));
        };
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Hash functor for EComparer
    template <>
    struct hash<ExtendedGOAP::EComparer>
    {
        size_t operator () (ExtendedGOAP::EComparer Source) const noexcept
        {
            return Source.GetHash();
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}