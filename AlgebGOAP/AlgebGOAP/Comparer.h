// Copyright 2025 Isaac Hsu

#pragma once

#include "Number.h"


namespace AlgebGOAP
{
    class CFactDefinition;
    class ETriState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CDummyEqualComparer // Dummy class used to invoke EComparer::Equal.GetSymbol()
    {
        friend class EComparer;
    public:
        const char* GetSymbol() const { return "=="; }

    private:
        CDummyEqualComparer() = default;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EComparer // Mock enum of comparative operators with a negative flag
    {
    public:
        enum class Type : unsigned char
        {
            almostEqual,
            less,
            lessOrEqual,
            lessOrAlmostEqual,
            greater,
            greaterOrEqual,
            greaterOrAlmostEqual,
        };

        static constexpr Type almostEqual           = Type::almostEqual;
        static constexpr Type less                  = Type::less;
        static constexpr Type lessOrEqual           = Type::lessOrEqual;
        static constexpr Type lessOrAlmostEqual     = Type::lessOrAlmostEqual;
        static constexpr Type greater               = Type::greater;
        static constexpr Type greaterOrEqual        = Type::greaterOrEqual;
        static constexpr Type greaterOrAlmostEqual  = Type::greaterOrAlmostEqual;

        static const CDummyEqualComparer Equal;
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

        static bool IsForward(Type Type);

    public:
        EComparer() : mType(almostEqual), mNegative(false) {}
        EComparer(Type Value, bool Negative = false) : mType(Value), mNegative(Negative) {}

        EComparer& operator = (Type Value)          { mType = Value; return *this; }

        bool operator == (Type Value) const         { return mType == Value && !mNegative; }
        bool operator == (EComparer Another) const  { return mData == Another.mData; }
        bool operator () (CNumber Left, CNumber Right, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        operator Type() const { return mType; }

        bool IsNegative() const { return mNegative; }
        void SetNegative(bool Value) { mNegative = Value; }
        // Negate this comparer.
        void Negate();
        // Reverse the direction of this comparer.
        void Reverse();
        // Return a reversed comparer.
        EComparer GetReverse() const;

        bool IsAlmostEqual() const      { return mType == EComparer::almostEqual && !mNegative; }
        bool IsNotAlmostEqual() const   { return mType == EComparer::almostEqual &&  mNegative; }
        bool IncludeEquality() const;
        // Return whether it doesn't matter if the left and right sides are swapped.
        bool IsSymmetric() const;
        // Is this comparison with a given number always true for all numbers?
        bool IsTautological(CNumber Right) const;
        // If this comparison with a given number always evaluates to the same result for all numbers, return the result. Otherwise, return unknown.
        ETriState UnconditionallyEvaluate(CNumber Right) const;
        Type GetType() const { return mType; }
        size_t GetHash() const { return mData; }
        int GetArity() const { return 2; }
        const char* GetName() const;
        const char* GetSymbol() const;
        std::string Stringize(const std::string& Left, const std::string& Right) const;
        // Return the difference between two given numbers. 
        // Always return 0 if this comparison holds true. 
        // Otherwise, return the absolute difference, plus an optional offset if this comparer is a strict inequality.
        // This function may handle infinity and NaN incorrectly, but the base relation cost will be used for the heuristic anyway.
        CNumber GetGap(CNumber Left, CNumber Right, CNumber Tolerance = CNumber::GetDefaultTolerance(), CNumber Offset = 0) const;
        CNumber GetGap(CNumber Left, CNumber Right, const CFactDefinition& Definition) const;

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
    struct hash<AlgebGOAP::EComparer>
    {
        size_t operator () (AlgebGOAP::EComparer Source) const noexcept
        {
            return Source.GetHash();
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}