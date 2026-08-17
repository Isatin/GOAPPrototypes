// Copyright 2026 Isaac Hsu

#pragma once

#include "Expression.h"


namespace ExtendedGOAP
{
    class CBooleanFact;
    class CBooleanExpression;
    class CEnumerationFact;
    class CMatrixFact;
    class CMatrixExpression;
    class CSetFact;
    class CSetExpression;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Used to convert operands to CExpression-compatible types when assembling a CExpression
    // Specifically, convert bools/integers/floats/enums/facts to COperand and different types of expressions to CExpression.
    // NOTE: It is designed to be used with universal references.
    template <typename T>
    struct SArgumentConverter;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T> requires std::is_arithmetic_v<T>
    struct SArgumentConverter<T>
    {
        COperand operator () (T Argument) const { return {Argument}; }
    };

    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CNumber>
    struct SArgumentConverter<T>
    {
        COperand operator () (CNumber Argument) const { return {Argument}; }
    };

    // An explicit specialization without a function implementation in the header prevents exposure of the template parameter definition.
    template <>
    struct SArgumentConverter<CNumericFact>
    {
        COperand operator () (const CNumericFact& Argument) const;
    };

    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CNumericFact>
    struct SArgumentConverter<T> : public SArgumentConverter<CNumericFact> {};

    // An rvalue reference is returned because a universal reference deduces to a non-reference base type when passed an rvalue argument.
    template <>
    struct SArgumentConverter<CExpression>
    {
        CExpression&& operator () (CExpression& Argument) const { return std::move(Argument); }
    };

    // An lvalue reference is returned because a universal reference deduces to a reference type when passed an lvalue argument.
    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CExpression>
    struct SArgumentConverter<T>
    {
        const CExpression& operator () (const CExpression& Argument) const { return Argument; }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // An explicit specialization without a function implementation in the header prevents exposure of the template parameter definition.
    template <>
    struct SArgumentConverter<CBooleanFact>
    {
        COperand operator () (const CBooleanFact& Argument) const;
    };

    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CBooleanFact>
    struct SArgumentConverter<T> : public SArgumentConverter<CBooleanFact> {};

    // An rvalue reference is returned because a universal reference deduces to a non-reference base type when passed an rvalue argument.
    template <>
    struct SArgumentConverter<CBooleanExpression>
    {
        CExpression&& operator () (CBooleanExpression& Argument) const;
    };

    // An lvalue reference is returned because a universal reference deduces to a reference type when passed an lvalue argument.
    template <>
    struct SArgumentConverter<CBooleanExpression&>
    {
        const CExpression& operator () (const CBooleanExpression& Argument) const;
    };

    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CBooleanExpression>
    struct SArgumentConverter<T> : public SArgumentConverter<CBooleanExpression&> {};
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T> requires std::is_enum_v<std::remove_cvref_t<T>>
    struct SArgumentConverter<T>
    {
        COperand operator () (T Argument) const { return {static_cast<int>(Argument)}; }
    };

    // An explicit specialization without a function implementation in the header prevents exposure of the template parameter definition.
    template <>
    struct SArgumentConverter<CEnumerationFact>
    {
        COperand operator () (const CEnumerationFact& Argument) const;
    };

    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CEnumerationFact>
    struct SArgumentConverter<T> : public SArgumentConverter<CEnumerationFact> {};
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // An explicit specialization without a function implementation in the header prevents exposure of the template parameter definition.
    template <>
    struct SArgumentConverter<CMatrixFact>
    {
        COperand operator () (const CMatrixFact& Argument) const;
    };

    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CMatrixFact>
    struct SArgumentConverter<T> : public SArgumentConverter<CMatrixFact> {};

    // An rvalue reference is returned because a universal reference deduces to a non-reference base type when passed an rvalue argument.
    template <>
    struct SArgumentConverter<CMatrixExpression>
    {
        CExpression&& operator () (CMatrixExpression& Argument) const;
    };

    // An lvalue reference is returned because a universal reference deduces to a reference type when passed an lvalue argument.
    template <>
    struct SArgumentConverter<CMatrixExpression&>
    {
        const CExpression& operator () (const CMatrixExpression& Argument) const;
    };

    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CMatrixExpression>
    struct SArgumentConverter<T> : public SArgumentConverter<CMatrixExpression&> {};
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // An explicit specialization without a function implementation in the header prevents exposure of the template parameter definition.
    template <>
    struct SArgumentConverter<CSetFact>
    {
        COperand operator () (const CSetFact& Argument) const;
    };

    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CSetFact>
    struct SArgumentConverter<T> : public SArgumentConverter<CSetFact> {};

    // An rvalue reference is returned because a universal reference deduces to a non-reference base type when passed an rvalue argument.
    template <>
    struct SArgumentConverter<CSetExpression>
    {
        CExpression&& operator () (CSetExpression& Argument) const;
    };

    // An lvalue reference is returned because a universal reference deduces to a reference type when passed an lvalue argument.
    template <>
    struct SArgumentConverter<CSetExpression&>
    {
        const CExpression& operator () (const CSetExpression& Argument) const;
    };

    template <typename T> requires std::is_same_v<std::remove_cvref_t<T>, CSetExpression>
    struct SArgumentConverter<T> : public SArgumentConverter<CSetExpression&> {};
    ///////////////////////////////////////////////////////////////////////////////////////////////
}