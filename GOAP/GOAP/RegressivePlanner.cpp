// Copyright 2024 Isaac Hsu

#include <iostream>
#include <sstream>

#include "Action.h"
#include "Node.h"
#include "RegressivePlanner.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CRegressivePlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    std::cout << __FUNCTION__ << std::endl;
    std::cout << "START {" << StartingState.ToString() << "}" << std::endl;
    std::cout << "GOAL  {" << GoalState.ToString() << "}" << std::endl;

    int Step = 0;
    oSteps.clear();

    std::vector<SNode> Nodes;
    Nodes.reserve(Actions.size() * MaxDepth);

    SNode& RootNode = Nodes.emplace_back();
    RootNode.ConstState = &GoalState;
    RootNode.BasicHeuristicCost = (float) GoalState.GetUnsatisfactionCount(StartingState);
    RootNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(GoalState);

    std::multimap<float, int> OpenMap; // a.k.a open set in A*
    OpenMap.emplace(RootNode.GetTotalCost(), 0);

    while (!OpenMap.empty())
    {
        auto itCurr = OpenMap.begin();
        const int CurrIdx = itCurr->second;
        SNode& CurrNode = Nodes[CurrIdx];

        bool Reached = CurrNode.ConstState->IsSatisfiedBy(StartingState);
        std::cout << "#" << ++Step << " #Nodes=" << Nodes.size() << " |" << ((Reached || CurrIdx == 0) ? "" : "? ") << GetPathName(Nodes, CurrIdx) << "| " << CurrNode.ToString() << std::endl;
        if (Reached)
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
            Explore(OpenMap, Nodes, CurrIdx, *Action, StartingState);
        }
    }

    return false;
}

void CRegressivePlanner::Explore(std::multimap<float, int>& oOpenMap, std::vector<SNode>& Nodes, int NodeIndex, const CAction& Action, const CState& StartingState)
{
    if (!Action.CheckPostcondition(*Nodes[NodeIndex].ConstState))
    {
        return;
    }

    int ChildIdx = (int) Nodes.size();
    SNode& ChildNode = Nodes.emplace_back();
    SNode& CurrNode = Nodes[NodeIndex];
    ChildNode.Action = &Action;
    ChildNode.MutableState = CurrNode.ConstState->Clone();
    ChildNode.MutableState->RemoveMatch(Action.GetEffect());
    Action.GetPrecondition().Overwrite(*ChildNode.MutableState);
    Action.Affect(*ChildNode.MutableState);
    ChildNode.ConstState = ChildNode.MutableState.get();
    ChildNode.Parent = NodeIndex;
    ChildNode.Depth = CurrNode.Depth + 1;
    ChildNode.PreviousCost = CurrNode.GetActualCost();
    ChildNode.CurrentCost = Action.GetCost(*CurrNode.ConstState, *ChildNode.ConstState);
    ChildNode.BasicHeuristicCost = (float) ChildNode.ConstState->GetUnsatisfactionCount(StartingState);    
    ChildNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(*ChildNode.ConstState);
    float TotalCost = ChildNode.GetTotalCost();
    oOpenMap.emplace(TotalCost, ChildIdx);
}

void CRegressivePlanner::BuildPlan(std::vector<const CAction*>& oSteps, const std::vector<SNode>& Nodes, int NodeIndex)
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

std::string CRegressivePlanner::GetPathName(const std::vector<SNode>& Nodes, int NodeIndex)
{
    std::stringstream Stream;
    bool First = true;

    while (NodeIndex >= 0)
    {
        if (Nodes[NodeIndex].Action)
        {
            if (First)
            {
                First = false;
            }
            else
            {
                Stream << " ";
            }

            Stream << Nodes[NodeIndex].Action->GetName();
        }
        NodeIndex = Nodes[NodeIndex].Parent;
    }

    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
