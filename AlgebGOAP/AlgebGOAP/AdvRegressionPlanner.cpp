// Copyright 2025 Isaac Hsu

#include <cassert>
#include <iostream>
#include <sstream>

#include "Action.h"
#include "AdvRegressionPlanner.h"
#include "Fact.h"
#include "State.h"


using namespace AlgebGOAP;
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CAdvRegressionPlanner::PrePlan(const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions)
{
    ReplaceFixedFacts(StartingState, GoalCondition, Actions);

    if (mFlags & ShowAlteredActions)
    {
        std::cout << "MODIFIED ACTIONS:\n";
    }

    // Remove actions with impossible preconditions or neutral effects.
    const CStateBase& EmptyState = StartingState.GetDefinition().GetEmptyState();
    for (int i = static_cast<int>(Actions.size()) - 1; i >= 0; i--)
    {
        CAction* Action = Actions[i];
        ETriState Result = Action->GetPrecondition().GetConstraint().Evaluate(EmptyState);
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

void CAdvRegressionPlanner::ReplaceFixedFacts(const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions)
{
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

    // Build a list of fixed facts.
    for (int FactIndex = 0; FactIndex < StartingState.GetPropertyCapacity(); FactIndex++)
    {
        CNumber Value = StartingState.GetProperty(FactIndex);
        if (Value.IsNull())
        {
            continue;
        }

        if (MutableFacts[FactIndex])
        {
            continue;
        }

        FixedFacts.emplace_back(FactIndex, Value);
    }

    GoalCondition.ReplaceFacts(FixedFacts);
    GoalCondition.Rearrange();

    for (CAction* Action : Actions)
    {
        Action->ReplaceFacts(FixedFacts);
        Action->Rearrange();
    }
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
