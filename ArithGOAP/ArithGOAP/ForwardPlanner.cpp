// Copyright 2024 Isaac Hsu

#include <iostream>
#include <map>
#include <sstream>

#include "Action.h"
#include "Fact.h"
#include "ForwardPlanner.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string CForwardPlanner::SNode::ToString() const
{
    std::stringstream Stream;
    Stream << "{Cost=" << GetTotalCost() << "=(" << PreviousCost << "+" << CurrentCost << ")+";

    if (ExtraHeuristicCost == 0.f)
    {
        Stream << BasicHeuristicCost;
    }
    else
    {
        Stream << "(" << BasicHeuristicCost << "+" << ExtraHeuristicCost << ")";
    }

    Stream << " Depth=" << Depth << " {" << (ConstState ? ConstState->ToString() : "") << "}}";
    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CForwardPlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    std::cout << __FUNCTION__ << std::endl;
    std::cout << "RANGE {" << StartingState.GetDefinition().StringizeBoundedRanges() << "}" << std::endl;
    std::cout << "START {" << StartingState.ToString() << "}" << std::endl;
    std::cout << "GOAL  {" << GoalState.ToString() << "}" << std::endl;

    int Step = 0;
    oSteps.clear();

    std::vector<SNode> Nodes;
    Nodes.reserve(Actions.size() * MaxDepth);

    SNode& RootNode = Nodes.emplace_back();
    RootNode.ConstState = &StartingState;
    RootNode.BasicHeuristicCost = StartingState.GetBasicHeuristicCost(GoalState);
    RootNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(GoalState);

    std::multimap<float, int> OpenMap; // a.k.a open set in A*
    OpenMap.emplace(RootNode.GetTotalCost(), 0);

    while (!OpenMap.empty())
    {
        auto itCurr = OpenMap.begin();
        const int CurrIdx = itCurr->second;
        SNode& CurrNode = Nodes[CurrIdx];

        std::cout << "#" << ++Step << " #Nodes=" << Nodes.size() << " |" << GetPathName(Nodes, CurrIdx) << "| " << CurrNode.ToString() << std::endl;

        if (GoalState.HasIntersection(*CurrNode.ConstState))
        {
            BuildPlan(oSteps, Nodes, CurrIdx);
            return true;
        }

        OpenMap.erase(itCurr);

        if (CurrNode.Depth >= MaxDepth)
        {
            continue;
        }

        for (const CAction* Action : Actions)
        {
            Explore(OpenMap, Nodes, CurrIdx, *Action, GoalState);
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

    int ChildIdx = (int) Nodes.size();
    SNode& ChildNode = Nodes.emplace_back();
    SNode& CurrNode = Nodes[NodeIndex];
    ChildNode.Action = &Action;
    ChildNode.MutableState = CurrNode.ConstState->Clone();
    Action.GetEffect().ApplyTo(*ChildNode.MutableState);
    Action.Affect(*ChildNode.MutableState);
    ChildNode.ConstState = ChildNode.MutableState.get();
    ChildNode.Parent = NodeIndex;
    ChildNode.Depth = CurrNode.Depth + 1;
    ChildNode.PreviousCost = CurrNode.GetActualCost();
    ChildNode.CurrentCost = Action.GetCost(*CurrNode.ConstState, *ChildNode.ConstState);
    ChildNode.BasicHeuristicCost = ChildNode.ConstState->GetBasicHeuristicCost(GoalState);
    ChildNode.ExtraHeuristicCost = ChildNode.ConstState->GetExtraHeuristicCost(GoalState);
    float TotalCost = ChildNode.GetTotalCost();
    oOpenMap.emplace(TotalCost, ChildIdx);
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
}

std::string CForwardPlanner::GetPathName(const std::vector<SNode>& Nodes, int NodeIndex)
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

    std::stringstream Stream;
    bool First = true;
    for (auto it = ActionNames.rbegin(); it != ActionNames.rend(); it++)
    {
        if (First)
        {
            First = false;
        }
        else
        {
            Stream << " ";
        }

        Stream << *it;
    }

    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
