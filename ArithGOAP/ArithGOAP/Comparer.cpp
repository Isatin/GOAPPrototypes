// Copyright 2024 Isaac Hsu

#include "Comparer.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const EComparer EComparer::Equal(equal);
const EComparer EComparer::GreaterOrEqual(greaterOrEqual);
const EComparer EComparer::LessOrEqual(lessOrEqual);

EComparer& EComparer::operator = (int Value)
{
    mValue = static_cast<Type>(Value);
    return *this;
}

const char* EComparer::GetName() const
{
    switch (mValue)
    {
    case nil:               return "Nil";
    case equal:             return "Equal";
    case greaterOrEqual:    return "GreaterOrEqual";
    case lessOrEqual:       return "LessOrEqual";
    }

    return "";
}

const char* EComparer::GetSymbol() const
{
    switch (mValue)
    {
    case equal:             return "==";
    case greaterOrEqual:    return ">=";
    case lessOrEqual:       return "<=";
    }

    return "";
}
///////////////////////////////////////////////////////////////////////////////////////////////////
