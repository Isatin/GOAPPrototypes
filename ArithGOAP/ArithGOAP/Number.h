// Copyright 2024 Isaac Hsu

#pragma once

#include <cmath>
#include <compare>
#include <limits>
#include <ostream>
#include <type_traits>


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SNumber // Floating-point number wrapper with error tolerance
    {
        using PValue = double; // Primitive type of value

        static const SNumber Infinity;

        PValue Value = static_cast<PValue>(0);

        SNumber() = default;
        SNumber(PValue Source) : Value(Source) {}

        SNumber operator = (PValue Source) { Value = Source; return *this; }

        operator PValue() const { return Value; }

        bool operator == (const SNumber& Another) const { return *this == Another.Value; }
        template <typename T>
        bool operator == (T Another) const;
        std::partial_ordering operator <=> (const SNumber& Another) const { return *this <=> Another.Value; }
        template <typename T>
        std::partial_ordering operator <=> (T Another) const;

        SNumber& operator += (const SNumber& Another) { Value += Another.Value; return *this; }
        SNumber& operator -= (const SNumber& Another) { Value -= Another.Value; return *this; }
        SNumber& operator *= (const SNumber& Another) { Value *= Another.Value; return *this; }
        SNumber& operator /= (const SNumber& Another) { Value /= Another.Value; return *this; }

        friend std::ostream& operator << (std::ostream& Stream, const SNumber& Source) { return Stream << Source.Value; }

        bool IsFinite() const { return std::isfinite(Value); }
        bool IsInfinity() const { return std::isinf(Value); }
        bool IsNaN() const { return std::isnan(Value); }
        bool IsIdentical(const SNumber& Another) const { return Value == Another.Value; }
        bool IsEquivalent(const SNumber& Another, PValue Tolerance = std::numeric_limits<PValue>::epsilon()) const;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    bool SNumber::operator == (T Another) const
    {
        static_assert(std::is_arithmetic_v<T>);
        // Using IsEquivalent makes Infinity unequal to each other, which allows the trick in SInterval::IsEmpty for [inf, inf] to be empty.
        return IsEquivalent(static_cast<PValue>(Another));
    }

    template <typename T>
    std::partial_ordering SNumber::operator <=> (T Another) const
    {
        static_assert(std::is_arithmetic_v<T>);

        if (*this == Another)
        {
            return std::partial_ordering::equivalent;
        }
        else if (Value > Another)
        {
            return std::partial_ordering::greater;
        }
        else  if (Value < Another)
        {
            return std::partial_ordering::less;
        }
        else
        {
            return std::partial_ordering::unordered;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
    // Hash functor for SNumber
    template <>
    struct hash<ArithGOAP::SNumber>
    {
        size_t operator () (const ArithGOAP::SNumber& Number) const noexcept
        {
            return hash<ArithGOAP::SNumber::PValue>{}(Number.Value);
        }
    };
}