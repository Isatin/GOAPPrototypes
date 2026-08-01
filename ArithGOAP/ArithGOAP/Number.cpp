// Copyright 2024 Isaac Hsu

#include <cassert>
#include <charconv>
#include <system_error>

#include "Number.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
static_assert(std::numeric_limits<CNumber::BValue>::has_signaling_NaN);
static_assert(std::numeric_limits<CNumber::BValue>::has_quiet_NaN);
static_assert(std::numeric_limits<CNumber::BValue>::has_infinity);

const CNumber CNumber::Null             = std::numeric_limits<BValue>::signaling_NaN();
const CNumber CNumber::NaN              = std::numeric_limits<BValue>::quiet_NaN();
const CNumber CNumber::Infinity         = std::numeric_limits<BValue>::infinity();
const CNumber CNumber::DefaultTolerance = std::numeric_limits<BValue>::epsilon() * 2;

bool CNumber::IsAlmostEqual(CNumber Another, CNumber Tolerance) const
{
    if (IsEqual(Another.mValue))
    {
        return true;
    }

    BValue FinalTolerance = CalcTolerance(Another, Tolerance);
    return std::abs(mValue - Another.mValue) <= FinalTolerance;
}

bool CNumber::IsEquivalent(CNumber Another, CNumber Tolerance) const
{
    if constexpr (IsEquivalenceApproximate())
    {
        return IsAlmostEqual(Another, Tolerance);
    }
    else
    {
        return mValue == Another.mValue;
    }
}

bool CNumber::IsLessOrEquivalent(CNumber Another, CNumber Tolerance) const
{
    if constexpr (IsEquivalenceApproximate())
    {
        return mValue < Another.mValue || IsAlmostEqual(Another, Tolerance);
    }
    else
    {
        return mValue <= Another.mValue;
    }
}

bool CNumber::IsGreaterOrEquivalent(CNumber Another, CNumber Tolerance) const
{
    if constexpr (IsEquivalenceApproximate())
    {
        return Another.IsLessOrEquivalent(*this, Tolerance);
    }
    else
    {
        return mValue >= Another.mValue;
    }
}

CNumber::BValue CNumber::CalcTolerance(CNumber Another, CNumber AbsoluteTolerance) const
{
    assert(AbsoluteTolerance >= 0);

    BValue Magnitude = std::max(std::abs(mValue), std::abs(Another.mValue));
    if (std::isfinite(Magnitude))
    {
        BValue RelativeTolerance = Magnitude * AbsoluteTolerance;
        return std::max<BValue>(RelativeTolerance, AbsoluteTolerance);
    }
    else
    {
        return 0;
    }
}

std::string CNumber::ToString() const
{
    const size_t BufferSize = 32;
    char Buffer[BufferSize] = {0};
    auto [End, Error] = std::to_chars(Buffer, Buffer + sizeof(Buffer), mValue);

    if (Error == std::errc{})
    {
        return std::string(Buffer, End);
    }
    else
    {
        return std::make_error_code(Error).message();
    }
}

std::string CNumber::ToSignedString() const
{
    const size_t BufferSize = 32;
    char Buffer[BufferSize] = {(mValue < 0 ? '-' : '+'), 0};
    auto [End, Error] = std::to_chars(Buffer + 1, Buffer + sizeof(Buffer) - 1, std::abs(mValue));

    if (Error == std::errc{})
    {
        return std::string(Buffer, End);
    }
    else
    {
        return std::make_error_code(Error).message();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
