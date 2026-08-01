// Copyright 2024 Isaac Hsu

#pragma once

#include <cmath>
#include <limits>
#include <ostream>
#include <string>

#include "SizeToInt.h"


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CNumber // Floating-point number wrapper with toleranced comparison
    {
    public:
        using BValue = double;                              // The basic type of the value
        using BInteger = SSizeToInt<sizeof(BValue)>::Type;  // The integer type that has the same size as the value type

        static const CNumber Null; // This represents an unset/invalid number but is still a kind of NaN.
        static const CNumber NaN;
        static const CNumber Infinity;
        static const CNumber DefaultTolerance; // Default tolerances used when no tolerance is specified (e.g., operator ==, <=, >=)

        static constexpr bool IsEquivalenceApproximate() { return true; }

    public:
        CNumber() = default;
        constexpr CNumber(BValue Source) : mValue(Source) {}
        template <typename T> requires std::is_arithmetic_v<T>
        constexpr CNumber(T Source) : mValue(static_cast<BValue>(Source)) {}

        CNumber& operator = (BValue Source) { mValue = Source; return *this; }

        operator BValue() const { return mValue; }
        CNumber operator - () const { return CNumber(-mValue); }

        CNumber& operator += (CNumber Another) { mValue += Another.mValue; return *this; }
        CNumber& operator -= (CNumber Another) { mValue -= Another.mValue; return *this; }
        CNumber& operator *= (CNumber Another) { mValue *= Another.mValue; return *this; }
        CNumber& operator /= (CNumber Another) { mValue /= Another.mValue; return *this; }

        friend std::ostream& operator << (std::ostream& Stream, CNumber Source) { return Stream << Source.ToString(); }

        BValue Get() const { return mValue; }
        BInteger GetBitwiseInteger() const { return reinterpret_cast<const BInteger&>(mValue); }
        CNumber GetPrior() const { return std::nextafter(mValue, -Infinity); }
        CNumber GetNext() const { return std::nextafter(mValue, Infinity); }
        bool IsNull() const { return IsIdentical(Null); }
        bool IsNaN() const { return std::isnan(mValue); }
        bool IsFinite() const { return std::isfinite(mValue); }
        bool IsInfinity() const { return std::isinf(mValue); }
        bool IsPositiveInfinity() const { return mValue == std::numeric_limits<BValue>::infinity(); }
        bool IsNegativeInfinity() const { return mValue == -std::numeric_limits<BValue>::infinity(); }
        // Bitwise equality even works for NaN.
        bool IsIdentical(CNumber Another) const;
        // Equality comparison using the underlying floating-point equality operator
        bool IsEqual(CNumber Another) const { return mValue == Another.mValue; }
        // Approximate equality comparison with tolerance
        bool IsAlmostEqual(CNumber Another, CNumber Tolerance = DefaultTolerance) const;
        // Equivalence comparison that could be with or without tolerance, based on the IsEquivalenceApproximate switch
        bool IsEquivalent(CNumber Another, CNumber Tolerance = DefaultTolerance) const;
        bool IsLessOrEquivalent(CNumber Another, CNumber Tolerance = DefaultTolerance) const;
        bool IsGreaterOrEquivalent(CNumber Another, CNumber Tolerance = DefaultTolerance) const;
        // Tolerance for approximate equality with another given number
        BValue CalcTolerance(CNumber Another, CNumber Tolerance = DefaultTolerance) const;

        std::string ToString() const;
        std::string ToSignedString() const;

    protected:
        BValue mValue{};
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline constexpr CNumber operator ""_n(unsigned long long int Value)    { return CNumber(static_cast<CNumber::BValue>(Value)); }
    inline constexpr CNumber operator ""_n(long double Value)               { return CNumber(static_cast<CNumber::BValue>(Value)); }

    inline bool operator == (CNumber Left, CNumber Right) { return Left.IsEquivalent(Right); }
    inline bool operator <  (CNumber Left, CNumber Right) { return Left.Get() < Right.Get(); }
    inline bool operator <= (CNumber Left, CNumber Right) { return operator < (Left, Right) || operator == (Left, Right); }
    inline bool operator >  (CNumber Left, CNumber Right) { return operator < (Right, Left); }
    inline bool operator >= (CNumber Left, CNumber Right) { return operator <= (Right, Left); }

    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator == (CNumber Left, T Right) { return Left.IsEquivalent(Right); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator <  (CNumber Left, T Right) { return Left.Get() < Right; }
    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator <= (CNumber Left, T Right) { return operator < (Left, Right) || operator == (Left, Right); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator >  (CNumber Left, T Right) { return operator < (Right, Left); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator >= (CNumber Left, T Right) { return operator <= (Right, Left); }

    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator == (T Left, CNumber Right) { return Right.IsEquivalent(Left); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator <  (T Left, CNumber Right) { return Left < Right.Get(); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator <= (T Left, CNumber Right) { return operator < (Left, Right) || operator == (Left, Right); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator >  (T Left, CNumber Right) { return operator < (Right, Left); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline bool operator >= (T Left, CNumber Right) { return operator <= (Right, Left); }

    inline CNumber operator + (CNumber Left, CNumber Right) { return Left.Get() + Right.Get(); }
    inline CNumber operator - (CNumber Left, CNumber Right) { return Left.Get() - Right.Get(); }
    inline CNumber operator * (CNumber Left, CNumber Right) { return Left.Get() * Right.Get(); }
    inline CNumber operator / (CNumber Left, CNumber Right) { return Left.Get() / Right.Get(); }

    template <typename T> requires std::is_arithmetic<T>::value
    inline CNumber operator + (CNumber Left, T Right) { return Left.Get() + Right; }
    template <typename T> requires std::is_arithmetic<T>::value
    inline CNumber operator - (CNumber Left, T Right) { return Left.Get() - Right; }
    template <typename T> requires std::is_arithmetic<T>::value
    inline CNumber operator * (CNumber Left, T Right) { return Left.Get() * Right; }
    template <typename T> requires std::is_arithmetic<T>::value
    inline CNumber operator / (CNumber Left, T Right) { return Left.Get() / Right; }

    template <typename T> requires std::is_arithmetic<T>::value
    inline CNumber operator + (T Left, CNumber Right) { return Left + Right.Get(); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline CNumber operator - (T Left, CNumber Right) { return Left - Right.Get(); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline CNumber operator * (T Left, CNumber Right) { return Left * Right.Get(); }
    template <typename T> requires std::is_arithmetic<T>::value
    inline CNumber operator / (T Left, CNumber Right) { return Left / Right.Get(); }

    inline bool CNumber::IsIdentical(CNumber Another) const
    {
        return GetBitwiseInteger() == Another.GetBitwiseInteger(); // NaN is not equal to NaN, so use bitwise equality instead.
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Hash functor for CNumber
    template <>
    struct hash<ArithGOAP::CNumber>
    {
        size_t operator () (const ArithGOAP::CNumber& Source) const noexcept
        {
            return hash<ArithGOAP::CNumber::BValue>{}(Source.Get());
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
