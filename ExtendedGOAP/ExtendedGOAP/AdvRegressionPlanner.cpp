// Copyright 2026 Isaac Hsu

#include <cassert>
#include <iostream>
#include <sstream>

#include "Action.h"
#include "AdvRegressionPlanner.h"
#include "Fact.h"
#include "State.h"


using namespace ExtendedGOAP;
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAdvRegressionPlanner::PrePlan(const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions)
{
    if (mFlags & ShowAlteredActions)
    {
        std::cout << "MODIFIED ACTIONS:\n";
    }

    // Remove actions with impossible preconditions or neutral effects.
    CState ConstantState = ReplaceFixedFacts(StartingState, GoalCondition, Actions);
    for (int i = static_cast<int>(Actions.size()) - 1; i >= 0; i--)
    {
        CAction* Action = Actions[i];
        ETriState Result = Action->GetPrecondition().GetConstraint().Evaluate(ConstantState);
        if (Result.IsNo() || Action->GetEffect().IsNeutral())
        {
            if (mFlags & ShowAlteredActions)
            {
                std::cout << "REDUNDANCY: " << Action->ToString() << std::endl;
            }

            Actions.erase(Actions.begin() + i);
        }
        else if (Result.IsYes())
        {
            Action->GetPrecondition().ClearConstraint(); // Remove trivial preconditions.
        }
    }

    if (mFlags & ShowAlteredActions)
    {
        for (const CAction* Action : Actions)
        {
            std::cout << Action->ToString() << std::endl;
        }
    }

    if (mFlags & ShowStartAndGoal)
    {
        std::cout << "START: {" << StartingState.ToString() << "}" << std::endl;
        std::cout << "GOAL : {" << GoalCondition.ToString() << "}" << std::endl;
    }
}

void CAdvRegressionPlanner::ConjoinCondition(CCondition& oLeft, const CCondition& Right) const
{
    oLeft.GetConstraint().Append(Right.GetConstraint());
    oLeft.GetConstraint().Rearrange(oLeft.GetDefinition());
}

CState CAdvRegressionPlanner::ReplaceFixedFacts(const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions)
{
    // TODO: Replace not only fixed scalars but also matrices and sets.

    CState ConstantState(StartingState.GetDefinition());

    std::vector<std::pair<int, CNumber>> FixedFacts;
    FixedFacts.reserve(StartingState.GetPropertyCapacity());

    // Build a bit vector for mutable facts.
    CBitVector MutableFacts(StartingState.GetPropertyCapacity());
    for (const CAction* Action : Actions)
    {
        if (Action)
        {
            MutableFacts |= Action->GetEffect().GetAffectedFactBits();
        }
    }

    // Build a list of fixed scalar facts.
    for (int FactIndex = 0; FactIndex < StartingState.GetPropertyCapacity(); FactIndex++)
    {
        const CVariant& Variant = StartingState.GetProperty(FactIndex);
        if (Variant.IsUnset())
        {
            continue;
        }

        if (MutableFacts[FactIndex])
        {
            continue;
        }

        ConstantState.SetProperty(FactIndex, Variant);

        if (Variant.IsScalar())
        {
            FixedFacts.emplace_back(FactIndex, Variant.GetScalar());
        }
    }

    GoalCondition.ReplaceFacts(FixedFacts);
    GoalCondition.Rearrange();

    for (CAction* Action : Actions)
    {
        Action->ReplaceFacts(FixedFacts);
        Action->Rearrange();
    }

    return ConstantState;
}

std::string CAdvRegressionPlanner::StringizeNode(const SNode& Node) const
{
    std::stringstream Stream;
    Stream << "{Cost=" << Node.GetTotalCost() << "=(" << Node.PreviousCost << "+" << Node.CurrentCost << ")+";

    if (Node.ExtraHeuristicCost == 0.f)
    {
        Stream << Node.BaseHeuristicCost;
    }
    else
    {
        Stream << "(" << Node.BaseHeuristicCost << "+" << Node.ExtraHeuristicCost << ")";
    }

    Stream << " Depth=" << Node.Depth;

    if (Node.ConstCondition)
    {
        if (Node.ConstCondition->IsEmpty())
        {
            Stream << ' ' << CSimultaneousFormulas::GetEmptySymbol();
        }
        else
        {
            Stream << " {" << Node.ConstCondition->ToString() << "}";
        }
    }
    else
    {
        Stream << ' ' << CSimultaneousFormulas::GetEmptySymbol();
    }

    Stream << "}";
    return Stream.str();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
