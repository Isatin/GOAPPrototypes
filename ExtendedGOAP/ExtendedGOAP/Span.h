// Copyright 2026 Isaac Hsu

#pragma once

#include "Comparer.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SAutocorrect {}; // Used for spans to indicate whether auto-flip is desired
    static constexpr SAutocorrect Autocorrect;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class ESpanType // Mock enum of span endpoint types with approximate equality
    {
    public:
        enum class Type : signed char
        {
            open                    = static_cast<signed char>(0b0000'0000), // Not equal to the left/right endpoint
            closed                  = static_cast<signed char>(0b0000'0001), // Equal to the left/right endpoint
            approximate             = static_cast<signed char>(0b0000'0011), // Almost equal to the left/right endpoint
            approximationExclusive  = static_cast<signed char>(0b1000'0010)  // Not almost equal to the left/right endpoint
        };

        static constexpr Type open                      = Type::open;
        static constexpr Type closed                    = Type::closed;
        static constexpr Type approximate               = Type::approximate;
        static constexpr Type approximationExclusive    = Type::approximationExclusive;

        static_assert(approximationExclusive < open);
        static_assert(open < closed);
        static_assert(closed < approximate);

    public:
        ESpanType() = default;
        ESpanType(Type Source) : mValue(Source) {}

        ESpanType& operator = (ESpanType Source) { mValue = Source; return *this; }
        operator Type() const { return mValue; }

        Type Get() const { return mValue; }

        bool IsOpen() const                     { return mValue == open; }
        bool IsClosed() const                   { return mValue == closed; }
        bool IsApproximate() const              { return mValue == approximate; }
        bool IsApproximationExclusive() const   { return mValue == approximationExclusive; }
        bool IsInclusive() const { return static_cast<signed char>(mValue) & 1; }
        bool IsExclusive() const { return (static_cast<signed char>(mValue) & 1) == 0; }
        // ESpanType::open and close are complementary; approximate and approximationExclusive are complementary.
        bool IsComplement(ESpanType Another) const;
        const char* GetLeftSymbol() const;
        const char* GetRightSymbol() const;
        EComparer GetInfimumComparer() const;
        EComparer GetSuprenumComparer() const;

    private:
        Type mValue = ESpanType::open;
    };    
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SApproximateNumber
    {
        CNumber Subject;

    public:
        explicit SApproximateNumber(CNumber Source) : Subject(Source) {}

        CNumber::BValue Get() const { return Subject.Get(); }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline SApproximateNumber operator ~ (CNumber Argument) { return SApproximateNumber(Argument); }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SSpan // Extended interval with approximate equality, used for bounds of inequalities
    {
        CNumber Infimum;
        CNumber Supremum;
        ESpanType InfimumType;
        ESpanType SupremumType;

    public:
        static const SSpan Empty;
        static const SSpan Boundless; // Represent the extended real numbers, including positive and negative infinity.
        static const SSpan PositiveInfinityExclusion;
        static const SSpan NegativeInfinityExclusion;

        static constexpr const char* GetEmptySymbol() { return "[]"; }
        static CNumber::BPromoted GetTolerancedInfimum(CNumber Infimum, ESpanType InfimumType, CNumber Tolerance = CNumber::GetDefaultTolerance());
        static CNumber::BPromoted GetTolerancedSupremum(CNumber Supremum, ESpanType SupremumType, CNumber Tolerance = CNumber::GetDefaultTolerance());
        static bool CompareInfimumAndSupremum(CNumber Infimum, ESpanType InfimumType, CNumber Supremum, ESpanType SupremumType, CNumber Tolerance = CNumber::GetDefaultTolerance());
        static bool CompareInfimum(CNumber Left, ESpanType LeftType, CNumber Right, ESpanType RightType, CNumber Tolerance = CNumber::GetDefaultTolerance());
        static bool CompareSupremum(CNumber Left, ESpanType LeftType, CNumber Right, ESpanType RightType, CNumber Tolerance = CNumber::GetDefaultTolerance());

    public:
        SSpan(); // Default-constructed spans are empty.
        SSpan(CNumber Value); // Create a degenerate approximate interval: [~a, a~]
        SSpan(CNumber Infimum, CNumber Supremum); // Create an open interval: (a, b)
        SSpan(CNumber Infimum, SApproximateNumber Supremum); // Create a left-open, right-approximation-exclusive interval: (a, b~)
        SSpan(SApproximateNumber Infimum, CNumber Supremum); // Create a left-approximation-exclusive, right-open interval: (~a, b)
        SSpan(SApproximateNumber Infimum, SApproximateNumber Supremum); // Create an approximation-exclusive interval: (~a, b~)
        SSpan(CNumber Value1, CNumber Value2, SAutocorrect); // Create an open interval with given numbers and flip the endpoints if empty.
        SSpan(CNumber Infimum, CNumber Supremum, ESpanType EndpointType); // Create an interval with given endpoints.
        SSpan(CNumber Infimum, CNumber Supremum, ESpanType InfimumType, ESpanType SupremumType); // Create an interval with given endpoints.
        SSpan(CNumber Value1, CNumber Value2, ESpanType Value1Type, ESpanType Value2Type, SAutocorrect); // Create an interval with given endpoints and flip the endpoints if empty.
        SSpan(EComparer Comparer, CNumber Right); // Create an interval equivalent to an inequality from a given comparer and a right-hand-side value.

        SSpan& operator =  (CNumber Value);
        SSpan& operator += (CNumber Value);
        SSpan& operator -= (CNumber Value);
        SSpan& operator *= (CNumber Value);
        SSpan& operator /= (CNumber Value);

        std::string ToString() const;
        std::string Stringize(const std::string& Subject) const;

        bool IsEmpty(CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        bool IsUnbounded() const;
        bool IsInfimumOpen() const                      { return InfimumType == ESpanType::open; }
        bool IsInfimumClosed() const                    { return InfimumType == ESpanType::closed; }
        bool IsInfimumApproximate() const               { return InfimumType == ESpanType::approximate; }
        bool IsInfimumApproximationExclusive() const    { return InfimumType == ESpanType::approximationExclusive; }
        bool IsSupremumOpen() const                     { return SupremumType == ESpanType::open; }
        bool IsSupremumClosed() const                   { return SupremumType == ESpanType::closed; }
        bool IsSupremumApproximate() const              { return SupremumType == ESpanType::approximate; }
        bool IsSupremumApproximationExclusive() const   { return SupremumType == ESpanType::approximationExclusive; }
        bool IsIdentical(const SSpan& Another) const;
        bool IsEqual(const SSpan& Another) const;
        bool IsAlmostEqual(const SSpan& Another, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        bool Contain(CNumber Value, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        bool HasIntersection(const SSpan& Another, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        SSpan GetIntersection(const SSpan& Another, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;

        // Set this span based on a given comparer and a right-hand-side value. Return whether the update succeeds.
        bool Set(EComparer Comparer, CNumber Right);
        // Swap the two endpoints.
        void Flip(); 
        // Intersect with another given span. Return true if succeeded.
        bool Intersect(const SSpan& Another, CNumber Tolerance = CNumber::GetDefaultTolerance());
        // Merge with another given span. Return true if succeeded.
        bool Union(const SSpan& Another, CNumber Tolerance = CNumber::GetDefaultTolerance());
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline bool operator == (const SSpan& Left, const SSpan& Right) { return Left.IsAlmostEqual(Right); }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Used to create closed intervals with brackets, like CSpan[1, 2], or approximation intervals, like CSpan[~1_n, ~2_n].
    class
    {
    public:
        // Create a closed interval: [a, b]
        SSpan operator [] (CNumber Infimum, CNumber Supremum) const
        {
            return SSpan(Infimum, Supremum, ESpanType::closed, ESpanType::closed);
        }

        // Create a left-closed, right-approximate interval: [a, b~]
        SSpan operator [] (CNumber Infimum, SApproximateNumber Supremum) const
        {
            return SSpan(Infimum, Supremum.Subject, ESpanType::closed, ESpanType::approximate);
        }

        // Create a left-approximate, right-closed interval: [~a, b]
        SSpan operator [] (SApproximateNumber Infimum, CNumber Supremum) const
        {
            return SSpan(Infimum.Subject, Supremum, ESpanType::approximate, ESpanType::closed);
        }

        // Create an approximate interval: [~a, b~]
        SSpan operator [] (SApproximateNumber Infimum, SApproximateNumber Supremum) const
        {
            return SSpan(Infimum.Subject, Supremum.Subject, ESpanType::approximate, ESpanType::approximate);
        }
    }
    constexpr CSpan; // Dummy object used to create closed intervals in the common bracket notation
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
