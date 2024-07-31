// Copyright 2024 Isaac Hsu

#include <cassert>
#include <sstream>

#include "Action.h"
#include "Fact.h"
#include "Notation.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CAction::CAction(const std::string& Name, const CStateDefinition& Def)
    : mName(Name)
    , mPrecondition(Def)
    , mEffect(Def)
{
}

std::string CAction::ToString() const
{
    std::stringstream Stream;
    Stream << mName << "{ ";

    std::string PreconditionStr = mPrecondition.ToString();
    if (!PreconditionStr.empty())
    {
        Stream << PreconditionStr << " ";
    }

    Stream << "=>";

    std::string EffectStr = mEffect.ToString();
    if (!EffectStr.empty())
    {
        Stream << " " << EffectStr;
    }

    Stream <<  " }";
    return Stream.str();
}

bool CAction::CheckPrecondition(const CState& State) const
{
    if (!mPrecondition.HasIntersection(State))
    {
        return false;
    }

    if (mEffect.IsNil())
    {
        return false; // Ineffective action
    }

    return true;
}

std::optional<std::vector<SInterval>> CAction::CheckPostcondition(const CState& State) const
{
    // This action is considered as a candidate in regressive search if it can satisfy at least one desired fact
    // and the current desired state has no conflicts with its effects and preconditions.
    assert(&State.GetDefinition() == &GetDefinition());
    auto Results = State.GetFacts(); // Copy facts for later modification
    bool AnySatisfaction = false;

    for (int i = 0; i < mEffect.GetTransformAmount(); i++)
    {
        const CTransform& Transform = mEffect.GetTransform(i);
        if (!Transform)
        {
            continue;
        }

        SInterval& Target = Results[i];
        if (!Target)
        {
            continue; // Ignore redundant effect
        }

        const CFact* Fact = GetDefinition().GetFact(i);
        const SInterval Range = Fact ? Fact->GetRange() : SInterval::Boundless;

        switch (Transform.Neutralize(Target, Range))
        {
        case ETriStateCompletion::failed:
            return std::nullopt;        // Unfeasible effect
        case ETriStateCompletion::complete:
            AnySatisfaction = true;
            Target = SInterval::Unset;  // Erase fully satisfied fact
            break;
        case ETriStateCompletion::partial:
            AnySatisfaction = true;     // Partial satisfaction is okay.
            break;
        }
    }

    if (!AnySatisfaction)
    {
        return std::nullopt; // None of the effects can satisfy any desired facts.
    }

    for (int i = 0; i < mPrecondition.GetFactAmount(); i++)
    {
        const SInterval& Constraint = mPrecondition.GetFact(i);
        if (!Constraint)
        {
            continue;
        }

        SInterval& Target = Results[i];
        if (!Target) 
        {
            Target = Constraint; // Add precondition to desired state for this action to execute
        }
        else if (!Target.Intersect(Constraint)) // Find common interval between precondition and desired state
        {
            return std::nullopt; // Conflict between precondition and desired state
        }
    }

    return Results;
}
///////////////////////////////////////////////////////////////////////////////////////////////////