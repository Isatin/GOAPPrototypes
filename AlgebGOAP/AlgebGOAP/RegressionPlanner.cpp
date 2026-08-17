// Copyright 2025 Isaac Hsu

#include <cassert>
#include <iostream>
#include <sstream>

#include "Action.h"
#include "BitVector.h"
#include "Fact.h"
#include "RegressionPlanner.h"
#include "State.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CRegressionPlanner::DoPlan(std::vector<const CAction*>& oSteps, const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions, int MaxDepth)
{
    int Step = 0;
    oSteps.clear();
    mNodes.reserve(Actions.size() * MaxDepth);

    if (ShouldPrintExcessiveHeuristic())
    {
        mPathGetter = [&](int NodeIndex)
        {
            const SNode& Node = mNodes[NodeIndex];
            bool Reached = Node.ConstCondition->IsSatisfiedBy(StartingState);
            return StringizeBackwardPath(mNodes, NodeIndex, Reached);
        };
    }

    SNode& RootNode = mNodes.emplace_back();
    RootNode.ConstCondition = &GoalCondition;
    RootNode.BaseHeuristicCost = GetBaseHeuristicCost(StartingState, GoalCondition, 0);
    RootNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(GoalCondition);

    std::multimap<float, int> OpenMap; // The open set in A*
    OpenMap.emplace(RootNode.GetTotalCost(), 0);

    while (!OpenMap.empty())
    {
        auto itCurr = OpenMap.begin();
        const int CurrIndex = itCurr->second;
        OpenMap.erase(itCurr);
        SNode& CurrNode = mNodes[CurrIndex];

        bool Reached = CurrNode.ConstCondition->IsSatisfiedBy(StartingState);
        if (mFlags & ShowSteps)
        {
            std::cout << "#" << ++Step << " #Nodes=" << mNodes.size();
            std::cout << ' ' << StringizeBackwardPath(mNodes, CurrIndex, Reached) << ' ';
            std::cout << StringizeNode(CurrNode) << std::endl;
        }
        if (Reached)
        {
            if (mFlags & ShowFinalOpenNodes)
            {
                std::cout << StringizeNodes(OpenMap, StartingState);
            }

            BuildBackwardPlan(oSteps, mNodes, CurrIndex);
            return true;
        }

        if (CurrNode.Depth >= MaxDepth)
        {
            continue;
        }

        for (const CAction* Action : Actions)
        {
            Explore(OpenMap, CurrIndex, *Action, StartingState);
        }
    }

    return false;
}

void CRegressionPlanner::PostPlan()
{
    mNodes.clear();
}

void CRegressionPlanner::Explore(std::multimap<float, int>& oOpenMap, int NodeIndex, const CAction& Action, const CState& StartingState)
{
    std::unique_ptr<CCondition> DesiredCondition = CheckPostcondition(*mNodes[NodeIndex].ConstCondition, Action);
    if (!DesiredCondition)
    {
        return;
    }

    int ChildIndex = static_cast<int>(mNodes.size());
    SNode& ChildNode = mNodes.emplace_back();
    SNode& CurrNode = mNodes[NodeIndex];

    ChildNode.Action = &Action;
    ChildNode.MutableCondition = std::move(DesiredCondition);
    ChildNode.ConstCondition = ChildNode.MutableCondition.get();
    ChildNode.Parent = NodeIndex;
    ChildNode.Depth = CurrNode.Depth + 1;
    Action.Affect(*ChildNode.MutableCondition);
    ChildNode.PreviousCost = CurrNode.GetActualCost();
    ChildNode.CurrentCost = Action.GetCost(*CurrNode.ConstCondition, *ChildNode.ConstCondition);
    ChildNode.BaseHeuristicCost = GetBaseHeuristicCost(StartingState, *ChildNode.ConstCondition, ChildIndex);
    ChildNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(*ChildNode.ConstCondition);
    float TotalCost = ChildNode.GetTotalCost();
    oOpenMap.emplace(TotalCost, ChildIndex);
}

std::unique_ptr<CCondition> CRegressionPlanner::CheckPostcondition(const CCondition& Postcondition, const CAction& Action) const
{
    assert(&Postcondition.GetDefinition() == &Action.GetDefinition());

    const CEffect& Effect = Action.GetEffect();
    if (!Effect.HasAnyTransform(Postcondition.GetUsedFactBits()))
    {
        return {};
    }

    std::unique_ptr<CCondition> Condition = Postcondition.Clone();
    if (!Condition)
    {
        return {};
    }

    // Reverse the effect by substituting the expressions into the postcondition.
    // Apply the transforms in reverse order because operations are executed backwards in the regressive search.
    CSimultaneousFormulas& Constraint = Condition->GetConstraint();
    for (int TransformIndex = Effect.GetTransformCount() - 1; TransformIndex >= 0; TransformIndex--)
    {
        const STransform& Transform = Effect.GetTransform(TransformIndex);
        Constraint.ReplaceFact(Transform.FactIndex, Transform.Expression);
    }

    ConjoinCondition(*Condition, Action.GetPrecondition());

    switch (Constraint.Evaluate(Postcondition.GetDefinition().GetEmptyState()))
    {
    case ETriState::no:
        return {}; // The action is infeasible due to one or more conflicts between the postcondition and the effect.
    case ETriState::yes:
        Condition->ClearConstraint(); // Remove satisfied conditions.
        break;
    }

    return Condition;
}

void CRegressionPlanner::ConjoinCondition(CCondition& oLeft, const CCondition& Right) const
{
    oLeft.GetConstraint().Unite(Right.GetConstraint());
    oLeft.GetConstraint().RemoveRedundancy(oLeft.GetDefinition());
}

std::string CRegressionPlanner::StringizeNodes(const std::multimap<float, int>& OpenMap, const CState& StartingState) const
{
    std::string Return;

    for (auto [Cost, NodeIndex] : OpenMap)
    {
        const SNode& CurrNode = mNodes[NodeIndex];
        Return += "Nodes[";
        Return += std::to_string(NodeIndex);
        Return += "] ";
        Return += mPathGetter(NodeIndex);
        Return += ' ';
        Return += StringizeNode(CurrNode);
        Return += '\n';
    }

    return Return;
}

std::string CRegressionPlanner::StringizeNode(const SNode& Node) const
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
        Stream << ' ' << Node.ConstCondition->ToString();
    }
    else
    {
        Stream << ' ' << CFormula::GetEmptySymbol();
    }

    Stream << "}";
    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
