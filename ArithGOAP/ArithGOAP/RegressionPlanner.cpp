// Copyright 2024 Isaac Hsu

#include <iostream>
#include <typeinfo>

#include "Action.h"
#include "Fact.h"
#include "RegressionPlanner.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CRegressionPlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    std::cout << typeid(*this).name() << std::endl;
    if (StartingState.GetDefinition().HasAnyRange())
    {
        std::cout << "RANGE: {" << StartingState.GetDefinition().StringizeBoundedRanges() << "}" << std::endl;
    }
    std::cout << "START: {" << StartingState.ToString() << "}" << std::endl;
    std::cout << "GOAL : {" << GoalState.ToString() << "}" << std::endl;

    int Step = 0;
    oSteps.clear();
    MaxDepth = std::max(MaxDepth, 0);

    std::vector<SNode> Nodes;
    Nodes.reserve(Actions.size() * MaxDepth);

    SNode& RootNode = Nodes.emplace_back();
    RootNode.ConstState = &GoalState;
    RootNode.BaseHeuristicCost = StartingState.GetBaseHeuristicCost(GoalState);
    RootNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(GoalState);

    std::multimap<float, int> OpenMap; // The open set in A*
    OpenMap.emplace(RootNode.GetTotalCost(), 0);

    while (!OpenMap.empty())
    {
        auto itCurr = OpenMap.begin();
        const int CurrIndex = itCurr->second;
        SNode& CurrNode = Nodes[CurrIndex];

        bool Reached = !CurrNode.ConstState->IsContradictory(StartingState);
        std::cout << "#" << ++Step << " #Nodes=" << Nodes.size();
        std::cout << " |" << ((Reached || CurrIndex == 0) ? "" : "? ") << StringizePath(Nodes, CurrIndex) << "| ";
        std::cout << CurrNode.ToString() << std::endl;
        if (Reached)
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
            Explore(OpenMap, Nodes, CurrIndex, *Action, StartingState);
        }
    }

    return false;
}

void CRegressionPlanner::Explore(std::multimap<float, int>& oOpenMap, std::vector<SNode>& Nodes, int NodeIndex, const CAction& Action, const CState& StartingState)
{
    std::unique_ptr<CState> Condition = Action.CheckPostcondition(*Nodes[NodeIndex].ConstState);
    if (!Condition)
    {
        return;
    }

    int ChildIndex = static_cast<int>(Nodes.size());
    SNode& ChildNode = Nodes.emplace_back();
    SNode& CurrNode = Nodes[NodeIndex];
    ChildNode.Action = &Action;
    ChildNode.MutableState = std::move(Condition);
    Action.Affect(*ChildNode.MutableState);
    ChildNode.ConstState = ChildNode.MutableState.get();
    ChildNode.Parent = NodeIndex;
    ChildNode.Depth = CurrNode.Depth + 1;
    ChildNode.PreviousCost = CurrNode.GetActualCost();
    ChildNode.CurrentCost = Action.GetCost(*CurrNode.ConstState, *ChildNode.ConstState);
    ChildNode.BaseHeuristicCost = StartingState.GetBaseHeuristicCost(*ChildNode.ConstState);
    ChildNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(*ChildNode.ConstState);
    float TotalCost = ChildNode.GetTotalCost();
    oOpenMap.emplace(TotalCost, ChildIndex);
}

void CRegressionPlanner::BuildPlan(std::vector<const CAction*>& oSteps, const std::vector<SNode>& Nodes, int NodeIndex)
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

std::string CRegressionPlanner::StringizePath(const std::vector<SNode>& Nodes, int NodeIndex) const
{
    std::string Path;
    bool Successive = false;

    while (NodeIndex >= 0)
    {
        if (Nodes[NodeIndex].Action)
        {
            if (Successive)
            {
                Path += ' ';
            }
            else
            {
                Successive = true;
            }

            Path += Nodes[NodeIndex].Action->GetName();
        }

        NodeIndex = Nodes[NodeIndex].Parent;
    }

    return Path;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

