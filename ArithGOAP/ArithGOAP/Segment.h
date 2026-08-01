// Copyright 2024 Isaac Hsu

#pragma once

#include "Comparer.h"


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SAutocorrect {}; // Used for segments to indicate whether auto-flip is desired
    static constexpr SAutocorrect Autocorrect;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SSegment // Closed interval
    {
        using BValue = CNumber::BValue;

        static const SSegment Unset;
        static const SSegment Empty;
        static const SSegment Boundless;

        CNumber Minimum;
        CNumber Maximum;

    public:
        static constexpr const char* GetEmptySymbol() { return "[]"; }

    public:
        SSegment() : Minimum(Unset.Minimum), Maximum(Unset.Maximum) {} // Default-constructed segment is unset.
        SSegment(CNumber Value) : Minimum(Value), Maximum(Value) {}
        SSegment(CNumber Min, CNumber Max) : Minimum(Min), Maximum(Max) {}
        SSegment(CNumber Value1, CNumber Value2, SAutocorrect);
        SSegment(EComparer Comparer, CNumber Value);

        SSegment& operator =  (CNumber Value);
        SSegment& operator += (CNumber Value);
        SSegment& operator -= (CNumber Value);
        SSegment& operator *= (CNumber Value);
        SSegment& operator /= (CNumber Value);

        std::string ToString() const;
        std::string Stringize(const std::string& Subject) const;

        // Is this segment set?
        bool IsSet() const { return !IsUnset(); }
        // Is this segment not set?
        bool IsUnset() const { return IsIdentical(Unset); }
        // Is this segment empty?
        bool IsEmpty() const { return !(Minimum.Get() <= Maximum.Get()); }
        // Does this segment represent just a number instead of two different bounds?
        bool IsDegenerate(CNumber Tolerance = CNumber::DefaultTolerance) const;
        // Is any endpoint bounded?
        bool IsAnyBounded() const;
        // Is this segment indential to another?
        bool IsIdentical(const SSegment& Another) const;
        // Is this segment equal to another?
        bool IsEqual(const SSegment& Another) const;
        // Is this segment equivalent to another?
        bool IsEquivalent(const SSegment& Another, CNumber Tolerance = CNumber::DefaultTolerance) const;
        // Is this segment completely before another?
        bool IsAllLessThan(const SSegment& Another) const;
        // Is this segment completely after another?
        bool IsAllGreaterThan(const SSegment& Another) const;
        // Does this segment contain a given number?
        bool Contain(CNumber Number, CNumber Tolerance = CNumber::DefaultTolerance) const;
        // Distance between this segment and another
        CNumber GetGap(const SSegment& Another, CNumber Tolerance = CNumber::DefaultTolerance) const;
        // Clamp a given number to this segment.
        void Clamp(CNumber& Number) const;
        // Clamp a given segment to this segment.
        void Clamp(SSegment& Another) const;
        // Solve clamp(x, this) ∩ Another ≠ Ø for x.
        bool Unclamp(SSegment& Another, CNumber Tolerance = CNumber::DefaultTolerance) const;
        // Does this segment intersect with another?
        bool HasIntersection(const SSegment& Another, CNumber Tolerance = CNumber::DefaultTolerance) const;
        // Return the intersection with another given segment.
        SSegment GetIntersection(const SSegment& Another, CNumber Tolerance = CNumber::DefaultTolerance) const;

        // Intersect this segment with another given segment.
        bool Intersect(const SSegment& Another, CNumber Tolerance = CNumber::DefaultTolerance);
        // Swap the minimum and maximum.
        void Flip(); 
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline bool operator == (const SSegment& Left, const SSegment& Right) { return Left.IsEquivalent(Right); }
    inline bool operator <  (const SSegment& Left, const SSegment& Right) { return Left.IsAllLessThan(Right); }
    inline bool operator >  (const SSegment& Left, const SSegment& Right) { return Left.IsAllGreaterThan(Right); }
}
