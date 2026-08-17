// Copyright 2026 Isaac Hsu

#pragma once

#include "ArgumentConverter.h"
#include "Expression.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Matrix expression composed of matrix operations on matrix facts
    // It is implemented based on CExpression, but direct upcasting is not allowed to avoid mixing with numeric expressions.
    // TODO: Replace this class with one in a CAS library.
    class CMatrixExpression : private CExpression 
    {
        using Super = CExpression;
        friend class CFactAssignment;
        friend class CRelation;
        friend class CTermList;
        template <typename T>
        friend struct SArgumentConverter;
    public:
        CMatrixExpression() = default;
        explicit CMatrixExpression(const CMatrixFact& Argument);
        template <RMatrixArgument T>
        CMatrixExpression(EOperator Operator, T&& Argument) 
            : CExpression(Operator, SArgumentConverter<T>{}(Argument)) 
        {}
        template <RMatrixArgument TLeft, RMatrixArgument TRight>
        CMatrixExpression(EOperator Operator, TLeft&& Left, TRight&& Right) 
            : CExpression(Operator, SArgumentConverter<TLeft>{}(Left), SArgumentConverter<TRight>{}(Right)) 
        {}
        template <RMatrixArgument TLeft, RNumericArgument TRight>
        CMatrixExpression(EOperator Operator, TLeft&& Left, TRight&& Right)
            : CExpression(Operator, SArgumentConverter<TLeft>{}(Left), SArgumentConverter<TRight>{}(Right))
        {}

        using Super::ToString;
        using Super::IsEmpty;
        using Super::GetOperationCount;
        using Super::GrabLastOperation;
        using Super::Rearrange;

        bool IsEqual(const CMatrixExpression& Another) const        { return Super::IsEqual(Another); }
        bool IsEquivalent(const CMatrixExpression& Another) const   { return Super::IsEquivalent(Another); }
        [[nodiscard]] CMatrixExpression GetRearranged() const;

        void ReplaceFact(const CMatrixFact& Fact, const CMatrixExpression& Replacement);

    private:
        CExpression& ToBase() { return *this; }
        const CExpression& ToBase() const { return *this; }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <RMatrixArgument TLeft, RMatrixArgument TRight>
    inline CMatrixExpression operator + (TLeft&& Left, TRight&& Right) { return {EOperator::matrixAddition, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RMatrixArgument TLeft, RMatrixArgument TRight>
    inline CMatrixExpression operator - (TLeft&& Left, TRight&& Right) { return {EOperator::matrixSubtraction, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RMatrixArgument TLeft, RMatrixArgument TRight>
    inline CMatrixExpression operator * (TLeft&& Left, TRight&& Right) { return {EOperator::matrixMultiplication, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RMatrixArgument TLeft, RNumericArgument TRight>
    inline CMatrixExpression operator * (TLeft&& Left, TRight&& Right) { return {EOperator::scalarMultiplication, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RNumericArgument TLeft, RMatrixArgument TRight>
    inline CMatrixExpression operator * (TLeft&& Left, TRight&& Right) { return {EOperator::scalarMultiplication, std::forward<TRight>(Right), std::forward<TLeft>(Left)}; }
    ///////////////////////////////////////////////////////////////////////////////////////////////
}