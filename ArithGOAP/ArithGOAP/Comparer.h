// Copyright 2024 Isaac Hsu

#pragma once


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EComparer // Class representing enum of comparative operators 
    {
    public:
        enum Type
        {
            nil,
            equal,
            greaterOrEqual,
            lessOrEqual,
        };

        EComparer() = default;
        EComparer(Type Value) : mValue(Value) {}
        explicit EComparer(int Value) : mValue(static_cast<Type>(Value)) {}

        EComparer& operator = (int Value);
        operator Type() const { return mValue; }

        Type Get() const { return mValue; }
        const char* GetName() const;
        const char* GetSymbol() const;

        static const EComparer Equal;
        static const EComparer GreaterOrEqual;
        static const EComparer LessOrEqual;

    private:
        Type mValue = nil;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}