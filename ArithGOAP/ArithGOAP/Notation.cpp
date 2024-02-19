// Copyright 2024 Isaac Hsu

#include "Notation.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
SVariableRange SVariable::operator == (SNumber Value) const
{
    return {*this, Value, Value};
}

SVariableRange SVariable::operator <= (SNumber Value) const
{
    return {*this, -SNumber::Infinity, Value};
}

SVariableRange ArithGOAP::operator <= (SNumber Value, const SVariable& Var)
{
    return {Var, Value, SNumber::Infinity};
}

SVariableRange SVariable::operator >= (SNumber Value) const
{
    return {*this, Value, SNumber::Infinity};
}

SVariableRange ArithGOAP::operator >= (SNumber Value, const SVariable& Var)
{
    return {Var, -SNumber::Infinity, Value};
}
///////////////////////////////////////////////////////////////////////////////////////////////////
