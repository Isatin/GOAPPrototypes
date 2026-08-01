// Copyright 2024 Isaac Hsu

#pragma once

#include "Comparer.h"
#include "Operator.h"

#include <string>


namespace ArithGOAP
{
    class CBooleanFact;
    class CEnumerationFact;
    class CFact;
    class CNumericFact;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SFactOperation // Used to store an operation on a fact and a number
    {
        const CFact&    Subject;
        EOperator       Operator;
        CNumber         Operand;

    public:
        SFactOperation(const CFact& Sbj, EOperator Op, CNumber Value) : Subject(Sbj), Operator(Op), Operand(Value) {}
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    SFactOperation operator ! (const CBooleanFact& Fact);
    SFactOperation operator - (const CNumericFact& Fact);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SFactEquation // Used to store an equation with a fact on one side and a number on the other
    {
        const CFact&    Subject;
        CNumber         Value;

    public:
        SFactEquation(const CFact& Sbj, CNumber Value) : Subject(Sbj), Value(Value) {}
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    SFactEquation operator == (const CBooleanFact& Fact, bool Value);
    SFactEquation operator == (const CEnumerationFact& Fact, int Value);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T> // Used to store a variable clamped between two numbers
    struct SRange
    {
        T       Subject;
        CNumber Minimum;
        CNumber Maximum;

    public:
        SRange(const T& Sbj, CNumber Min, CNumber Max) : Subject(Sbj), Minimum(Min), Maximum(Max) {}

        SRange operator <= (CNumber Value) const { return {Subject, Minimum, std::min(Maximum, Value)}; }
        SRange operator >= (CNumber Value) const { return {Subject, std::max(Minimum, Value), Maximum}; }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SNumericFactRange : public SRange<const CNumericFact&>
    {
        SNumericFactRange(const SRange& Source) : SRange(Source) {}
        using SRange::SRange;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline SNumericFactRange operator == (const CNumericFact& Fact, CNumber Value) { return {Fact, Value, Value}; }
    inline SNumericFactRange operator <= (const CNumericFact& Fact, CNumber Value) { return {Fact, -CNumber::Infinity, Value }; }
    inline SNumericFactRange operator >= (const CNumericFact& Fact, CNumber Value) { return {Fact, Value, CNumber::Infinity}; }
    inline SNumericFactRange operator <= (CNumber Value, const CNumericFact& Fact) { return {Fact, Value, CNumber::Infinity}; }
    inline SNumericFactRange operator >= (CNumber Value, const CNumericFact& Fact) { return {Fact, -CNumber::Infinity, Value}; }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SVariable // Named variable
    {
        std::string Name;

    public:
        explicit SVariable(const std::string& iName) : Name(iName) {}
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SVariableRange : public SRange<const SVariable>
    {
        SVariableRange(const SRange& Source) : SRange(Source) {}
        using SRange::SRange;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline SVariableRange operator == (const SVariable& Var, CNumber Value) { return {Var, Value, Value}; }
    inline SVariableRange operator <= (const SVariable& Var, CNumber Value) { return {Var, -CNumber::Infinity, Value}; }
    inline SVariableRange operator >= (const SVariable& Var, CNumber Value) { return {Var, Value, CNumber::Infinity}; }
    inline SVariableRange operator <= (CNumber Value, const SVariable& Var) { return {Var, Value, CNumber::Infinity}; }
    inline SVariableRange operator >= (CNumber Value, const SVariable& Var) { return {Var, -CNumber::Infinity, Value}; }
}
