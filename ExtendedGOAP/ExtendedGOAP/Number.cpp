// Copyright 2026 Isaac Hsu

#include <cassert>
#include <charconv>
#include <system_error>

#include "Number.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
static_assert(std::numeric_limits<CNumber::BValue>::has_signaling_NaN);
static_assert(std::numeric_limits<CNumber::BValue>::has_quiet_NaN);
static_assert(std::numeric_limits<CNumber::BValue>::has_infinity);

const CNumber CNumber::Null         = std::numeric_limits<BValue>::signaling_NaN();
const CNumber CNumber::NaN          = std::numeric_limits<BValue>::quiet_NaN();
const CNumber CNumber::Infinity     = std::numeric_limits<BValue>::infinity();
CNumber CNumber::DefaultTolerance   = std::numeric_limits<BValue>::epsilon() * 2;

void CNumber::SetDefaultTolerance(CNumber Value)
{
    assert(Value > 0);

    DefaultTolerance = Value;
}

CNumber CNumber::CalcULPDifference(CNumber Another) const
{
    if (mValue == Another.mValue) // For +0 vs. -0
    {
        return 0;
    }
    if (IsNaN() || Another.IsNaN()) // For NaN vs. any number
    {
        return CNumber::Null;
    }

    if (IsPositiveInfinity())
    {
        if (Another.IsNegativeInfinity())
        {
            return CNumber::Infinity;
        }
    }
    else if (IsNegativeInfinity())
    {
        if (Another.IsPositiveInfinity())
        {
            return -CNumber::Infinity;
        }
    }

    if (std::signbit(mValue))
    {
        if (std::signbit(Another.mValue)) // If this value and the given value are both negative
        {
            return Another.GetBitwiseInteger() - GetBitwiseInteger();
        }
        else // If this value is negative and the given value is non-negative
        {
            CNumber ThisDiff = GetBitwiseInteger() - CNumber(-0.0).GetBitwiseInteger();
            CNumber AnotherDiff = Another.GetBitwiseInteger() - CNumber(+0.0).GetBitwiseInteger();
            return -(ThisDiff + AnotherDiff);
        }
    }
    else
    {
        if (std::signbit(Another.mValue)) // If this value is non-negative and the given value is negative
        {
            CNumber ThisDiff = GetBitwiseInteger() - CNumber(+0.0).GetBitwiseInteger();
            CNumber AnotherDiff = Another.GetBitwiseInteger() - CNumber(-0.0).GetBitwiseInteger();
            return ThisDiff + AnotherDiff;
        }
        else // If this value and the given value are both non-negative
        {
            return GetBitwiseInteger() - Another.GetBitwiseInteger();
        }
    }
}

CNumber::BValue CNumber::CalcToleranceWith(CNumber Another, CNumber Tolerance) const
{
    BValue Magnitude = std::max(std::abs(mValue), std::abs(Another.mValue));
    return CalcTolerance(Magnitude, Tolerance.Get());
}

CNumber::BValue CNumber::CalcTolerance(CNumber Tolerance) const
{
    return CalcTolerance(std::abs(mValue), Tolerance.Get());
}

CNumber::BValue CNumber::CalcTolerance(BValue Magnitude, BValue AbsoluteTolerance)
{
    assert(AbsoluteTolerance >= 0);

    if (std::isfinite(Magnitude))
    {
        assert(Magnitude >= 0);

        BValue RelativeTolerance = Magnitude * AbsoluteTolerance;
        return std::max(RelativeTolerance, AbsoluteTolerance);
    }
    else
    {
        return 0;
    }
}

bool CNumber::IsAlmostEqual(CNumber Another, CNumber Tolerance) const
{
    if (IsEqual(Another))
    {
        return true;
    }

    BValue FinalTolerance = CalcToleranceWith(Another, Tolerance);
    return std::abs(mValue - Another.mValue) <= FinalTolerance;
}

bool CNumber::IsLessOrAlmostEqual(CNumber Another, CNumber Tolerance) const
{
    return mValue < Another.mValue || IsAlmostEqual(Another, Tolerance);
}

bool CNumber::IsGreaterOrAlmostEqual(CNumber Another, CNumber Tolerance) const
{
    return Another.IsLessOrAlmostEqual(*this, Tolerance);
}

std::string CNumber::ToString() const
{
#ifdef USE_UNICODE_SYMBOLS
    if (IsPositiveInfinity())
    {
        return "∞";
    }
    else if (IsNegativeInfinity())
    {
        return "-∞";
    }
#endif

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
#ifdef USE_UNICODE_SYMBOLS
    if (IsPositiveInfinity())
    {
        return "+∞";
    }
    else if (IsNegativeInfinity())
    {
        return "-∞";
    }
#endif

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
