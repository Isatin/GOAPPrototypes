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
    class CFactDefinition;
    struct SFactOperation;
    struct SSegment;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    enum class ETriStateCompletion
    {
        failed,
        complete,
        partial,
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CTransform // Operation on a constant number and an external variable
    {
    public:
        static const CTransform Nil;

    public:
        CTransform() = default;
        CTransform(EOperator Op, CNumber Value) : Operator(Op), Operand(Value) {}

        std::string ToString() const; // For debugging
        std::string Stringize(const std::string& Subject) const; // For debugging

        EOperator GetOperator() const { return Operator; }
        CNumber GetOperand() const { return Operand; }

        // Is the operator nil?
        bool IsNil() const { return Operator == EOperator::nil; }
        // Apply this transformation to a given number.
        void ApplyTo(CNumber& oNumber) const;
        // Apply this transformation to a given segment.
        void ApplyTo(SSegment& oSegment) const;
        // Solve clamp(x Operator Operand, Range) ∩ Target ≠ Ø for x.
        ETriStateCompletion Reserve(SSegment& oTarget, const SSegment& Range, CNumber Tolerance = CNumber::DefaultTolerance) const;

    private:
        EOperator Operator = EOperator::nil;
        CNumber Operand{};
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CEffect // Collection of transformations
    {
        friend class CAction;
    public:
        CEffect(const CFactDefinition& Definition);

        std::string ToString() const; // For debugging

        bool IsEmpty() const;
        const CFactDefinition& GetDefinition() const { return mDefinition; }

        int GetTransformCapacity() const { return static_cast<int>(mTransforms.size()); } // Total number of all transforms, including unset ones
        int CountTransforms() const; // Number of transforms that are set
        const CTransform& GetTransform(const CFact& Fact) const;
        const CTransform& GetTransform(int FactIndex) const;
        bool SetTransform(const CFact& Fact, const CTransform& Transform);
        bool SetTransform(const CFact& Fact, CNumber Value);
        bool SetTransform(const SFactOperation& Operation);

        // Apply this effect to a given state.
        void ApplyTo(CState& State) const;

    private:
        void Expand(int Size);

    private:
        const CFactDefinition& mDefinition;
        std::vector<CTransform> mTransforms;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
    