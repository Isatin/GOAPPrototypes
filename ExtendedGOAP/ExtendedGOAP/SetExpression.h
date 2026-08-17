// Copyright 2026 Isaac Hsu

#pragma once

#include "ArgumentConverter.h"
#include "Expression.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Expression composed of set operations on set facts
    // It is implemented based on CExpression, but direct upcasting is not allowed to avoid mixing with numeric expressions.
    // TODO: Replace this class with one in a CAS library.
    class CSetExpression : private CExpression 
    {
        using Super = CExpression;
        friend class CFactAssignment;
        friend class CRelation;
        friend class CTermList;
        template <typename T>
        friend struct SArgumentConverter;
    public:
        CSetExpression() = default;
        explicit CSetExpression(const CSetFact& Argument);
        template <RSetArgument T>
        CSetExpression(EOperator Operator, T&& Argument) 
            : CExpression(Operator, SArgumentConverter<T>{}(Argument)) 
        {}
        template <RSetArgument TLeft, RSetArgument TRight>
        CSetExpression(EOperator Operator, TLeft&& Left, TRight&& Right) 
            : CExpression(Operator, SArgumentConverter<TLeft>{}(Left), SArgumentConverter<TRight>{}(Right)) 
        {}
        template <RSetArgument TLeft, RNumericArgument TRight>
        CSetExpression(EOperator Operator, TLeft&& Left, TRight&& Right)
            : CExpression(Operator, SArgumentConverter<TLeft>{}(Left), SArgumentConverter<TRight>{}(Right))
        {}

        using Super::ToString;
        using Super::IsEmpty;
        using Super::GetOperationCount;
        using Super::GrabLastOperation;
        using Super::Rearrange;

        bool IsEqual(const CSetExpression& Another) const { return Super::IsEqual(Another); }
        bool IsEquivalent(const CSetExpression& Another) const { return Super::IsEquivalent(Another); }
        [[nodiscard]] CSetExpression GetRearranged() const;

        void ReplaceFact(const CSetFact& Fact, const CSetExpression& Replacement);

    private:
        CExpression& ToBase() { return *this; }
        const CExpression& ToBase() const { return *this; }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <RSetArgument T>
    inline CSetExpression operator ~ (T&& Argument) { return {EOperator::setComplement, std::forward<T>(Argument)}; }

    template <RSetArgument TLeft, RSetArgument TRight>
    inline CSetExpression operator & (TLeft&& Left, TRight&& Right) { return {EOperator::setIntersection, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RSetArgument TLeft, RSetArgument TRight>
    inline CSetExpression operator | (TLeft&& Left, TRight&& Right) { return {EOperator::setUnion, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RSetArgument TLeft, RSetArgument TRight>
    inline CSetExpression operator - (TLeft&& Left, TRight&& Right) { return {EOperator::setDifference, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RSetArgument TLeft, RSetArgument TRight>
    inline CSetExpression operator ^ (TLeft&& Left, TRight&& Right) { return {EOperator::setSymmetricDifference, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    ///////////////////////////////////////////////////////////////////////////////////////////////
}