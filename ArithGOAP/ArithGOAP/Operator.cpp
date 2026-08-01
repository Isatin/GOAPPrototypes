// Copyright 2024 Isaac Hsu

#include <cassert>

#include "Operator.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int EOperator::GetArity() const
{
    switch (mValue)
    {
    case assignment:        return 2;
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
    case assignment:        return "Assignment";
    case negation:          return "Negation";
    case addition:          return "Addition";
    case multiplication:    return "Multiplication";
    }

    return "UNDEF";
}

const char* EOperator::GetSymbol() const
{
    switch (mValue)
    {
    case nil:               return "";
    case assignment:        return "=";
    case negation:          return "!";
    case addition:          return "+=";
    case multiplication:    return "*=";
    }

    assert(!"Invalid operator type");
    return "";
}

std::string EOperator::Stringize(const std::string& Subject) const
{
    std::string Return;

    if (GetArity() == 1)
    {
        Return = GetSymbol();
        Return += Subject;
    }
    else
    {
        Return = Subject;
        Return += GetSymbol();
    }

    return Return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
