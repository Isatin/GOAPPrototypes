// Copyright 2026 Isaac Hsu

#pragma once

#include "Number.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // CNumber::Null is a signaling NaN, and arithmetic operations convert it into a quiet NaN by default.
    // Hence, it can lose the nullity used to determine if a number is unset.
    // Unlike CNumber, this class can avoid this issue for null numbers.
    class CNullStableNumber : public CNumber
    {
    public:
        static const CNullStableNumber Null;

    public:
        CNullStableNumber() = default;
        CNullStableNumber(CNumber::BValue Source) : CNumber(Source) {}
        CNullStableNumber(CNumber Source) : CNumber(Source) {}

        CNullStableNumber& operator = (CNumber::BValue Source)  { mValue = Source; return *this; }
        CNullStableNumber& operator = (CNumber Source)          { mValue = Source; return *this; }

        CNullStableNumber& operator += (CNumber Right);
        CNullStableNumber& operator -= (CNumber Right);
        CNullStableNumber& operator *= (CNumber Right);
        CNullStableNumber& operator /= (CNumber Right);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Hash functor for CNullStableNumber
    template <>
    struct hash<ExtendedGOAP::CNullStableNumber>
    {
        size_t operator () (const ExtendedGOAP::CNullStableNumber& Source) const noexcept
        {
            return hash<ExtendedGOAP::CNullStableNumber::BValue>{}(Source.Get());
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}