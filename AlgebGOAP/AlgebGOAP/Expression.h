// Copyright 2025 Isaac Hsu

#pragma once

#include <unordered_map>
#include <vector>

#include "OperationConcept.h"
#include "Operator.h"


namespace AlgebGOAP
{
    class CBitVector;
    class CFact;
    class CFactDefinition;
    class CStateBase;
    struct STerm;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    enum class EOperand : unsigned char // Enum of operand types in algebraic expressions
    {
        null,
        fact,
        operation,
        constant
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SFactIndex // Fact index wrapper for COperand's ctors to avoid ambiguity
    {
        int Index = InvalidIndex;

    public:
        explicit SFactIndex(size_t Index) : Index(static_cast<int>(Index)) {}
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SOperationIndex // Operation index wrapper for COperand's ctors to avoid ambiguity
    {
        int Index = InvalidIndex;

    public:
        explicit SOperationIndex(size_t Index) : Index(static_cast<int>(Index)) {}
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class COperand // Operand of algebraic operations that can be a fact reference, an operation reference, or a constant
    {
        friend class CBooleanOperand;
        friend class CExpression;
        friend class CFactAssignment;
        friend class CRelation;
        friend struct SOperation;
        template <typename T>
        friend struct SArgumentConverter;
        friend struct SRepeatedOperation;
    public:
        COperand() = default;
        COperand(SFactIndex Index);
        COperand(SOperationIndex Index);
        COperand(const CNumericFact& Fact);
        COperand(CNumber Value);
        template <typename T> requires std::is_arithmetic_v<T>
        COperand(T Value) : mType(EOperand::constant), mConstant(Value) {}

        bool operator == (const COperand& Another) const;

        EOperand GetType() const { return mType; }
        // PREREQUISITE: This is a fact.
        int GrabFactIndex() const;
        // PREREQUISITE: This is a operation reference.
        int GrabOperationIndex() const;
        // PREREQUISITE: This is a constant.
        CNumber GrabConstant() const;

        bool IsNull() const                 { return mType == EOperand::null; }
        bool IsFact() const                 { return mType == EOperand::fact; }
        bool IsFact(int Index) const        { return IsFact() && mIndex == Index; }
        bool IsOperation() const            { return mType == EOperand::operation; }
        bool IsOperation(int Index) const   { return IsOperation() && mIndex == Index; }
        bool IsConstant() const             { return mType == EOperand::constant; }
        bool IsConstant(CNumber Value) const{ return IsConstant() && mConstant == Value; }

    private:
        COperand(const CFact& Fact);

        void Set(const COperand& Operand);
        void SetNull();
        void SetFact(int FactIndex);
        void SetOperation(int OperationIndex);
        void SetOperationIndex(int OperationIndex);
        void SetConstant(CNumber Value);
        void ShiftOperationIndex(int Offset);

    public:
        static const COperand Null;

    private:
        EOperand mType = EOperand::null;
        union
        {
            int mIndex;
            CNumber mConstant = CNumber::Null;
        };
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SOperation // Algebraic operation storing an operator, a left operand, and a right operand
    {
        EOperator Operator;
        COperand Left;  // NOTE: The left operand must be null in a unary operation.
        COperand Right; // NOTE: The right operand must not be null in an operation.

    public:
        SOperation() = default;
        SOperation(EOperator Operator, const COperand& Argument)
            : Operator(Operator), Right(Argument)
        {}
        SOperation(EOperator Operator, const COperand& Left, const COperand& Right)
            : Operator(Operator), Left(Left), Right(Right)
        {}

        bool operator == (const SOperation& Another) const;

        bool HasOperator()      const { return Operator != EOperator::nil; }
        bool IsAddition()       const { return Operator == EOperator::addition; }
        bool IsSubtraction()    const { return Operator == EOperator::subtraction; }
        bool IsMultiplication() const { return Operator == EOperator::multiplication; }
        bool IsDivision()       const { return Operator == EOperator::division; }
        bool IsModulo()         const { return Operator == EOperator::modulo; }
        bool IsMinus()          const { return Operator == EOperator::minus; }
        bool IsBooleanNOT()     const { return Operator == EOperator::booleanNOT; }
        bool IsBooleanAND()     const { return Operator == EOperator::booleanAND; }
        bool IsBooleanOR()      const { return Operator == EOperator::booleanOR; }
        bool IsOnlyOneFact() const;
        bool IsOnlyOneOperationReference() const;
        bool IsOnlyOneConstant() const;
        bool IsOnlyOneOperand() const;
        bool IsOperation(int OperationIndex) const;
        // PREREQUISITE: IsOnlyOneFact returns true.
        int GrabTheOnlyFactIndex() const;
        // PREREQUISITE: IsOnlyOneConstant returns true.
        CNumber GrabTheOnlyConstant() const;
        // PREREQUISITE: The capacity of the given container is greater than the maximum referenced operation index.
        void MarkOperationFlags(std::vector<bool>& oFlags);

        void SetTo(const COperand& Operand);
        void SetTo(CNumber Value);
        void ShiftOperationIndex(int Offset);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Algebraic expression composed of facts and constants that are combined with operators
    // NOTE: It stores only fact indexes and doesn't check if the facts come from the same CFactDefinition.
    // TODO: Replace this class with one in a CAS library.
    class CExpression
    {
        friend class CBooleanExpression;
        friend class CRelation;
    public:
        static constexpr const char* GetEmptySymbol() { return "()"; }

        static const CExpression Empty;

    public:
        CExpression() = default;
        explicit CExpression(const COperand& Operand);
        CExpression(EOperator Operator, const COperand& Operand);
        CExpression(EOperator Operator, const CExpression&  Expression);
        CExpression(EOperator Operator,       CExpression&& Expression);
        CExpression(EOperator Operator, const COperand& Left, const COperand& Right);
        CExpression(EOperator Operator, const COperand& Left, const CExpression&  Right);
        CExpression(EOperator Operator, const COperand& Left,       CExpression&& Right);
        CExpression(EOperator Operator, const CExpression& Left, const COperand& Right);
        CExpression(EOperator Operator,       CExpression&& Left, const COperand& Right);
        CExpression(EOperator Operator, const CExpression&  Left, const CExpression& Right);
        CExpression(EOperator Operator,       CExpression&& Left, const CExpression& Right);
        CExpression(EOperator Operator, const CExpression&  Left,      CExpression&& Right);
        CExpression(EOperator Operator,       CExpression&& Left,      CExpression&& Right);
        CExpression(EOperator Operator, const std::vector<int>& FactIndexes, const std::vector<int>& OperationIndexes, CNumber Constant = CNumber::Null, int StartOperationIndex = 0);

        std::string ToString(const CFactDefinition& Definition) const;
        std::string ToString() const; // NOTE: Facts are printed as indexes with an 'F' prefix, without providing the fact definition.
        std::string ToString(int StartOperationIndex) const;

        bool IsEmpty() const;
        bool IsFactUsed(int FactIndex) const;
        bool IsEqual(const CExpression& Another) const;
        // PREREQUISITE: Call Rearrange on both beforehand.
        bool IsEquivalent(const CExpression& Another) const;
        bool IsOnlyOneFact() const;
        bool IsOnlyOneConstant() const;
        bool IsInfinity() const;        
        bool IsConstant(CNumber Value) const;
        bool IsOnlyOneOperand() const;
        bool IsTransposable() const;
        int GetOperationCount() const { return static_cast<int>(mOperations.size()); }
        int AddUpArity() const;
        // Return a bit vector indicating which facts are referenced in this expression.
        CBitVector GetUsedFactBits() const;
        // PREREQUISITE: IsOnlyOneFact returns true.
        int GrabTheOnlyFactIndex() const;
        // PREREQUISITE: IsOnlyOneConstant returns true.
        CNumber GrabTheOnlyConstant() const;
        // PREREQUISITE: IsOnlyOneOperand returns true.
        const COperand& GrabTheOnlyOperand() const;
        // PREREQUISITE: Call Rearrange beforehand.
        CNumber GetConstantTerm() const;
        // PREREQUISITE: This expression is not empty.
        const SOperation& GrabLastOperation() const { return mOperations.back(); }
        int CountUnusedOperations() const;
        int CountReferencedUnivariateOperations() const;
        bool ValidateOperation(const SOperation& Operation) const;
        [[nodiscard]] std::vector<int> GetInvalidOperationIndexes() const;
        // Evaluate this expression for a given state.
        CNumber Evaluate(const CStateBase& State) const;
        [[nodiscard]] CExpression GetSubExpression(int OperationIndex) const;
        [[nodiscard]] CExpression GetRearranged() const;
        // Split this expression into terms and return the constant term.
        // PREREQUISITE: Call Rearrange beforehand.
        CNumber SplitInto(std::vector<STerm>& oTerms, CNumber OuterScale = 1) const;

        void Clear() { mOperations.clear(); }
        void SetTo(const COperand& Operand);
        void Prepend(EOperator Operator);
        void Prepend(EOperator Operator, const COperand& Left);
        void Prepend(EOperator Operator, const CExpression& Left);
        void Append(EOperator Operator, const COperand& Right);
        void Append(EOperator Operator, const CExpression&  Right);
        void Append(EOperator Operator,       CExpression&& Right);
        void ReplaceFact(const CNumericFact& Fact, CNumber Replacement);
        void ReplaceFact(const CNumericFact& Fact, const CNumericFact& Replacement);
        void ReplaceFact(const CNumericFact& Fact, const CExpression& Replacement);
        void ReplaceFact(int FactIndex, const COperand& Replacement);
        void ReplaceFact(int FactIndex, const CExpression& Replacement);
        template <template <typename...> class TContainer, typename... TArgs>
        void ReplaceFacts(const TContainer<TArgs...>& Replacements);
        int RemoveUnusedOperations();
        // Rearrange the operands into a consistent order so that equivalent expressions yield the same form.
        void Rearrange();

    private:
        struct SRepeatedOperation;
        struct SSummation;
        struct SFraction;

        int AddUpArity(const std::vector<int>& OperationIndexes) const;
        std::vector<int> GetUsedOperationIndexes() const;
        std::vector<int> GetUsedOperationIndexes(int StartOperationIndex) const;
        std::string ToString(const std::vector<int>& OperationIndexes) const;
        bool ValidateOperation(const SOperation& Operation, int OperationLimit) const;
        void SortSubExpressions(std::vector<int>& OperationIndexes) const;

        void ShiftOperationIndexes(int Offset, int StartOperationIndex = 0);
        // PREREQUISITE: This is a Boolean expression.
        void Negate();
        // PREREQUISITE: The last oeprator is unary or nil.
        void EraseLastOperator();
        void ReplaceFact(const CFact& Fact, const COperand& Replacement);
        void ReplaceFact(const CFact& Fact, const CExpression& Replacement);
        void ApplyDeMorgansLaws();
        void RemoveDoubleNegations();
        void FactorOutNegations();
        void RearrangeFractions();
        void RearrangeSubtractions();
        void RearrangeCommutativeOperations();
        void RearrangeBooleanOperations();
        int RemoveOperation(int OperationIndex);
        int RemoveOperations(int StartOperationIndex, int EndOperationIndex);
        void RemoveIdentityElements();
        void RemoveConstantOperations();
        void RemoveIneffectiveOperations();
        [[nodiscard]] std::unordered_map<int, int> ReplaceOperation(int OperationIndex, CExpression&& Replacement);
        void DistributeOperationsOverOthers();

    private:
        // NOTE: Operations can only reference preceding operations, not subsequent ones. 
        // NOTE: Operations can be referenced multiple times.
        // TODO: We may use std::map instead to avoid updating operation indexes when adding or removing operations, but it's probably slower.
        std::vector<SOperation> mOperations;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <RNumericArgument T> requires RNumericOperation<T>
    inline CExpression operator - (T&& Argument) { return {EOperator::minus, std::forward<T>(Argument)}; }

    template <RNumericArgument T> requires RNumericOperation<T>
    inline CExpression abs(T&& Argument) { return {EOperator::absoluteValue, std::forward<T>(Argument)}; }

    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CExpression operator + (TLeft&& Left, TRight&& Right) { return {EOperator::addition, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CExpression operator - (TLeft&& Left, TRight&& Right) { return {EOperator::subtraction, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CExpression operator * (TLeft&& Left, TRight&& Right) { return {EOperator::multiplication, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CExpression operator / (TLeft&& Left, TRight&& Right) { return {EOperator::division, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CExpression operator % (TLeft&& Left, TRight&& Right) { return {EOperator::modulo, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }

    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CExpression min(TLeft&& Left, TRight&& Right) { return {EOperator::minimum, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RNumericArgument TLeft, RNumericArgument TRight> requires RNumericOperation<TLeft, TRight>
    inline CExpression max(TLeft&& Left, TRight&& Right) { return {EOperator::maximum, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }

    template <template <typename...> class TContainer, typename... TArgs>
    void CExpression::ReplaceFacts(const TContainer<TArgs...>& Replacements)
    {
        for (const auto& [FactIndex, Replacement] : Replacements)
        {
            ReplaceFact(FactIndex, Replacement);
        }
    }
}
