// Copyright 2025 Isaac Hsu

#pragma once

#include <type_traits>


namespace AlgebGOAP
{
    class CBooleanExpression;
    class CBooleanFact;
    class CEnumerationFact;
    class CExpression;
    class CNumber;
    class CNumericFact;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    concept RBooleanArgument =
        std::is_same_v<std::remove_cvref_t<T>, bool> ||
        std::is_same_v<std::remove_cvref_t<T>, CBooleanFact> ||
        std::is_same_v<std::remove_cvref_t<T>, CBooleanExpression>;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TLeft, typename TRight = void>
    concept RBooleanOperation =
        std::is_same_v<std::remove_cvref_t<TLeft>, CBooleanFact> || std::is_same_v<std::remove_cvref_t<TLeft>, CBooleanExpression> ||
        std::is_same_v<std::remove_cvref_t<TRight>, CBooleanFact> || std::is_same_v<std::remove_cvref_t<TRight>, CBooleanExpression>;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    concept REnumerationArgument =
        std::is_same_v<std::remove_cvref_t<T>, int> ||
        std::is_enum_v<std::remove_cvref_t<T>> ||
        std::is_same_v<std::remove_cvref_t<T>, CEnumerationFact>;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Do not allow both sides to be ints/enums. At least one side MUST be a fact.
    template <typename TLeft, typename TRight = void>
    concept REnumerationOperation = 
        std::is_same_v<std::remove_cvref_t<TLeft>, CEnumerationFact> || 
        std::is_same_v<std::remove_cvref_t<TRight>, CEnumerationFact>;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    concept RNumericArgument =
        std::is_arithmetic_v<T> ||
        std::is_same_v<std::remove_cvref_t<T>, CNumber> ||
        std::is_same_v<std::remove_cvref_t<T>, CNumericFact> ||
        std::is_same_v<std::remove_cvref_t<T>, CExpression>;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Do not allow both sides to be numbers. At least one side MUST contain a fact.
    template <typename TLeft, typename TRight = void>
    concept RNumericOperation =
        std::is_same_v<std::remove_cvref_t<TLeft>, CNumericFact> || std::is_same_v<std::remove_cvref_t<TLeft>, CExpression> ||
        std::is_same_v<std::remove_cvref_t<TRight>, CNumericFact> || std::is_same_v<std::remove_cvref_t<TRight>, CExpression>;
    ///////////////////////////////////////////////////////////////////////////////////////////////
}