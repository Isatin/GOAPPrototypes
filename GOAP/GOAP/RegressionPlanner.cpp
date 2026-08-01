// Copyright 2024 Isaac Hsu

#include <iostream>
#include <typeinfo>

#include "Action.h"
#include "Node.h"
#include "RegressionPlanner.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CRegressionPlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
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
    RootNode.ConstState = &GoalState;
    RootNode.BaseHeuristicCost = static_cast<float>(GoalState.CountUnsatisfiedProperties(StartingState));
    RootNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(GoalState);

    std::multimap<float, int> OpenMap; // The open set in A*
    OpenMap.emplace(RootNode.GetTotalCost(), 0);

    while (!OpenMap.empty())
    {
        auto itCurr = OpenMap.begin();
        const int CurrIndex = itCurr->second;
        SNode& CurrNode = Nodes[CurrIndex];

        bool Reached = CurrNode.ConstState->IsSatisfiedBy(StartingState);
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
    if (!Action.CheckPostcondition(*Nodes[NodeIndex].ConstState))
    {
        return;
    }

    int ChildIndex = static_cast<int>(Nodes.size());
    SNode& ChildNode = Nodes.emplace_back();
    SNode& CurrNode = Nodes[NodeIndex];
    ChildNode.Action = &Action;
    ChildNode.MutableState = CurrNode.ConstState->Clone();
    ChildNode.MutableState->RemoveMatch(Action.GetEffect());        // Remove satisfied properties in the desired state.
    Action.GetPrecondition().Overwrite(*ChildNode.MutableState);    // Add the preconditions to the desired state as new constraints.
    Action.Affect(*ChildNode.MutableState);
    ChildNode.ConstState = ChildNode.MutableState.get();
    ChildNode.Parent = NodeIndex;
    ChildNode.Depth = CurrNode.Depth + 1;
    ChildNode.PreviousCost = CurrNode.GetActualCost();
    ChildNode.CurrentCost = Action.GetCost(*CurrNode.ConstState, *ChildNode.ConstState);
    ChildNode.BaseHeuristicCost = static_cast<float>(ChildNode.ConstState->CountUnsatisfiedProperties(StartingState));
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
