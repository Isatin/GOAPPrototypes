// Copyright 2024 Isaac Hsu

#include <cassert>

#include "Comparer.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const EComparer EComparer::Equal            = equal;
const EComparer EComparer::LessOrEqual      = lessOrEqual;
const EComparer EComparer::GreaterOrEqual   = greaterOrEqual;

const char* EComparer::GetName() const
{
    switch (mValue)
    {
    case equal:             return "Equal";
    case lessOrEqual:       return "LessOrEqual";
    case greaterOrEqual:    return "GreaterOrEqual";
    }

    return "UNDEF";
}

const char* EComparer::GetSymbol() const
{
    switch (mValue)
    {
    case equal:             return "==";
    case lessOrEqual:       return "<=";
    case greaterOrEqual:    return ">=";
    }

    assert(!"Invalid comparer type");
    return "";
}
///////////////////////////////////////////////////////////////////////////////////////////////////
