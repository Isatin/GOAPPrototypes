// Copyright 2024 Isaac Hsu

#include <sstream>

#include "Operator.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const EOperator EOperator::Assign(assign);
const EOperator EOperator::Negation(negation);
const EOperator EOperator::Addition(addition);
const EOperator EOperator::Multiplication(multiplication);
///////////////////////////////////////////////////////////////////////////////////////////////////
EOperator& EOperator::operator = (int Value)
{
    mValue = static_cast<Type>(Value);
    return *this;
}

int EOperator::GetArity() const
{
    switch (mValue)
    {
    case assign:            return 2;
    case negation:          return 1;
    case addition:          return 2;
    case multiplication:    return 2;
    }

    return 0;
}

const char* EOperator::GetName() const
{
    switch (mValue)
    {
    case nil:               return "Nil";
    case assign:            return "Assign";
    case negation:          return "Negation";
    case addition:          return "Addition";
    case multiplication:    return "Multiplication";
    }

    return "";
}

const char* EOperator::GetSymbol() const
{
    switch (mValue)
    {
    case assign:            return "=";
    case negation:          return "!";
    case addition:          return "+=";
    case multiplication:    return "*=";
    }

    return "";
}

std::string EOperator::ToString(const std::string& Subject) const
{
    std::stringstream Stream;

    if (GetArity() == 1)
    {
        Stream << GetSymbol() << Subject;
    }
    else
    {
        Stream << Subject << GetSymbol();
    }

    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
