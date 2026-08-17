// Copyright 2026 Isaac Hsu

#pragma once

#include "Number.h"


namespace ExtendedGOAP
{
    class ETriState;
    class CVariant;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EOperator // Mock enum of algebraic operators 
    {
    public:
        enum class Type : unsigned char
        {
            nil,
            addition,
            subtraction,
            multiplication,
            division,
            modulo,
            minus,
            absoluteValue,
            minimum,
            maximum,
            booleanNOT,
            booleanAND,
            booleanOR,
            matrixAddition,
            matrixSubtraction,
            matrixMultiplication,
            scalarMultiplication,
            setComplement,
            setIntersection,
            setUnion,
            setDifference,
            setSymmetricDifference,

            count
        };

        static constexpr Type nil               = Type::nil;
        static constexpr Type addition          = Type::addition;
        static constexpr Type subtraction       = Type::subtraction;
        static constexpr Type multiplication    = Type::multiplication;
        static constexpr Type division          = Type::division;
        static constexpr Type modulo            = Type::modulo;
        static constexpr Type minus             = Type::minus;
        static constexpr Type absoluteValue     = Type::absoluteValue;
        static constexpr Type minimum           = Type::minimum;
        static constexpr Type maximum           = Type::maximum;
        static constexpr Type booleanNOT        = Type::booleanNOT;
        static constexpr Type booleanAND        = Type::booleanAND;
        static constexpr Type booleanOR         = Type::booleanOR;
        static constexpr Type matrixAddition        = Type::matrixAddition;
        static constexpr Type matrixSubtraction     = Type::matrixSubtraction;
        static constexpr Type matrixMultiplication  = Type::matrixMultiplication;
        static constexpr Type scalarMultiplication  = Type::scalarMultiplication;
        static constexpr Type setComplement             = Type::setComplement;
        static constexpr Type setIntersection           = Type::setIntersection;
        static constexpr Type setUnion                  = Type::setUnion;
        static constexpr Type setDifference             = Type::setDifference;
        static constexpr Type setSymmetricDifference    = Type::setSymmetricDifference;

        static constexpr CNumber GetLeftIdentity(Type Value);
        static constexpr CNumber GetRightIdentity(Type Value);
        static int VerifyOperators();

    public:
        EOperator() = default;
        EOperator(Type Value) : mValue(Value) {}

        EOperator& operator = (Type Value) { mValue = Value; return *this; }
        operator Type() const { return mValue; }
        CNumber operator () (CNumber Left, CNumber Right) const;
        [[nodiscard]] CVariant operator () (CVariant&& Left, CVariant&& Right) const;

        bool IsNil() const { return mValue == Type::nil; }
        bool IsPostfix() const;
        bool IsBoolean() const;
        bool IsCommutative() const;
        ETriState IsScalar() const;
        bool IsMatrix() const;
        bool IsSet() const;
        bool ReturnMatrix() const;
        bool ReturnSet() const;
        EFactType ReturnType() const;

        Type Get() const { return mValue; }
        int GetArity() const;
        const char* GetName() const;
        const char* GetFunctionName() const;
        const char* GetSymbol() const;
        CNumber GetLeftIdentity() const { return GetLeftIdentity(mValue); }
        CNumber GetRightIdentity() const { return GetRightIdentity(mValue); }
        std::string Stringize(const std::string& Left, const std::string& Right) const;

        void SetNil() { mValue = nil; }

    private:
        Type mValue = nil;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    constexpr CNumber EOperator::GetLeftIdentity(Type Value)
    {
        switch (Value)
        {
        case addition:          return 0;
        case multiplication:    return 1;
        case booleanAND:        return true;
        case booleanOR:         return false;
        }

        return CNumber::Null;
    }

    constexpr CNumber EOperator::GetRightIdentity(Type Value)
    {
        switch (Value)
        {
        case addition:          return 0;
        case subtraction:       return 0;
        case multiplication:    return 1;
        case division:          return 1;
        case booleanAND:        return true;
        case booleanOR:         return false;
        }

        return CNumber::Null;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Hash functor for EOperator
    template <>
    struct hash<ExtendedGOAP::EOperator>
    {
        size_t operator () (ExtendedGOAP::EOperator Source) const noexcept
        {
            return std::hash<ExtendedGOAP::EOperator::Type>{}(Source.Get());
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}