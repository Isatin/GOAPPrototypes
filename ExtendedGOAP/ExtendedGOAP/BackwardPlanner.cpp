// Copyright 2026 Isaac Hsu

#include <cassert>
#include <iostream>
#include <sstream>

#include "Action.h"
#include "BackwardPlanner.h"
#include "BitVector.h"
#include "Fact.h"
#include "State.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string CBackwardPlanner::SNode::ToString() const
{
    std::stringstream Stream;
    Stream << "{Cost=" << GetTotalCost() << "=(" << PreviousCost << "+" << CurrentCost << ")+";

    if (ExtraHeuristicCost == 0.f)
    {
        Stream << BaseHeuristicCost;
    }
    else
    {
        Stream << "(" << BaseHeuristicCost << "+" << ExtraHeuristicCost << ")";
    }

    Stream << " Depth=" << Depth;

    if (ConstState)
    {
        Stream << " {" << ConstState->ToString() << "}";
        Stream << " {" << ConstState->GetDefinition().StringizeFactBits(DesiredFactBits, ",") << "}";
    }
    else
    {
        Stream << " {}";
    }

    Stream << "}";
    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CBackwardPlanner::DoPlan(std::vector<const CAction*>& oSteps, const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions, int MaxDepth)
{
    int Step = 0;
    oSteps.clear();
    mNodes.reserve(Actions.size() * MaxDepth);

    if (ShouldPrintExcessiveHeuristic())
    {
        mPathGetter = [&](int NodeIndex)
        {
            const SNode& Node = mNodes[NodeIndex];
            bool Reached = GoalCondition.IsSatisfiedBy(*Node.ConstState);
            return StringizeBackwardPath(mNodes, NodeIndex, Reached);
        };
    }

    SNode& RootNode = mNodes.emplace_back();
    RootNode.ConstState = &StartingState;
    RootNode.DesiredFactBits = GoalCondition.GetUsedFactBits();
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

        bool Reached = GoalCondition.IsSatisfiedBy(*CurrNode.ConstState);
        if (mFlags & ShowSteps)
        {
            std::cout << "#" << ++Step << " #Nodes=" << mNodes.size();
            std::cout << ' ' << StringizeBackwardPath(mNodes, CurrIndex, Reached) << ' ';
            std::cout << CurrNode.ToString() << std::endl;
        }
        if (Reached)
        {
            if (mFlags & ShowFinalOpenNodes)
            {
                std::cout << StringizeNodes(OpenMap, GoalCondition);
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
            Explore(OpenMap, CurrIndex, *Action, StartingState, GoalCondition);
        }
    }

    return false;
}

void CBackwardPlanner::PostPlan()
{
    mNodes.clear();
}

void CBackwardPlanner::Explore(std::multimap<float, int>& oOpenMap, int NodeIndex, const CAction& Action, const CState& StartingState, const CCondition& GoalCondition)
{
    const CCondition& Precondition = Action.GetPrecondition();
    const CEffect& Effect = Action.GetEffect();
    if (!Effect.HasAnyTransform(mNodes[NodeIndex].DesiredFactBits))
    {
        return;
    }

    int ChildIndex = static_cast<int>(mNodes.size());
    SNode& ChildNode = mNodes.emplace_back();
    SNode& CurrNode = mNodes[NodeIndex];
    ChildNode.Action = &Action;
    ChildNode.MutableState = StartingState.Clone();
    ChildNode.ConstState = ChildNode.MutableState.get();
    ChildNode.Parent = NodeIndex;
    ChildNode.Depth = CurrNode.Depth + 1;
    ApplyEffectsOnPath(*ChildNode.MutableState, ChildIndex); // TODO: Avoid exploring infeasible paths.
    Action.Affect(*ChildNode.MutableState);    
    ChildNode.DesiredFactBits = CurrNode.DesiredFactBits 
        | Effect.GetAffectingFactBits(CurrNode.DesiredFactBits) 
        | Precondition.GetUsedFactBits();
    ChildNode.PreviousCost = CurrNode.GetActualCost();
    ChildNode.CurrentCost = Action.GetCost(*CurrNode.ConstState, *ChildNode.ConstState);
    ChildNode.BaseHeuristicCost = GetBaseHeuristicCost(*ChildNode.ConstState, GoalCondition, ChildIndex);
    ChildNode.ExtraHeuristicCost = ChildNode.ConstState->GetExtraHeuristicCost(GoalCondition);
    float TotalCost = ChildNode.GetTotalCost();
    oOpenMap.emplace(TotalCost, ChildIndex);
}

bool CBackwardPlanner::ApplyEffectsOnPath(CState& oState, int NodeIndex)
{
    while (NodeIndex > 0)
    {
        const SNode& CurrNode = mNodes[NodeIndex];
        assert (CurrNode.Action);
        if (!CurrNode.Action->CheckPrecondition(oState))
        {
            return false;
        }

        CurrNode.Action->GetEffect().ApplyTo(oState);
        NodeIndex = CurrNode.Parent;
    }

    return true;
}

std::string CBackwardPlanner::StringizeNodes(const std::multimap<float, int>& OpenMap, const CCondition& GoalCondition) const
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
        Return += CurrNode.ToString();
        Return += '\n';
    }

    return Return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
