// Copyright 2025 Isaac Hsu

#include <cassert>

#include "BooleanExpression.h"
#include "Fact.h"
#include "TriState.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CBooleanExpression::CBooleanExpression(bool Argument)
    : CExpression(Argument)
{}

CBooleanExpression::CBooleanExpression(const CBooleanFact& Argument)
    : CExpression(SFactIndex(Argument.GetIndex()))
{}

bool CBooleanExpression::IsOnlyOneBool() const
{
    if (!IsOnlyOneConstant())
    {
        return false;
    }

    CNumber Constant = GrabTheOnlyConstant();
    return Constant.IsEqual(false) || Constant.IsEqual(true);
}

bool CBooleanExpression::IsBool(bool Value) const
{
    return IsConstant(Value);
}

CBooleanExpression CBooleanExpression::GetRearranged() const
{
    CBooleanExpression Return = *this;
    Return.Rearrange();
    return Return;
}

ETriState CBooleanExpression::Evaluate(const CStateBase& State) const
{
    CNumber Result = Super::Evaluate(State);
    if (Result.IsNull())
    {
        return ETriState::unknown;
    }
    else if (Result.IsEqual(0))
    {
        return ETriState::no;
    }
    else
    {
        return ETriState::yes;
    }
}

void CBooleanExpression::ReplaceFact(const CBooleanFact& Fact, const CBooleanExpression& Replacement)
{
    Super::ReplaceFact(Fact, Replacement.ToBase());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
