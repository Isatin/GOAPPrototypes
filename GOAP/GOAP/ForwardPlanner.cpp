// Copyright 2024 Isaac Hsu

#include <iostream>
#include <map>
#include <typeinfo>

#include "Action.h"
#include "ForwardPlanner.h"
#include "Node.h"


using namespace GOAP;

bool CForwardPlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    std::cout << typeid(*this).name() << std::endl;
    std::cout << "START: {" << StartingState.ToString() << "}" << std::endl;
    std::cout << "GOAL : {" << GoalState.ToString() << "}" << std::endl;

    int Step = 0;
    oSteps.clear();
    MaxDepth = std::max(MaxDepth, 0);

    std::vector<SNode> Nodes;
    Nodes.reserve(Actions.size() * MaxDepth);

    SNode& RootNode = Nodes.emplace_back();
    RootNode.ConstState = &StartingState;
    RootNode.BaseHeuristicCost = static_cast<float>(GoalState.CountUnsatisfiedProperties(StartingState));
    RootNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(GoalState);

    std::multimap<float, int> OpenMap; // The open set in A*
    OpenMap.emplace(RootNode.GetTotalCost(), 0);

    while (!OpenMap.empty())
    {
        auto itCurr = OpenMap.begin();
        const int CurrIndex = itCurr->second;
        SNode& CurrNode = Nodes[CurrIndex];

        std::cout << "#" << ++Step << " #Nodes=" << Nodes.size() << " |" << StringizePath(Nodes, CurrIndex) << "| " << CurrNode.ToString() << std::endl;

        if (GoalState.IsSatisfiedBy(*CurrNode.ConstState))
        {
            BuildPlan(oSteps, Nodes, CurrIndex);
            return true;
        }

        OpenMap.erase(itCurr);

        if (CurrNode.Depth >= MaxDepth)
        {
            continue;
        }

        for (const CAction* Action : Actions)
        {
            Explore(OpenMap, Nodes, CurrIndex, *Action, GoalState);
        }
    }

    return false;
}

void CForwardPlanner::Explore(std::multimap<float, int>& oOpenMap, std::vector<SNode>& Nodes, int NodeIndex, const CAction& Action, const CState& GoalState)
{
    if (!Action.CheckPrecondition(*Nodes[NodeIndex].ConstState))
    {
        return;
    }

    int ChildIndex = static_cast<int>(Nodes.size());
    SNode& ChildNode = Nodes.emplace_back();
    SNode& CurrNode = Nodes[NodeIndex];
    ChildNode.Action = &Action;
    ChildNode.MutableState = CurrNode.ConstState->Clone();
    Action.GetEffect().Overwrite(*ChildNode.MutableState);
    Action.Affect(*ChildNode.MutableState);
    ChildNode.ConstState = ChildNode.MutableState.get();
    ChildNode.Parent = NodeIndex;
    ChildNode.Depth = CurrNode.Depth + 1;
    ChildNode.PreviousCost = CurrNode.GetActualCost();
    ChildNode.CurrentCost = Action.GetCost(*CurrNode.ConstState, *ChildNode.ConstState);
    ChildNode.BaseHeuristicCost = static_cast<float>(GoalState.CountUnsatisfiedProperties(*ChildNode.ConstState));
    ChildNode.ExtraHeuristicCost = ChildNode.ConstState->GetExtraHeuristicCost(GoalState);
    float TotalCost = ChildNode.GetTotalCost();
    oOpenMap.emplace(TotalCost, ChildIndex);
}

void CForwardPlanner::BuildPlan(std::vector<const CAction*>& oSteps, const std::vector<SNode>& Nodes, int NodeIndex)
{
    while (NodeIndex >= 0)
    {
        if (Nodes[NodeIndex].Action)
        {
            oSteps.push_back(Nodes[NodeIndex].Action);
        }

        NodeIndex = Nodes[NodeIndex].Parent;
    }

    std::reverse(oSteps.begin(), oSteps.end());
}

std::string CForwardPlanner::StringizePath(const std::vector<SNode>& Nodes, int NodeIndex) const
{
    std::vector<std::string> ActionNames;
    while (NodeIndex >= 0)
    {
        if (Nodes[NodeIndex].Action)
        {
            ActionNames.push_back(Nodes[NodeIndex].Action->GetName());
        }

        NodeIndex = Nodes[NodeIndex].Parent;
    }

    std::string Path;
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

    return Path;
}

