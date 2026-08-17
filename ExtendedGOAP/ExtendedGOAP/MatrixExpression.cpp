// Copyright 2026 Isaac Hsu

#include <cassert>

#include "Fact.h"
#include "MatrixExpression.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CMatrixExpression::CMatrixExpression(const CMatrixFact& Argument)
    : CExpression(CFactInfo(Argument))
{}

CMatrixExpression CMatrixExpression::GetRearranged() const
{
    CMatrixExpression Return = *this;
    Return.Rearrange();
    return Return;
}

void CMatrixExpression::ReplaceFact(const CMatrixFact& Fact, const CMatrixExpression& Replacement)
{
    Super::ReplaceFact(Fact, Replacement.ToBase());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
