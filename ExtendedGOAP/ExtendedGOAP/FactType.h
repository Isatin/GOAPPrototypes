// Copyright 2026 Isaac Hsu

#pragma once


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EFactType // Mock enum of fact types, also used as bit flags
    {
    public:
        enum Type : unsigned char
        {
            none        = 0,
            boolean     = 1 << 0,
            enumeration = 1 << 1,
            number      = 1 << 2,
            matrix      = 1 << 3,
            set         = 1 << 4,
        };

    public:
        EFactType() = default;
        EFactType(Type Value) : mValue(Value) {}

        operator Type() const { return mValue; }
        EFactType& operator |= (Type Right);

        bool IsScalar() const;

    private:
        Type mValue = none;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}