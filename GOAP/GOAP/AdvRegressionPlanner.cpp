// Copyright 2024 Isaac Hsu

#include <iostream>
#include <map>
#include <typeinfo>
#include <unordered_set>

#include "Action.h"
#include "AdvRegressionPlanner.h"
#include "Hash.h"
#include "Node.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CAdvRegressionPlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    std::cout << typeid(*this).name() << std::endl;
    std::cout << "START: {" << StartingState.ToString() << "}" << std::endl;
    std::cout << "GOAL : {" << GoalState.ToString() << "}" << std::endl;

    using FactPair = std::pair<std::string, BProperty>;
    std::unordered_multimap<FactPair, const CAction*> EffectMap;
    for (const CAction* Action : Actions)
    {
        for (const auto& Pair : Action->GetEffect())
        {
            EffectMap.emplace(Pair, Action);
        }
    }

    std::unordered_set<const CAction*> FeasibleActions;
    FeasibleActions.reserve(Actions.size());

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

        bool Unsatisfiable = false;
        FeasibleActions.clear();
        for (const auto& DesiredFact : *CurrNode.ConstState)
        {            
            auto Range = EffectMap.equal_range(DesiredFact);
            if (Range.first == Range.second) // Check if no effects match the desired property.
            {
                std::optional<BProperty> StartingValue = StartingState.GetProperty(DesiredFact.first);
                if (StartingValue != DesiredFact.second)
                {
                    Unsatisfiable = true; // It's impossible to satisfy the desired property because neither the starting value nor the effects match it.
                    break;
                }
            }

            for (auto it = Range.first; it != Range.second; ++it)
            {
                FeasibleActions.insert(it->second);
            }
        } 

        if (Unsatisfiable)
        {
            continue;
        }

        for (const CAction* Action : FeasibleActions)
        {
            Explore(OpenMap, Nodes, CurrIndex, *Action, StartingState);
        }
    }

    return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
