// Copyright 2026 Isaac Hsu

#include <cassert>

#include "Matrix.h"
#include "Notation.h"
#include "Operator.h"
#include "Set.h"
#include "TriState.h"
#include "Utility.h"
#include "Variant.h"


using namespace ExtendedGOAP;
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
        else if (Operator.IsPostfix() && Operator.GetArity() != 1)
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
    case scalarMultiplication:  return Left * Right; // For a matrix multiplied by more than one scalar
    }

    assert(!"Invalid operator type");
    return CNumber::Null;
}

CVariant EOperator::operator () (CVariant&& Left, CVariant&& Right) const
{
    switch (mValue)
    {
    case nil:               return std::move(Right); // Nil operator means evaluating only the right operand.
    case addition:          return CVariant(Left.GrabScalar() + Right.GrabScalar());
    case subtraction:       return CVariant(Left.GrabScalar() - Right.GrabScalar());
    case multiplication:    return CVariant(Left.GrabScalar() * Right.GrabScalar());
    case division:          return CVariant(Left.GrabScalar() / Right.GrabScalar());
    case modulo:            return CVariant(std::fmod(Left.GrabScalar(), Right.GrabScalar()));
    case minus:             return CVariant(-Right.GrabScalar());
    case absoluteValue:     return CVariant(std::abs(Right.GrabScalar()));
    case minimum:           return CVariant(Right.GrabScalar().IsNaN() ? Left.GrabScalar() : (Left.GrabScalar() < Right.GrabScalar() ? Left.GrabScalar() : Right.GrabScalar()));
    case maximum:           return CVariant(Right.GrabScalar().IsNaN() ? Left.GrabScalar() : (Left.GrabScalar() > Right.GrabScalar() ? Left.GrabScalar() : Right.GrabScalar()));
    case booleanNOT:        return CVariant(!Right.GrabScalar());
    case booleanAND:        return CVariant(Left.GrabScalar() && Right.GrabScalar());
    case booleanOR:         return CVariant(Left.GrabScalar() || Right.GrabScalar());
    case matrixAddition:        return Left.GrabMatrix() + Right.GrabMatrix();
    case matrixSubtraction:     return Left.GrabMatrix() - Right.GrabMatrix();
    case matrixMultiplication:  return Left.GrabMatrix() * Right.GrabMatrix();
    case scalarMultiplication:  return Left.IsScalar() ? Right.GrabMatrix() * Left.GrabScalar() : Left.GrabMatrix() * Right.GrabScalar();
    case setComplement:             return ~Right.GrabSet();
    case setIntersection:           return Left.GrabSet().GetIntersection(Right.GrabSet());
    case setUnion:                  return Left.GrabSet().GetUnion(Right.GrabSet());
    case setDifference:             return Left.GrabSet().GetDifference(Right.GrabSet());
    case setSymmetricDifference:    return Left.GrabSet().GetSymmetricDifference(Right.GrabSet());
    }

    assert(!"Invalid operator type");
    return CVariant::Unset;
}

bool EOperator::IsPostfix() const
{
    switch (mValue)
    {
    case setComplement: return true;
    }

    return false;
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

ETriState EOperator::IsScalar() const
{
    switch (mValue)
    {
    case nil:               return ETriState::unknown; // Return unknown because it depends on whether the right operand is a scalar, not the operator.
    case addition:          return ETriState::yes;
    case subtraction:       return ETriState::yes;
    case multiplication:    return ETriState::yes;
    case division:          return ETriState::yes;
    case modulo:            return ETriState::yes;
    case minus:             return ETriState::yes;
    case absoluteValue:     return ETriState::yes;
    case minimum:           return ETriState::yes;
    case maximum:           return ETriState::yes;
    case booleanNOT:        return ETriState::yes;
    case booleanAND:        return ETriState::yes;
    case booleanOR:         return ETriState::yes;
    }

    return ETriState::no;
}

bool EOperator::IsMatrix() const
{
    switch (mValue)
    {
    case matrixAddition:        return true;
    case matrixSubtraction:     return true;
    case matrixMultiplication:  return true;
    case scalarMultiplication:  return true;
    }

    return false;
}

bool EOperator::ReturnMatrix() const
{
    switch (mValue)
    {
    case matrixAddition:        return true;
    case matrixSubtraction:     return true;
    case matrixMultiplication:  return true;
    case scalarMultiplication:  return true;
    }

    return false;
}

bool EOperator::IsSet() const
{
    switch (mValue)
    {
    case setComplement:             return true;
    case setIntersection:           return true;
    case setUnion:                  return true;
    case setDifference:             return true;
    case setSymmetricDifference:    return true;
    }

    return false;
}

bool EOperator::ReturnSet() const
{
    switch (mValue)
    {
    case setComplement:             return true;
    case setIntersection:           return true;
    case setUnion:                  return true;
    case setDifference:             return true;
    case setSymmetricDifference:    return true;
    }

    return false;
}

EFactType EOperator::ReturnType() const
{
    switch (mValue)
    {
    case nil:               assert(!"Unsupported operator type for this function"); return EFactType::none;
    case addition:          return EFactType::number;
    case subtraction:       return EFactType::number;
    case multiplication:    return EFactType::number;
    case division:          return EFactType::number;
    case modulo:            return EFactType::number;
    case minus:             return EFactType::number;
    case absoluteValue:     return EFactType::number;
    case minimum:           return EFactType::number;
    case maximum:           return EFactType::number;
    case booleanNOT:        return EFactType::boolean;
    case booleanAND:        return EFactType::boolean;
    case booleanOR:         return EFactType::boolean;
    case matrixAddition:        return EFactType::matrix;
    case matrixSubtraction:     return EFactType::matrix;
    case matrixMultiplication:  return EFactType::matrix;
    case scalarMultiplication:  return EFactType::matrix;
    case setComplement:             return EFactType::set;
    case setIntersection:           return EFactType::set;
    case setUnion:                  return EFactType::set;
    case setDifference:             return EFactType::set;
    case setSymmetricDifference:    return EFactType::set;
    }

    assert(!"Invalid operator type");
    return EFactType::none;
}

bool EOperator::IsCommutative() const
{
    switch (mValue)
    {
    case addition:              return true;
    case multiplication:        return true;
    case minimum:               return true;
    case maximum:               return true;
    case booleanAND:            return true;
    case booleanOR:             return true;
    case matrixAddition:        return true;
    case scalarMultiplication:  return true;
    case setIntersection:           return true;
    case setUnion:                  return true;
    case setSymmetricDifference:    return true;
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
    case matrixAddition:        return 2;
    case matrixSubtraction:     return 2;
    case matrixMultiplication:  return 2;
    case scalarMultiplication:  return 2;
    case setComplement:             return 1;
    case setIntersection:           return 2;
    case setUnion:                  return 2;
    case setDifference:             return 2;
    case setSymmetricDifference:    return 2;
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
    case matrixAddition:        return "MatrixAddition";
    case matrixSubtraction:     return "MatrixSubtraction";
    case matrixMultiplication:  return "MatrixMultiplication";
    case scalarMultiplication:  return "ScalarMultiplication";
    case setComplement:             return "SetComplement";
    case setIntersection:           return "SetIntersection";
    case setUnion:                  return "SetUnion";
    case setDifference:             return "SetDifference";
    case setSymmetricDifference:    return "SetSymmetricDifference";
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
    case matrixAddition:        return "madd";
    case matrixSubtraction:     return "msub";
    case matrixMultiplication:  return "mmul";
    case scalarMultiplication:  return "smul";
    case setComplement:             return "scmp";
    case setIntersection:           return "sint";
    case setUnion:                  return "suni";
    case setDifference:             return "sdif";
    case setSymmetricDifference:    return "ssdf";
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
    case matrixAddition:        return "+";
    case matrixSubtraction:     return "-";
#ifdef USE_UNICODE_SYMBOLS
    case matrixMultiplication:  return "×";
    case scalarMultiplication:  return "·";
    case setIntersection:           return "∩";
    case setUnion:                  return "∪";
    case setDifference:             return "∖";
    case setSymmetricDifference:    return "△";
#else
    case matrixMultiplication:  return "*";
    case scalarMultiplication:  return "*";
    case setIntersection:           return "&";
    case setUnion:                  return "|";
    case setDifference:             return "\\";
    case setSymmetricDifference:    return "^"; 
#endif
    case setComplement:             return CSet::GetComplementSymbol();
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
                if (IsPostfix())
                {
                    (Return = Right) += Symbol;
                }
                else
                {
                    (Return = Symbol) += Right;
                }
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
