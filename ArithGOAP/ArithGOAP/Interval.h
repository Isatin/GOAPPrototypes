// Copyright 2024 Isaac Hsu

#pragma once

#include <string>

#include "Comparer.h"
#include "Number.h"


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SInterval // closed interval
    {
        using PValue = SNumber::PValue;

        static const SInterval Unset;
        static const SInterval Empty;
        static const SInterval Boundless;

        SNumber Minimum;
        SNumber Maximum;

        SInterval() : Minimum(-SNumber::Infinity), Maximum(-SNumber::Infinity) {} // Default-constructed interval is considered as unset.
        SInterval(SNumber Value) : Minimum(Value), Maximum(Value) {}
        SInterval(SNumber Value1, SNumber Value2);
        SInterval(EComparer Cmp, SNumber Value);

        bool operator ! () const { return IsUnset(); }
        operator bool() const { return !IsUnset(); }

        bool operator == (const SInterval& Another) const;
        std::partial_ordering operator <=> (const SInterval& Another) const;

        SInterval& operator = (const SNumber& Another) { Minimum = Another; Maximum = Another; return *this; }
        SInterval& operator += (const SNumber& Another) { Minimum += Another; Maximum += Another; return *this; }
        SInterval& operator -= (const SNumber& Another) { Minimum -= Another; Maximum -= Another; return *this; }
        SInterval& operator *= (const SNumber& Another) { Minimum *= Another; Maximum *= Another; return *this; }
        SInterval& operator /= (const SNumber& Another) { Minimum /= Another; Maximum /= Another; return *this; }

        std::string ToString() const; // For debug
        std::string ToString(const std::string& Subject) const; // For debug

        // Whether this interval is not set
        bool IsUnset() const { return IsIdentical(Unset); }
        // Whether this interval is empty, including unset
        bool IsEmpty() const { return !(Minimum <= Maximum); }
        // Whether this interval just represents a number
        bool IsDegenerate() const;
        // Whether any endpoint is bounded
        bool IsBounded() const { return std::isfinite(Minimum) || std::isfinite(Maximum); }
        // Whether this interval is indential to another
        bool IsIdentical(const SInterval& Another) const { return Minimum.IsIdentical(Another.Minimum) && Maximum.IsIdentical(Another.Maximum); }
        // Whether this interval is within another
        bool IsWithin(const SInterval& Another) const { return Minimum >= Another.Minimum && Maximum <= Another.Maximum; }
        // Whether the whole interval is greater than another
        bool IsAllGreaterThan(const SInterval& Another) const { return Minimum > Another.Maximum; }
        // Whether the whole interval is less than another
        bool IsAllLessThan(const SInterval& Another) const { return Maximum < Another.Minimum; }
        // Whether this interval contains the given number
        bool Contain(SNumber Number) const { return Minimum <= Number && Number <= Maximum; }
        // The length of gap between this interval and another
        SNumber GetGap(const SInterval& Another) const;
        
        // Swap minimum and maximum 
        void Flip(); 
        // Clamp the given number by this interval
        void Clamp(SNumber& Number) const;
        // Clamp the given interval by this interval
        void Clamp(SInterval& Another) const;
        // Solve clamp(x, this) ∩ Another ≠ Ø for x
        bool Unclamp(SInterval& Another) const; 
        // Whether this interval has intersection with another
        bool HasIntersection(const SInterval& Another) const;
        // Become the intersection if intersected
        bool Intersect(const SInterval& Another);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
