// Copyright 2024 Isaac Hsu

#pragma once

#include <string>
#include <vector>

#include "Number.h"
#include "Operator.h"


namespace ArithGOAP
{
    class CFact;
    class CState;
    class CStateDefinition;
    struct SBooleanFactOperation;
    struct SFactOperation;
    struct SInterval;
    struct SNumericFactOperation;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    enum class ETriStateCompletion
    {
        failed,
        complete,
        partial,
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CTransform // Operation with a numeric operand on a variable not included in the class
    {
    public:
        static const CTransform Nil;

        CTransform() = default;
        CTransform(EOperator Op, SNumber Value) : Operator(Op), Operand(Value) {}

        bool operator ! () const { return IsNil(); }
        operator bool() const { return !IsNil(); }

        std::string ToString() const; // For debug
        std::string ToString(const std::string& Subject) const; // For debug

        EOperator GetOperator() const { return Operator; }
        SNumber GetOperand() const { return Operand; }

        // Whether the operation is nil
        bool IsNil() const { return Operator == EOperator::nil; }
        // Apply this transformation to the given number
        void ApplyTo(SNumber& Number) const;
        // Apply this transformation to the given interval
        void ApplyTo(SInterval& Interval) const;
        // Solve clamp(x Operator Operand, Range) ∩ Target ≠ Ø for x
        ETriStateCompletion Neutralize(SInterval& Target, const SInterval& Range) const;

    private:
        EOperator Operator = EOperator::nil;
        SNumber Operand{};
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CEffect // Collection of transformation
    {
        friend class CAction;
    public:
        CEffect(const CStateDefinition& Def);
        std::string ToString() const; // For debug

        const CStateDefinition& GetDefinition() const { return mDefinition; }

        bool IsNil() const;
        int GetTransformAmount() const { return (int) mTransforms.size(); }
        int GetTransformCount() const;
        const CTransform& GetTransform(const CFact& Fact) const;
        const CTransform& GetTransform(int Index) const;
        bool SetTransform(const CFact& Fact, const CTransform& Transform);
        bool SetTransform(const CFact& Fact, SNumber Value);
        bool SetTransform(const SFactOperation& Operation);
        bool SetTransform(const SBooleanFactOperation& Operation);
        bool SetTransform(const SNumericFactOperation& Operation);

        // Apply this effect to the given state
        void ApplyTo(CState& State) const;

    private:
        void Expand(int Size);

    private:
        const CStateDefinition& mDefinition;
        std::vector<CTransform> mTransforms;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
    