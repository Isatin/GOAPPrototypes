// Copyright 2026 Isaac Hsu

#include "ArgumentConverter.h"
#include "BooleanExpression.h"
#include "Fact.h"
#include "MatrixExpression.h"
#include "SetExpression.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CExpression&& SArgumentConverter<CBooleanExpression>::operator () (CBooleanExpression& Argument) const
{
    return std::move(Argument.ToBase());
}

const CExpression& SArgumentConverter<CBooleanExpression&>::operator () (const CBooleanExpression& Argument) const
{
    return Argument.ToBase();
}

CExpression&& SArgumentConverter<CMatrixExpression>::operator () (CMatrixExpression& Argument) const
{
    return std::move(Argument.ToBase());
}

const CExpression& SArgumentConverter<CMatrixExpression&>::operator () (const CMatrixExpression& Argument) const
{
    return Argument.ToBase();
}

CExpression&& SArgumentConverter<CSetExpression>::operator () (CSetExpression& Argument) const
{
    return std::move(Argument.ToBase());
}

const CExpression& SArgumentConverter<CSetExpression&>::operator () (const CSetExpression& Argument) const
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

COperand SArgumentConverter<CMatrixFact>::operator () (const CMatrixFact& Argument) const
{
    return {Argument};
}

COperand SArgumentConverter<CSetFact>::operator () (const CSetFact& Argument) const
{
    return {Argument};
}
///////////////////////////////////////////////////////////////////////////////////////////////////