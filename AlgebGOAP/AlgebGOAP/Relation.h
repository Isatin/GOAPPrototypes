// Copyright 2025 Isaac Hsu

#pragma once

#include <cassert>

#include "BooleanExpression.h"
#include "Comparer.h"


namespace AlgebGOAP
{
    class CFactDefinition;
    class CState;
    class ETriState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Relational expression comparing two algebraic expressions
    // TODO: Replace this class with one in a CAS library.
    class CRelation
    {
        friend class CFormula;
    public:
        CRelation() = default;
        template <RBooleanArgument T> requires RBooleanOperation<T>
        CRelation(T&& Argument);
        template <RBooleanArgument TLeft, RBooleanArgument TRight>
        CRelation(TLeft&& Left, EComparer Comp, TRight&& Right);
        template <REnumerationArgument TLeft, REnumerationArgument TRight>
        CRelation(TLeft&& Left, TRight&& Right, EComparer Comp);
        template <RNumericArgument TLeft, RNumericArgument TRight>
        CRelation(EComparer Comp, TLeft&& Left, TRight&& Right);

        bool operator == (const CRelation& Another) const { return IsEqual(Another); }

        EComparer GetComparer() const { return mComparer; }
        CExpression& GetLeft() { return mLeft; }
        const CExpression& GetLeft() const { return mLeft; }
        CExpression& GetRight() { return mRight; }
        const CExpression& GetRight() const { return mRight; }

        std::string ToString() const;
        std::string ToString(const CFactDefinition& Definition) const;
        bool IsEmpty() const { return mLeft.IsEmpty() || mRight.IsEmpty(); }
        bool IsEqual(const CRelation& Another) const;
        // PREREQUISITE: Call Rearrange beforehand.
        bool IsEquivalent(const CRelation& Another) const;
        // PREREQUISITE: The right side is only a constant.
        bool IsTautological() const;
        // PREREQUISITE: The right side is only a constant.
        ETriState UnconditionallyEvaluate() const;
        int AddUpArity() const;
        // Calculate the difference between both expressions evaluated for a given state.
        CNumber GetGap(const CStateBase& State) const;
        CBitVector GetUsedFactBits() const;
        // Evaluate both expressions.
        std::pair<CNumber, CNumber> EvaluateBothSides(const CStateBase& State) const;
        // Return evaluated result of this relation. Return unknown if the evaluation fails (e.g. due to unset facts).
        ETriState Evaluate(const CStateBase& State) const;
        [[nodiscard]] CRelation GetRearranged() const;

        void ReplaceFact(const CNumericFact& Fact, CNumber Replacement);
        void ReplaceFact(const CNumericFact& Fact, const CNumericFact& Replacement);
        void ReplaceFact(const CNumericFact& Fact, const CExpression& Replacement);
        void ReplaceFact(const CBooleanFact& Fact, const CBooleanExpression& Replacement);
        // Rearrange the operands into a consistent order so that equivalent relations yield the same form.
        void Rearrange();
        void SwapSides();

    private:
        void Negate();
        void RearrangeNegations();
        // PREREQUISITE: Both sides are transposable.
        void RearrangeTerms();
        void RearrangeNonTransposableSides();
        void CancelOutNegationsOnBothSides();
        void ConvertNegationToInequation();
        void SortComparands();

    private:
        EComparer mComparer = EComparer::almostEqual;
        CExpression mLeft;
        CExpression mRight;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <RBooleanArgument T> requires RBooleanOperation<T>
    CRelation::CRelation(T&& Argument)
        : mComparer(EComparer::almostEqual)
        , mLeft(SArgumentConverter<T>{}(Argument))
        , mRight(SArgumentConverter<bool>{}(true))
    {}

    template <RBooleanArgument TLeft, RBooleanArgument TRight>
    CRelation::CRelation(TLeft&& Left, EComparer Comp, TRight&& Right)
        : mComparer(Comp)
        , mLeft(SArgumentConverter<TLeft>{}(Left))
        , mRight(SArgumentConverter<TRight>{}(Right))
    {
        assert(Comp.GetType() == EComparer::almostEqual); // Only == and != are allowed for Boolean relational expressions.
    }

    template <REnumerationArgument TLeft, REnumerationArgument TRight>
    CRelation::CRelation(TLeft&& Left, TRight&& Right, EComparer Comp)
        : mComparer(Comp)
        , mLeft(SArgumentConverter<TLeft>{}(Left))
        , mRight(SArgumentConverter<TRight>{}(Right))
    {
        assert(Comp.GetType() == EComparer::almostEqual); // Only == and != are allowed for operations with enumerations.
    }

    template <RNumericArgument TLeft, RNumericArgument TRight>
    CRelation::CRelation(EComparer Comp, TLeft&& Left, TRight&& Right)
        : mComparer(Comp)
        , mLeft(SArgumentConverter<TLeft>{}(Left))
        , mRight(SArgumentConverter<TRight>{}(Right))
    {}

    template <RBooleanArgument TLeft, RBooleanArgument TRight>
    inline CRelation operator == (TLeft&& Left, TRight&& Right) { return {std::forward<TLeft>(Left), EComparer::AlmostEqual, std::forward<TRight>(Right)}; }
    template <RBooleanArgument TLeft, RBooleanArgument TRight>
    inline CRelation operator != (TLeft&& Left, TRight&& Right) { return {std::forward<TLeft>(Left), EComparer::NotAlmostEqual, std::forward<TRight>(Right)}; }

    template <REnumerationArgument TLeft, REnumerationArgument TRight> requires REnumerationOperation<TLeft, TRight>
    inline CRelation operator == (TLeft&& Left, TRight&& Right) { return {std::forward<TLeft>(Left), std::forward<TRight>(Right), EComparer::AlmostEqual}; }
    template <REnumerationArgument TLeft, REnumerationArgument TRight> requires REnumerationOperation<TLeft, TRight>
    inline CRelation operator != (TLeft&& Left, TRight&& Right) { return {std::forward<TLeft>(Left), std::forward<TRight>(Right), EComparer::NotAlmostEqual}; }

    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CRelation operator == (TLeft&& Left, TRight&& Right) { return {EComparer::AlmostEqual, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CRelation operator != (TLeft&& Left, TRight&& Right) { return {EComparer::NotAlmostEqual, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }

    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CRelation operator <  (TLeft&& Left, TRight&& Right) { return {EComparer::Less, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CRelation operator <= (TLeft&& Left, TRight&& Right) { return {EComparer::LessOrAlmostEqual, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }

    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CRelation operator >  (TLeft&& Left, TRight&& Right) { return {EComparer::Greater, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CRelation operator >= (TLeft&& Left, TRight&& Right) { return {EComparer::GreaterOrAlmostEqual, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
