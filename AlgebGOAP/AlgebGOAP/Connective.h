// Copyright 2025 Isaac Hsu

#pragma once

#include "Number.h"
#include "TriState.h"


namespace AlgebGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EConnective // Mock enum of logical connectives 
    {
    public:
        enum class Type : unsigned char
        {
            nil,
            logicalNOT,
            logicalAND,
            logicalOR,

            count
        };

        static constexpr Type nil           = Type::nil;
        static constexpr Type logicalNOT    = Type::logicalNOT;
        static constexpr Type logicalAND    = Type::logicalAND;
        static constexpr Type logicalOR     = Type::logicalOR;
        static constexpr Type invalid       = Type::count;

        static constexpr ETriState GetIdentity(Type Value);

    public:
        EConnective() = default;
        EConnective(Type Value) : mValue(Value) {}

        EConnective& operator = (Type Value) { mValue = Value; return *this; }
        operator Type() const { return mValue; }
        bool operator () (bool Left, bool Right) const;
        ETriState operator () (ETriState Left, ETriState Right) const;        

        bool IsValid() const    { return mValue >= Type::nil && mValue < Type::count; }
        bool IsNonEmpty() const { return mValue >  Type::nil && mValue < Type::count; }
        bool IsNil() const      { return mValue == Type::nil; }
        bool IsNOT() const      { return mValue == Type::logicalNOT; }
        bool IsAND() const      { return mValue == Type::logicalAND; }
        bool IsOR() const       { return mValue == Type::logicalOR; }
        bool IsCommutative() const;

        Type Get() const { return mValue; }
        int GetArity() const;
        ETriState GetIdentity() const { return GetIdentity(mValue); }
        const char* GetName() const;
        const char* GetFunctionName() const;
        const char* GetSymbol() const;
        std::string Stringize(const std::string& Left, const std::string& Right) const;

    private:
        Type mValue = nil;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    constexpr ETriState EConnective::GetIdentity(Type Value)
    {
        switch (Value)
        {
        case logicalAND:    return ETriState::yes;
        case logicalOR:     return ETriState::no;
        }

        return ETriState::unknown;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Hash functor for EConnective
    template <>
    struct hash<AlgebGOAP::EConnective>
    {
        size_t operator () (AlgebGOAP::EConnective Source) const noexcept
        {
            return std::hash<AlgebGOAP::EConnective::Type>{}(Source.Get());
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}