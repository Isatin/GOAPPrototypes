// Copyright 2026 Isaac Hsu

#include <cassert>
#include <iostream>
#include <map>
#include <typeinfo>

#include "BasePlanner.h"
#include "Action.h"
#include "Fact.h"
#include "State.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CBasePlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    MaxDepth = std::max(MaxDepth, 0);
    mDebugInfo.Clear();

    if (mFlags & ShowMethodName)
    {
        std::cout << typeid(*this).name() << std::endl;
    }

    if (mFlags & ShowActions)
    {
        for (const CAction* Action : Actions)
        {
            std::cout << Action->ToString() << std::endl;
        }
    }

    // It's difficult to estimate the heuristic for logical formulas with negation. 
    // Thus, we apply De Morgan's laws to push logical NOTs into the inner relational expressions.  
    // So, we only need to calculate the heuristics for negated relational expressions instead.
    std::unique_ptr<CCondition> ModifiedGoal = GoalCondition.Clone(); // Clone the goal for later modification.
    ModifiedGoal->GetConstraint().ApplyDeMorgansLaws();

    std::vector<std::unique_ptr<CAction>> ModifiedActions;  // A vector for holding cloned actions
    std::vector<CAction*> ModifiableActions;                // A modifiable vector of non-const actions to pass to DoPlan
    ModifiedActions.reserve(Actions.size());
    ModifiableActions.reserve(Actions.size());
    for (const CAction* Action : Actions)
    {
        std::unique_ptr<CAction> ModifiedAction = Action->Clone(); // Clone the action for later modification.
        ModifiedAction->GetPrecondition().GetConstraint().ApplyDeMorgansLaws();
        auto& ModifiableAction = ModifiedActions.emplace_back(std::move(ModifiedAction));
        ModifiableActions.push_back(ModifiableAction.get());
    }

    PrePlan(StartingState, *ModifiedGoal, ModifiableActions);
    bool Return = DoPlan(oSteps, StartingState, *ModifiedGoal, ModifiableActions, MaxDepth);

    if (ShouldPrintExcessiveHeuristic())
    {
        PrintExcessiveHeuristic(StartingState.GetDefinition());
    }

    if (mFlags & ShowResult)
    {
        if (Return)
        {
            std::cout << "SUCCEEDED |" << StringizeSteps(oSteps) << "| \n";
        }
        else
        {
            std::cout << "FAILED\n";
        }
    }

    PostPlan();
    mDebugInfo.Clear();
    mPathGetter = nullptr;

    return Return;
}

void CBasePlanner::PrePlan(const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions)
{
    if (mFlags & ShowStartAndGoal)
    {
        std::cout << "START: {" << StartingState.ToString() << "}" << std::endl;
        std::cout << "GOAL : " << GoalCondition.ToString() << std::endl;
    }
}

std::string CBasePlanner::StringizeSteps(const std::vector<const CAction*>& Steps) const
{
    std::string Return;
    bool Successive = false;

    for (const CAction* Action : Steps)
    {
        if (!Action)
        {
            break;
        }

        if (Successive)
        {
            Return += ' ';
        }
        else
        {
            Successive = true;
        }

        Return += Action->GetName();
    }

    return Return;
}

float CBasePlanner::GetBaseHeuristicCost(const CState& State, const CCondition& Condition, int NodeIndex)
{
    assert(mDebugInfo.CurrentNodeIndex == InvalidIndex);

    float Return = 0.f;
    if (ShouldPrintExcessiveHeuristic())
    {
        mDebugInfo.CurrentNodeIndex = NodeIndex;
        Return = State.GetBaseHeuristicCost(Condition, &mDebugInfo);
        mDebugInfo.CurrentNodeIndex = InvalidIndex;
    }
    else
    {
        Return = State.GetBaseHeuristicCost(Condition);
    }
    return Return;
}

void CBasePlanner::PrintExcessiveHeuristic(const CFactDefinition& Definition) const
{
    assert(mPathGetter);

    // Find facts whose max heuristic values are from the same search node and constraint.
    std::map<std::pair<int, std::string>, std::vector<int>> NodeConstraintToFactsMap;
    for (int FactIndex = 0; FactIndex < mDebugInfo.HeuristicTuples.size(); FactIndex++)
    {
        const SDebugHeuristicTuple& Tuple = mDebugInfo.HeuristicTuples[FactIndex];
        if (Tuple.MaxHeuristic.IsNull())
        {
            continue;
        }

        if (Tuple.MaxHeuristic.Get() <= Definition.GetBaseRelationCost().Get())
        {
            continue;
        }

        NodeConstraintToFactsMap[std::make_pair(Tuple.NodeIndex, Tuple.Constraint)].push_back(FactIndex);
    }

    bool TitlePrinted = false;
    const char* TimesSymbol = EOperator(EOperator::multiplication).GetSymbol();
    for (const auto& [Pair, FactIndexes] : NodeConstraintToFactsMap)
    {
        assert(!FactIndexes.empty());
        const SDebugHeuristicTuple& Tuple = mDebugInfo.HeuristicTuples[FactIndexes.front()];
        const auto& [NodeIndex, Constraint] = Pair;

        bool Successive = false;
        for (int IndexOfFactIndexes = 0; IndexOfFactIndexes < FactIndexes.size(); IndexOfFactIndexes++)
        {
            int FactIndex = FactIndexes[IndexOfFactIndexes];
            if (const CFact* Fact = Definition.GetFact(FactIndex))
            {
                if (!TitlePrinted)
                {
                    TitlePrinted = true;
                    std::cout << "EXCESSIVE HEURISTIC:\n";
                }

                if (Successive)
                {
                    std::cout << ", ";
                }
                else
                {
                    Successive = true;
                }

                std::cout << Fact->GetName();
            }
        }

        
        std::string Path = mPathGetter(NodeIndex);
        std::cout << ": " << Path;
        std::cout << " h( " << Constraint << ", {" << Tuple.State << "}";
        if (!Tuple.GapWeightOfMaxHeuristic.IsNull())
        {
            std::cout << ", " << TimesSymbol << Tuple.GapWeightOfMaxHeuristic;
        }
        std::cout << " ) = " << Tuple.MaxHeuristic;
        std::cout << " > " << Definition.GetBaseRelationCost() << std::endl;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////