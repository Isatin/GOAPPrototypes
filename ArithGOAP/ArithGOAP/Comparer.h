// Copyright 2024 Isaac Hsu

#pragma once

#include "Number.h"


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EComparer // Mock enum of equality and inclusive comparators
    {
    public:
        enum class Type
        {
            equal,
            lessOrEqual,
            greaterOrEqual,
        };

        static constexpr Type equal             = Type::equal;
        static constexpr Type lessOrEqual       = Type::lessOrEqual;
        static constexpr Type greaterOrEqual    = Type::greaterOrEqual;

        // Because a class cannot contain static constexpr data members of its own incomplete type,
        // separate static instances of the enum and the class are declared for different purposes.
        static const EComparer Equal;
        static const EComparer LessOrEqual;
        static const EComparer GreaterOrEqual;

    public:
        EComparer() = default;
        EComparer(Type Value) : mValue(Value) {}

        EComparer& operator = (Type Value) { mValue = Value; return *this; }
        operator Type() const { return mValue; }

        Type Get() const { return mValue; }
        const char* GetName() const;
        const char* GetSymbol() const;        

    private:
        Type mValue = equal;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Hash functor for EComparer
    template <>
    struct hash<ArithGOAP::EComparer>
    {
        size_t operator () (ArithGOAP::EComparer Source) const noexcept
        {
            return std::hash<ArithGOAP::EComparer::Type>()(Source.Get());
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}