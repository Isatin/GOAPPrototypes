// Copyright 2026 Isaac Hsu

#include <cassert>

#include "Fact.h"
#include "SetExpression.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CSetExpression::CSetExpression(const CSetFact& Argument)
    : CExpression(CFactInfo(Argument))
{}

CSetExpression CSetExpression::GetRearranged() const
{
    CSetExpression Return = *this;
    Return.Rearrange();
    return Return;
}

void CSetExpression::ReplaceFact(const CSetFact& Fact, const CSetExpression& Replacement)
{
    Super::ReplaceFact(Fact, Replacement.ToBase());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
