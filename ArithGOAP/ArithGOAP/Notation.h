// Copyright 2024 Isaac Hsu

#pragma once

#include "Comparer.h"
#include "Number.h"
#include "Operator.h"

#include <string>


namespace ArithGOAP
{
    class CBooleanFact;
    class CFact;
    class CNumericFact;
    struct SVariableRange;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T> // A variable clamped between two numbers
    struct SRange
    {
        T       Subject;
        SNumber Minimum;
        SNumber Maximum;

        SRange(const T& Sbj, SNumber Min, SNumber Max) : Subject(Sbj), Minimum(Min), Maximum(Max) {}

        SRange operator <= (SNumber Value) const { return {Subject, Minimum, std::min(Maximum, Value)}; }
        SRange operator >= (SNumber Value) const { return {Subject, std::max(Minimum, Value), Maximum}; }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T> // An operation on a variable with another operand
    struct SOperation
    {
        T           Subject;
        EOperator   Operator;
        SNumber     Operand;

        SOperation(const T& Sbj, EOperator Op, SNumber Value) : Subject(Sbj), Operator(Op), Operand(Value) {}
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SVariable // A named variable
    {
        std::string Name;

        explicit SVariable(const std::string& iName) : Name(iName) {}

        SVariableRange operator == (SNumber Value) const;
        SVariableRange operator <= (SNumber Value) const;
        SVariableRange operator >= (SNumber Value) const;
        friend SVariableRange operator <= (SNumber Value, const SVariable& Var);
        friend SVariableRange operator >= (SNumber Value, const SVariable& Var);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SVariableRange : public SRange<const SVariable>
    {
        SVariableRange(const SRange& Source) : SRange(Source) {}
        using SRange::SRange;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SFactRange : public SRange<const CFact&>
    {
        SFactRange(const SRange& Source) : SRange(Source) {}
        using SRange::SRange;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SFactOperation : public SOperation<const CFact&>
    {
        SFactOperation(const SOperation& Source) : SOperation(Source) {}
        using SOperation::SOperation;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SBooleanFactOperation : public SOperation<const CBooleanFact&>
    {
        SBooleanFactOperation(const SOperation& Source) : SOperation(Source) {}
        using SOperation::SOperation;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SNumericFactRange : public SRange<const CNumericFact&>
    {
        SNumericFactRange(const SRange& Source) : SRange(Source) {}
        using SRange::SRange;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SNumericFactOperation : public SOperation<const CNumericFact&>
    {
        SNumericFactOperation(const SOperation& Source) : SOperation(Source) {}
        using SOperation::SOperation;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
