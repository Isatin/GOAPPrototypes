// Copyright 2024 Isaac Hsu

#pragma once

#include <string>


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EOperator // Mock enum of transformative operators 
    {
    public:
        enum class Type
        {
            nil,
            assignment,
            negation,       // Logical negation
            addition,
            multiplication,
        };

        static constexpr Type nil               = Type::nil;
        static constexpr Type assignment        = Type::assignment;
        static constexpr Type negation          = Type::negation;
        static constexpr Type addition          = Type::addition;
        static constexpr Type multiplication    = Type::multiplication;

    public:
        EOperator() = default;
        EOperator(Type Value) : mValue(Value) {}

        EOperator& operator = (Type Value) { mValue = Value; return *this; }
        operator Type() const   { return mValue; }

        Type Get() const        { return mValue; }
        int GetArity() const;
        const char* GetName() const;
        const char* GetSymbol() const;
        std::string Stringize(const std::string& Subject) const;

    private:
        Type mValue = nil;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}