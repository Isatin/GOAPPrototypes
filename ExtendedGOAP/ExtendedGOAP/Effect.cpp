// Copyright 2026 Isaac Hsu

#include <cassert>

#include "BitVector.h"
#include "Effect.h"
#include "Fact.h"
#include "Notation.h"
#include "State.h"
#include "Term.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
STransform::STransform(int FactIndex, const CExpression& Expression)
    : FactIndex(FactIndex)
    , Expression(Expression)
{
    assert(FactIndex >= 0);
}

STransform::STransform(int FactIndex, CExpression&& Expression)
    : FactIndex(FactIndex)
    , Expression(std::move(Expression))
{
    assert(FactIndex >= 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CEffect::CEffect(const CFactDefinition& Definition)
    : mDefinition(Definition)
{}

std::string CEffect::ToString() const
{
    std::string Return;
    bool Successive = false;

    for (const STransform& Transform : mTransforms)
    {
        if (Successive)
        {
            Return += ", ";
        }
        else
        {
            Successive = true;
        }

        Return += GetDefinition().GetFact(Transform.FactIndex)->GetName();
        Return += "=";
        Return += Transform.Expression.ToString(GetDefinition());
    }

    return Return;
}

bool CEffect::IsNeutral() const
{
    for (const STransform& Transform : mTransforms)
    {
        // An empty transform has no effect; an identity function makes no difference.
        if (!Transform.Expression.IsEmpty() && !Transform.IsIdentityFunction())
        {
            return false;
        }
    }

    return true;
}

bool CEffect::HasAnyTransform(const CBitVector& FactBits) const
{
    for (const STransform& Transform : mTransforms)
    {
        if (Transform.FactIndex < FactBits.GetSize() && FactBits[Transform.FactIndex])
        {
            return true;
        }
    }

    return false;
}

CBitVector CEffect::GetUsedFactBits() const
{
    CBitVector Return(mDefinition.GetFactCount());

    for (const STransform& Transform : mTransforms)
    {
        Return[Transform.FactIndex] = true;
        Return |= Transform.Expression.GetUsedFactBits();
    }

    return Return;
}

CBitVector CEffect::GetAffectedFactBits() const
{
    CBitVector Return(mDefinition.GetFactCount());

    for (const STransform& Transform : mTransforms)
    {
        Return[Transform.FactIndex] = true;
    }

    return Return;
}

CBitVector CEffect::GetAffectingFactBits(const CBitVector& FactBits) const
{
    CBitVector Return(mDefinition.GetFactCount());

    for (const STransform& Transform : mTransforms)
    {
        if (Transform.FactIndex < FactBits.GetSize() && FactBits[Transform.FactIndex])
        {
            Return |= Transform.Expression.GetUsedFactBits();
        }
    }

    return Return;
}

bool CEffect::ApplyTo(CState& oState) const
{
    assert(&oState.GetDefinition() == &mDefinition);

    for (const STransform& Transform : mTransforms)
    {
        CVariant Variant = Transform.Expression.Evaluate(oState);
        if (Variant.IsUnset())
        {
            return false; // Disallow invalid effects.
        }

        if (Variant.IsScalar())
        {
            if (Variant.GrabScalar().IsNaN())
            {
                return false; // Disallow effects that produce NaN.
            }
        }
        else
        {
            if (Variant.HasError())
            {
                return false; // Disallow invalid effects.
            }
        }

        oState.SetProperty(Transform.FactIndex, std::move(Variant));
    }

    return true;
}

const STransform* CEffect::GetFirstTransform(const CFact& Fact) const
{
    assert(&Fact.GetOwner() == &mDefinition);

    for (const STransform& Transform : mTransforms)
    {
        if (Transform.FactIndex == Fact.GetIndex())
        {
            return &Transform;
        }
    }

    return nullptr;
}

STransform& CEffect::GetTransform(int TransformIndex)
{
    assert(TransformIndex >= 0 && TransformIndex < mTransforms.size());

    return mTransforms[TransformIndex];
}

bool CEffect::AddTransform(CFactAssignment&& Assignment)
{
    assert(&Assignment.mSubject.GetOwner() == &mDefinition);

    return AddTransform(Assignment.mSubject.GetIndex(), std::move(Assignment.mExpression));
}

bool CEffect::AddTransform(const CNumericFact& Fact, const CExpression& Expression)
{
    assert(&Fact.GetOwner() == &mDefinition);

    return AddTransform(Fact.GetIndex(), Expression);
}

bool CEffect::AddTransform(const CNumericFact& Fact, CExpression&& Expression)
{
    assert(&Fact.GetOwner() == &mDefinition);

    return AddTransform(Fact.GetIndex(), std::move(Expression));
}

bool CEffect::AddTransform(int FactIndex, CExpression&& Expression)
{
    assert(FactIndex >= 0 && FactIndex < mDefinition.GetFactCount());

    if (Expression.IsEmpty())
    {
        return false;
    }

    mTransforms.emplace_back(FactIndex, std::move(Expression));
    return true;
}

void CEffect::Rearrange()
{
    for (STransform& Transform : mTransforms)
    {
        // Convert to terms, combine like terms, and then convert back to an expression.
        CTermList TermList(Transform.Expression);
        TermList.Rearrange(); 
        Transform.Expression = TermList.ToExpression();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////