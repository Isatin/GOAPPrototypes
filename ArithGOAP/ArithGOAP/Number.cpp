// Copyright 2024 Isaac Hsu

#include "Number.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const SNumber SNumber::Infinity(std::numeric_limits<PValue>::infinity());

bool SNumber::IsEquivalent(const SNumber& Another, PValue Tolerance) const
{
    return std::abs(Value - Another.Value) <= Tolerance;
}
///////////////////////////////////////////////////////////////////////////////////////////////////