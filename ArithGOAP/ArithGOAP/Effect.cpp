// Copyright 2024 Isaac Hsu

#include <cassert>
#include <sstream>

#include "Effect.h"
#include "Fact.h"
#include "Interval.h"
#include "Notation.h"
#include "State.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const CTransform CTransform::Nil;

std::string CTransform::ToString() const
{
    std::stringstream Stream;
    Stream << Operator.GetSymbol() << Operand;
    return Stream.str();
}

std::string CTransform::ToString(const std::string& Subject) const
{
    std::stringstream Stream;

    Stream << Operator.ToString(Subject);
    if (Operator.GetArity() >= 2)
    {
        Stream << Operand;
    }

    return Stream.str();
}

void CTransform::ApplyTo(SNumber& Number) const
{
    switch (Operator)
    {
    case EOperator::assign:         Number = Operand; break;
    case EOperator::negation:       Number = !Number; break;
    case EOperator::addition:       Number += Operand; break;
    case EOperator::multiplication: Number *= Operand; break;
    }
}

void CTransform::ApplyTo(SInterval& Interval) const
{
    if (Interval.Maximum == Interval.Minimum)
    {
        ApplyTo(Interval.Minimum);
        Interval.Maximum = Interval.Minimum;
    }
    else
    {
        ApplyTo(Interval.Minimum);
        ApplyTo(Interval.Maximum);
    }
}

ETriStateCompletion CTransform::Neutralize(SInterval& Target, const SInterval& Range) const
{
    if (!Operand.IsFinite())
    {
        return ETriStateCompletion::failed;
    }

    switch (Operator)
    {
    case EOperator::assign:
        return Target.Contain(Operand) && Range.Contain(Operand) ? ETriStateCompletion::complete : ETriStateCompletion::failed;

    case EOperator::negation:
        if (Target.IsDegenerate())
        {
            Target = (Target.Minimum == 0);
            return Target.Intersect(Range) ? ETriStateCompletion::partial : ETriStateCompletion::failed;
        }
        else
        {
            return ETriStateCompletion::failed;
        }

    case EOperator::addition:
    case EOperator::multiplication:
        if (!Range.Unclamp(Target))
        {
            return ETriStateCompletion::failed;
        }

        if (Operator == EOperator::addition)
        {
            Target -= Operand;
        }
        else if (Operator == EOperator::multiplication)
        {
            if (Operand == 0)
            {          
                if (Target.Contain(0))
                {
                    Target = SInterval::Boundless; // Any number multiplied by zero is zero.
                    return ETriStateCompletion::partial;
                }
                else
                {
                    return ETriStateCompletion::failed;
                }
            }
            else
            {
                Target /= Operand;

                if (Operand < 0)
                {
                    Target.Flip();
                }
            }
        }

        return Target.Intersect(Range) ? ETriStateCompletion::partial : ETriStateCompletion::failed;
    }

    return ETriStateCompletion::failed;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CEffect::CEffect(const CStateDefinition& Def)
    : mDefinition(Def) 
{
    Expand(Def.GetFactCount());
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
    std::stringstream Stream;
    bool First = true;

    for (int i = 0; i < mTransforms.size(); i++)
    {        
        if (const CTransform& Transform = mTransforms[i])
        {
            if (First)
            {
                First = false;
            }
            else
            {
                Stream << ", ";
            }

            const std::string& Name = GetDefinition().GetFact(i)->GetName();
            Stream << mTransforms[i].ToString(Name);
        }
    }

    return Stream.str();
}

bool CEffect::IsNil() const
{
    for (int i = 0; i < mTransforms.size(); i++)
    {
        const CTransform& Transform = mTransforms[i];
        if (Transform.GetOperator() != EOperator::nil)
        {
            return false;
        }
    }

    return true;
}

const CTransform& CEffect::GetTransform(const CFact& Fact) const
{
    assert(&Fact.GetOwner() == &mDefinition);

    return GetTransform(Fact.GetIndex());
}

const CTransform& CEffect::GetTransform(int Index) const
{
    if (Index < mTransforms.size())
    {
        return mTransforms[Index];
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

    case EOperator::assign:
        if (!Fact.GetRange().Contain(Transform.GetOperand()))
        {
            return false;
        }
        break;
    }

    Expand(Fact.GetIndex() + 1);
    mTransforms[Fact.GetIndex()] = Transform;
    return true;
}

bool CEffect::SetTransform(const CFact& Fact, SNumber Value)
{
    return SetTransform(Fact, CTransform(EOperator::Assign, Value));
}

bool CEffect::SetTransform(const SFactOperation& Operation)
{
    return SetTransform(Operation.Subject, CTransform(Operation.Operator, Operation.Operand));
}

bool CEffect::SetTransform(const SBooleanFactOperation& Operation)
{
    return SetTransform(Operation.Subject, CTransform(Operation.Operator, Operation.Operand));
}

bool CEffect::SetTransform(const SNumericFactOperation& Operation)
{ 
    return SetTransform(Operation.Subject, CTransform(Operation.Operator, Operation.Operand));
}

void CEffect::ApplyTo(CState& State) const
{
    assert(&State.GetDefinition() == &mDefinition);

    for (int i = 0; i < mTransforms.size(); i++)
    {
        const CTransform& Transform = mTransforms[i];
        if (SInterval& Interval = State.GetFact(i))
        {
            Transform.ApplyTo(Interval);

            if (const CFact* Fact = mDefinition.GetFact(i))
            {
                Fact->GetRange().Clamp(Interval);
            }
        }
        else if (Transform.GetOperator() == EOperator::assign)
        {
            State.SetFact(i, Transform.GetOperand());
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////