// Copyright 2025 Isaac Hsu

#pragma once

#include "ArgumentConverter.h"
#include "Expression.h"
#include "OperationConcept.h"


namespace AlgebGOAP
{
    class ETriState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Boolean expression composed of Boolean operations on Boolean constants or facts
    // It is implemented based on CExpression, but direct upcasting is not allowed to avoid mixing with numeric expressions.
    // TODO: Replace this class with one in a CAS library.
    class CBooleanExpression : private CExpression 
    {
        using Super = CExpression;
        friend class CFactAssignment;
        friend class CRelation;
        template <typename T>
        friend struct SArgumentConverter;
    public:
        CBooleanExpression() = default;
        explicit CBooleanExpression(bool Argument);
        explicit CBooleanExpression(const CBooleanFact& Argument);
        template <RBooleanArgument T>
        CBooleanExpression(EOperator Operator, T&& Argument) 
            : CExpression(Operator, SArgumentConverter<T>{}(Argument)) 
        {}
        template <RBooleanArgument TLeft, RBooleanArgument TRight>
        CBooleanExpression(EOperator Operator, TLeft&& Left, TRight&& Right) 
            : CExpression(Operator, SArgumentConverter<TLeft>{}(Left), SArgumentConverter<TRight>{}(Right)) 
        {}

        using Super::ToString;
        using Super::IsEmpty;
        using Super::GetOperationCount;
        using Super::GrabLastOperation;
        using Super::Rearrange;

        bool IsEqual(const CBooleanExpression& Another) const { return Super::IsEqual(Another); }
        bool IsEquivalent(const CBooleanExpression& Another) const { return Super::IsEquivalent(Another); }
        bool IsOnlyOneBool() const;
        bool IsBool(bool Value) const;
        [[nodiscard]] CBooleanExpression GetRearranged() const;
        ETriState Evaluate(const CStateBase& State) const;

        void ReplaceFact(const CBooleanFact& Fact, const CBooleanExpression& Replacement);

    private:
        CExpression& ToBase() { return *this; }
        const CExpression& ToBase() const { return *this; }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <RBooleanArgument T>
    inline CBooleanExpression operator ! (T&& Argument) { return {EOperator::booleanNOT, std::forward<T>(Argument)}; }
    template <RBooleanArgument TLeft, RBooleanArgument TRight>
    inline CBooleanExpression operator && (TLeft&& Left, TRight&& Right) { return {EOperator::booleanAND, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RBooleanArgument TLeft, RBooleanArgument TRight>
    inline CBooleanExpression operator || (TLeft&& Left, TRight&& Right) { return {EOperator::booleanOR, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    ///////////////////////////////////////////////////////////////////////////////////////////////
}