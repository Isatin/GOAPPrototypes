// Copyright 2025 Isaac Hsu

#pragma once

#include "Number.h"


namespace AlgebGOAP
{
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

        static constexpr CNumber GetLeftIdentity(Type Value);
        static constexpr CNumber GetRightIdentity(Type Value);
        static int VerifyOperators();

    public:
        EOperator() = default;
        EOperator(Type Value) : mValue(Value) {}

        EOperator& operator = (Type Value) { mValue = Value; return *this; }
        operator Type() const { return mValue; }
        CNumber operator () (CNumber Left, CNumber Right) const;

        bool IsNil() const { return mValue == Type::nil; }
        bool IsBoolean() const;
        bool IsCommutative() const;

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
    struct hash<AlgebGOAP::EOperator>
    {
        size_t operator () (AlgebGOAP::EOperator Source) const noexcept
        {
            return std::hash<AlgebGOAP::EOperator::Type>{}(Source.Get());
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}