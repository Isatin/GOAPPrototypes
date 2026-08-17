// Copyright 2025 Isaac Hsu

#include "ArgumentConverter.h"
#include "BooleanExpression.h"
#include "Fact.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CExpression&& SArgumentConverter<CBooleanExpression>::operator () (CBooleanExpression& Argument) const
{
    return std::move(Argument.ToBase());
}

const CExpression& SArgumentConverter<CBooleanExpression&>::operator () (const CBooleanExpression& Argument) const
{
    return Argument.ToBase();
}

COperand SArgumentConverter<CBooleanFact>::operator () (const CBooleanFact& Argument) const
{
    return {Argument};
}

COperand SArgumentConverter<CEnumerationFact>::operator () (const CEnumerationFact& Argument) const
{
    return {Argument};
}

COperand SArgumentConverter<CNumericFact>::operator () (const CNumericFact& Argument) const
{
    return {Argument};
}
///////////////////////////////////////////////////////////////////////////////////////////////////