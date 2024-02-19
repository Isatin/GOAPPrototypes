// Copyright 2024 Isaac Hsu

#pragma once

#include <string>


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EOperator // Class representing enum of transformative operators 
    {
    public:
        enum Type
        {
            nil,
            assign,
            negation,       // logical negation
            addition,
            multiplication,
        };

        EOperator() = default;
        EOperator(Type Value) : mValue(Value) {}
        explicit EOperator(int Value) : mValue(static_cast<Type>(Value)) {}

        EOperator& operator = (int Value);
        operator Type() const { return mValue; }

        Type Get() const { return mValue; }
        int GetArity() const;
        const char* GetName() const;
        const char* GetSymbol() const;
        std::string ToString(const std::string& Subject) const;

        static const EOperator Assign;
        static const EOperator Negation;
        static const EOperator Addition;
        static const EOperator Multiplication;

    private:
        Type mValue = nil;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}