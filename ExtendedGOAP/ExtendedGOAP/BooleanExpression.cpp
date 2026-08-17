// Copyright 2026 Isaac Hsu

#include <cassert>

#include "BooleanExpression.h"
#include "Fact.h"
#include "TriState.h"
#include "Variant.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CBooleanExpression::CBooleanExpression(bool Argument)
    : CExpression(Argument)
{}

CBooleanExpression::CBooleanExpression(const CBooleanFact& Argument)
    : CExpression(CFactInfo(Argument))
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
    CVariant Variant = Super::Evaluate(State);
    if (Variant.IsUnset())
    {
        return ETriState::unknown;
    }
    else
    {
        assert(Variant.IsScalar());

        CNumber Value = Variant.GrabScalar();
        assert(Value.IsFinite());
        return ETriState(Value);
    }
}

void CBooleanExpression::ReplaceFact(const CBooleanFact& Fact, const CBooleanExpression& Replacement)
{
    Super::ReplaceFact(Fact, Replacement.ToBase());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
