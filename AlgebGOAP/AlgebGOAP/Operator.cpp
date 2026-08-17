// Copyright 2025 Isaac Hsu

#include <cassert>

#include "Notation.h"
#include "Operator.h"
#include "Utility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
static const int gInvalidOperatorCount = EOperator::VerifyOperators(); // Dummy global variable to call the function

int EOperator::VerifyOperators()
{
    int InvalidCount = 0;

    const int OpCount = static_cast<int>(Type::count);
    for (int OpIndex = static_cast<int>(Type::nil); OpIndex < OpCount; OpIndex++)
    {
        EOperator Operator(static_cast<Type>(OpIndex));
        if (Operator.IsCommutative() && Operator.GetArity() != 2)
        {
            InvalidCount++;
        }
    }

    assert(InvalidCount == 0); // This can be replaced with static_assert, but the invoked functions must be constexpr, and thus exposed in the header.
    return InvalidCount;
}

CNumber EOperator::operator () (CNumber Left, CNumber Right) const
{
    switch (mValue)
    {
    case nil:               return Right; // Nil operator means evaluating only the right operand.
    case addition:          return Left + Right;
    case subtraction:       return Left - Right;
    case multiplication:    return Left * Right;
    case division:          return Left / Right;
    case modulo:            return std::fmod(Left, Right);
    case minus:             return -Right;
    case absoluteValue:     return std::abs(Right);
    case minimum:           return Right.IsNaN() ? Left : (Left < Right ? Left : Right);
    case maximum:           return Right.IsNaN() ? Left : (Left > Right ? Left : Right);
    case booleanNOT:        return !Right;
    case booleanAND:        return Left && Right;
    case booleanOR:         return Left || Right;
    }

    assert(!"Invalid operator type");
    return CNumber::Null;
}

bool EOperator::IsBoolean() const
{
    switch (mValue)
    {
    case booleanNOT:    return true;
    case booleanAND:    return true;
    case booleanOR:     return true;
    }

    return false;
}

bool EOperator::IsCommutative() const
{
    switch (mValue)
    {
    case addition:          return true;
    case multiplication:    return true;
    case minimum:           return true;
    case maximum:           return true;
    case booleanAND:        return true;
    case booleanOR:         return true;
    }

    return false;
}

int EOperator::GetArity() const
{
    switch (mValue)
    {
    case nil:               return 1; // Nil operator means evaluating only the right operand.
    case addition:          return 2;
    case subtraction:       return 2;
    case multiplication:    return 2;
    case division:          return 2;
    case modulo:            return 2;
    case minus:             return 1;
    case absoluteValue:     return 1;
    case minimum:           return 2;
    case maximum:           return 2;
    case booleanNOT:        return 1;
    case booleanAND:        return 2;
    case booleanOR:         return 2;
    }

    return 0;
}

const char* EOperator::GetName() const
{
    switch (mValue)
    {
    case nil:               return "Nil";
    case addition:          return "Addition";
    case subtraction:       return "Subtraction";
    case multiplication:    return "Multiplication";
    case division:          return "Division";
    case modulo:            return "Modulo";
    case minus:             return "Minus";
    case absoluteValue:     return "AbsoluteValue";
    case minimum:           return "Minimum";
    case maximum:           return "Maximum";
    case booleanNOT:        return "BooleanNOT";
    case booleanAND:        return "BooleanAND";
    case booleanOR:         return "BooleanOR";
    }

    return "UNDEF";
}

const char* EOperator::GetFunctionName() const
{
    switch (mValue)
    {
    case nil:               return "";
    case addition:          return "add";
    case subtraction:       return "sub";
    case multiplication:    return "mul";
    case division:          return "div";
    case modulo:            return "mod";
    case minus:             return "mns";
    case absoluteValue:     return "abs";
    case minimum:           return "min";
    case maximum:           return "max";
    case booleanNOT:        return "not";
    case booleanAND:        return "and";
    case booleanOR:         return "or";
    }

    return "UNDEF";
}

const char* EOperator::GetSymbol() const
{
    switch (mValue)
    {
    case nil:               return nullptr;
    case addition:          return "+";
    case subtraction:       return "-";
#ifdef USE_UNICODE_SYMBOLS
    case multiplication:    return "×";
    case division:          return "÷";
#else
    case multiplication:    return "*";
    case division:          return "/";
#endif
    case modulo:            return "%";
    case minus:             return "-";
    case absoluteValue:     return nullptr;
    case minimum:           return nullptr;
    case maximum:           return nullptr;
    case booleanNOT:        return "!";
    case booleanAND:        return "&&";
    case booleanOR:         return "||";
    }

    assert(!"Invalid operator type");
    return nullptr;
}

std::string EOperator::Stringize(const std::string& Left, const std::string& Right) const
{
    if (IsNil())
    {
        return Right;
    }
    else
    {
        std::string Return;
        const int Arity = GetArity();
        const char* Symbol = GetSymbol();

        if (Arity == 1)
        {
            if (Symbol)
            {
                (Return = Symbol) += Right;
            }
            else if (IsParenthetic(Right))
            {
                (Return = GetFunctionName()) += Right;
            }
            else
            {
                (((Return = GetFunctionName()) += '(') += Right) += ')';
            }
        }
        else if (Arity == 2)
        {
            if (Symbol)
            {
                ((Return = Left) += Symbol) += Right;
            }
            else
            {
                (((((Return = GetFunctionName()) += '(') += Left) += ',') += Right) += ')';
            }
        }

        return Return;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
