// Copyright 2024 Isaac Hsu

#include <cassert>

#include "Action.h"
#include "Fact.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CAction::CAction(const std::string& Name, const CFactDefinition& Definition)
    : mName(Name)
    , mPrecondition(Definition)
    , mEffect(Definition)
{}

std::string CAction::ToString() const
{
    std::string Return = mName;
    Return += "{ ";

    if (!mPrecondition.IsEmpty())
    {
        Return += mPrecondition.ToString();
        Return += ' ';
    }

    Return += "->";

    std::string EffectText = mEffect.ToString();
    if (!EffectText.empty())
    {
        Return += ' ';
        Return += EffectText;
    }

    Return += " }";
    return Return;
}

bool CAction::CheckPrecondition(const CState& State) const
{
    if (mPrecondition.IsContradictory(State))
    {
        return false; // The precondition is unmet.
    }

    if (mEffect.IsEmpty())
    {
        return false; // This action is ineffective.
    }

    return true;
}

std::unique_ptr<CState> CAction::CheckPostcondition(const CState& Postcondition) const
{
    // This action is considered a candidate in the regressive search if it can satisfy at least one desired property,
    // and its effects and preconditions do not conflict with the given desired state.
    assert(&Postcondition.GetDefinition() == &GetDefinition());

    std::unique_ptr<CState> Condition = Postcondition.Clone();
    bool AnySatisfaction = false;
    CNumber Tolerance = GetDefinition().GetTolerance();

    for (int FactIndex = 0; FactIndex < mEffect.GetTransformCapacity(); FactIndex++)
    {
        const CTransform& Transform = mEffect.GetTransform(FactIndex);
        if (Transform.IsNil())
        {
            continue; // Skip unset effects.
        }

        SSegment& Target = Condition->GetProperty(FactIndex);
        if (Target.IsUnset())
        {
            continue; // Skip unset constraints.
        }

        const CFact* Fact = GetDefinition().GetFact(FactIndex);
        const SSegment Range = Fact ? Fact->GetRange() : SSegment::Boundless;

        switch (Transform.Reserve(Target, Range, Tolerance))
        {
        case ETriStateCompletion::failed:
            return {};                  // This action is infeasible due to the conflicting effect.
        case ETriStateCompletion::complete:
            AnySatisfaction = true;
            Target = SSegment::Unset;   // Erase the fully satisfied property.
            break;
        case ETriStateCompletion::partial:
            AnySatisfaction = true;     // Partial satisfaction is okay.
            break;
        }
    }

    if (!AnySatisfaction)
    {
        return {}; // This action cannot satisfy any desired properties.
    }

    for (int FactIndex = 0; FactIndex < mPrecondition.GetPropertyCapacity(); FactIndex++)
    {
        const SSegment& Constraint = mPrecondition.GetProperty(FactIndex);
        if (Constraint.IsUnset())
        {
            continue;
        }

        SSegment& Target = Condition->GetProperty(FactIndex);
        if (Target.IsUnset())
        {
            Target = Constraint; // Add the precondition to the desired state if it does not exist.
        }
        else if (!Target.Intersect(Constraint, Tolerance)) // Find the intersection between the precondition and desired state.
        {
            return {}; // This action is infeasible due to a conflict between the precondition and desired state.
        }
    }

    return Condition;
}
///////////////////////////////////////////////////////////////////////////////////////////////////