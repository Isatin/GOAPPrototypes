// Copyright 2024 Isaac Hsu

#include <cassert>

#include "Effect.h"
#include "Fact.h"
#include "Notation.h"
#include "Segment.h"
#include "State.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const CTransform CTransform::Nil;

std::string CTransform::ToString() const
{
    std::string Return = Operator.GetSymbol();
    Return += Operand.ToString();
    return Return;
}

std::string CTransform::Stringize(const std::string& Subject) const
{
    std::string Return = Operator.Stringize(Subject);
    if (Operator.GetArity() >= 2)
    {
        Return += Operand.ToString(); // Print the right operand if there is more than one operand.
    }
    return Return;
}

void CTransform::ApplyTo(CNumber& oNumber) const
{
    switch (Operator)
    {
    case EOperator::assignment:     oNumber = Operand; break;
    case EOperator::negation:       oNumber = !oNumber; break;
    case EOperator::addition:       oNumber += Operand; break;
    case EOperator::multiplication: oNumber *= Operand; break;
    }
}

void CTransform::ApplyTo(SSegment& oSegment) const
{
    if (oSegment.Maximum == oSegment.Minimum)
    {
        ApplyTo(oSegment.Minimum);
        oSegment.Maximum = oSegment.Minimum;
    }
    else
    {
        ApplyTo(oSegment.Minimum);
        ApplyTo(oSegment.Maximum);
    }
}

ETriStateCompletion CTransform::Reserve(SSegment& oTarget, const SSegment& Range, CNumber Tolerance) const
{
    if (!Operand.IsFinite())
    {
        return ETriStateCompletion::failed;
    }

    switch (Operator)
    {
    case EOperator::assignment:
        return oTarget.Contain(Operand, Tolerance) && Range.Contain(Operand, Tolerance) ? ETriStateCompletion::complete : ETriStateCompletion::failed;

    case EOperator::negation:
        if (oTarget.IsDegenerate(Tolerance))
        {            
            oTarget = !oTarget.Minimum; // ¬x==B ⇒ x==¬B
            return oTarget.Intersect(Range, Tolerance) ? ETriStateCompletion::partial : ETriStateCompletion::failed;
        }
        else
        {
            return ETriStateCompletion::failed;
        }

    case EOperator::addition:
    case EOperator::multiplication:
        if (!Range.Unclamp(oTarget, Tolerance))
        {
            return ETriStateCompletion::failed;
        }

        if (Operator == EOperator::addition)
        {
            oTarget -= Operand;
        }
        else if (Operator == EOperator::multiplication)
        {
            if (Operand.IsEquivalent(0, Tolerance))
            {          
                if (oTarget.Contain(0, Tolerance))
                {
                    oTarget = SSegment::Boundless; // Any number multiplied by zero equals zero.
                    return ETriStateCompletion::partial;
                }
                else
                {
                    return ETriStateCompletion::failed;
                }
            }
            else
            {
                oTarget /= Operand;
            }
        }

        return oTarget.Intersect(Range, Tolerance) ? ETriStateCompletion::partial : ETriStateCompletion::failed;
    }

    return ETriStateCompletion::failed;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CEffect::CEffect(const CFactDefinition& Definition)
    : mDefinition(Definition) 
{
    Expand(Definition.GetFactCount());
}

void CEffect::Expand(int Size)
{
    if (Size > mTransforms.size())
    {
        mTransforms.resize(Size);
    }
}

std::string CEffect::ToString() const
{
    std::string Return;
    bool Successive = false;

    for (int FactIndex = 0; FactIndex < mTransforms.size(); FactIndex++)
    {       
        const CTransform& Transform = mTransforms[FactIndex];
        if (!Transform.IsNil())
        {
            if (Successive)
            {
                Return += ", ";
            }
            else
            {
                Successive = true;
            }

            const std::string& FactName = GetDefinition().GetFact(FactIndex)->GetName();
            Return += mTransforms[FactIndex].Stringize(FactName);
        }
    }

    return Return;
}

bool CEffect::IsEmpty() const
{
    for (const CTransform& Transform : mTransforms)
    {
        if (Transform.GetOperator() != EOperator::nil)
        {
            return false;
        }
    }

    return true;
}

int CEffect::CountTransforms() const
{
    int Count = 0;

    for (const CTransform& Transform : mTransforms)
    {
        if (Transform.GetOperator() != EOperator::nil)
        {
            Count++;
        }
    }

    return Count;
}

const CTransform& CEffect::GetTransform(const CFact& Fact) const
{
    assert(&Fact.GetOwner() == &mDefinition);

    return GetTransform(Fact.GetIndex());
}

const CTransform& CEffect::GetTransform(int FactIndex) const
{
    if (FactIndex >= 0 && FactIndex < mTransforms.size())
    {
        return mTransforms[FactIndex];
    }
    else
    {
        return CTransform::Nil;
    }
}

bool CEffect::SetTransform(const CFact& Fact, const CTransform& Transform)
{
    assert(&Fact.GetOwner() == &mDefinition);

    if (!Transform.GetOperand().IsFinite())
    {
        return false;
    }

    switch (Transform.GetOperator())
    {
    case EOperator::nil:
        return false;

    case EOperator::assignment:
        if (!Fact.GetRange().Contain(Transform.GetOperand(), mDefinition.GetTolerance()))
        {
            return false;
        }
        break;
    }

    Expand(Fact.GetIndex() + 1);
    mTransforms[Fact.GetIndex()] = Transform;
    return true;
}

bool CEffect::SetTransform(const CFact& Fact, CNumber Value)
{
    return SetTransform(Fact, CTransform(EOperator::assignment, Value));
}

bool CEffect::SetTransform(const SFactOperation& Operation)
{
    return SetTransform(Operation.Subject, CTransform(Operation.Operator, Operation.Operand));
}

void CEffect::ApplyTo(CState& State) const
{
    assert(&State.GetDefinition() == &mDefinition);

    for (int FactIndex = 0; FactIndex < mTransforms.size(); FactIndex++)
    {
        const CTransform& Transform = mTransforms[FactIndex];
        SSegment& Property = State.GetProperty(FactIndex);
        if (Property.IsSet())
        {
            Transform.ApplyTo(Property);

            if (const CFact* Fact = mDefinition.GetFact(FactIndex))
            {
                Fact->GetRange().Clamp(Property);
            }
        }
        else if (Transform.GetOperator() == EOperator::assignment)
        {
            State.SetProperty(FactIndex, Transform.GetOperand());
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////