// Copyright 2026 Isaac Hsu

#include <iostream>
#include <map>
#include <sstream>

#include "Action.h"
#include "Condition.h"
#include "Fact.h"
#include "ForwardPlanner.h"
#include "State.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string CForwardPlanner::SNode::ToString() const
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

    Stream << " Depth=" << Depth << " {" << (ConstState ? ConstState->ToString() : "") << "}}";
    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CForwardPlanner::DoPlan(std::vector<const CAction*>& oSteps, const CState& StartingState, CCondition& GoalCondition, std::vector<CAction*>& Actions, int MaxDepth)
{
    int Step = 0;
    oSteps.clear();
    mNodes.reserve(Actions.size() * MaxDepth);

    if (ShouldPrintExcessiveHeuristic())
    {
        mPathGetter = [&](int NodeIndex)
        {
            return StringizeForwardPath(NodeIndex);
        };
    }

    SNode& RootNode = mNodes.emplace_back();
    RootNode.ConstState = &StartingState;
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

        if (mFlags & ShowSteps)
        {
            std::cout << "#" << ++Step << " #Nodes=" << mNodes.size();
            std::cout << ' ' << StringizeForwardPath(CurrIndex) << ' ' << CurrNode.ToString() << std::endl;
        }

        if (GoalCondition.IsSatisfiedBy(*CurrNode.ConstState))
        {
            if (mFlags & ShowFinalOpenNodes)
            {
                std::cout << StringizeNodes(OpenMap, GoalCondition);
            }

            BuildForwardPlan(oSteps, CurrIndex);
            return true;
        }

        if (CurrNode.Depth >= MaxDepth)
        {
            continue;
        }

        for (const CAction* Action : Actions)
        {
            Explore(OpenMap, CurrIndex, *Action, GoalCondition);
        }
    }

    return false;
}

void CForwardPlanner::PostPlan()
{
    mNodes.clear();
}

void CForwardPlanner::Explore(std::multimap<float, int>& oOpenMap, int NodeIndex, const CAction& Action, const CCondition& GoalCondition)
{
    if (!Action.CheckPrecondition(*mNodes[NodeIndex].ConstState))
    {
        return;
    }

    auto MutableState = mNodes[NodeIndex].ConstState->Clone();
    if (!Action.GetEffect().ApplyTo(*MutableState))
    {
        return;
    }

    int ChildIndex = static_cast<int>(mNodes.size());
    SNode& ChildNode = mNodes.emplace_back();
    SNode& CurrNode = mNodes[NodeIndex];
    ChildNode.Action = &Action;
    ChildNode.MutableState = std::move(MutableState);
    Action.Affect(*ChildNode.MutableState);
    ChildNode.ConstState = ChildNode.MutableState.get();
    ChildNode.Parent = NodeIndex;
    ChildNode.Depth = CurrNode.Depth + 1;
    ChildNode.PreviousCost = CurrNode.GetActualCost();
    ChildNode.CurrentCost = Action.GetCost(*CurrNode.ConstState, *ChildNode.ConstState);
    ChildNode.BaseHeuristicCost = GetBaseHeuristicCost(*ChildNode.ConstState, GoalCondition, ChildIndex);
    ChildNode.ExtraHeuristicCost = ChildNode.ConstState->GetExtraHeuristicCost(GoalCondition);
    float TotalCost = ChildNode.GetTotalCost();
    oOpenMap.emplace(TotalCost, ChildIndex);
}

std::string CForwardPlanner::StringizeNodes(const std::multimap<float, int>& OpenMap, const CCondition& GoalCondition) const
{
    std::string Return;

    for (auto [Cost, NodeIndex] : OpenMap)
    {
        const SNode& CurrNode = mNodes[NodeIndex];
        Return += "Nodes["; 
        Return += std::to_string(NodeIndex); 
        Return += "] ";
        Return += StringizeForwardPath(NodeIndex);
        Return += ' ';
        Return += CurrNode.ToString();
        Return += '\n';
    }

    return Return;
}

void CForwardPlanner::BuildForwardPlan(std::vector<const CAction*>& oSteps, int NodeIndex) const
{
    BuildBackwardPlan(oSteps, mNodes, NodeIndex);
    std::reverse(oSteps.begin(), oSteps.end());
}

std::string CForwardPlanner::StringizeForwardPath(int NodeIndex) const
{
    std::vector<std::string> ActionNames;
    while (NodeIndex >= 0)
    {
        const SNode& CurrNode = mNodes[NodeIndex];
        if (CurrNode.Action)
        {
            ActionNames.push_back(CurrNode.Action->GetName());
        }

        NodeIndex = CurrNode.Parent;
    }

    std::string Path = "|";
    bool Successive = false;
    for (auto it = ActionNames.rbegin(); it != ActionNames.rend(); it++)
    {
        if (Successive)
        {
            Path += ' ';
        }
        else
        {
            Successive = true;
        }

        Path += *it;
    }

    Path += "|";
    return Path;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
