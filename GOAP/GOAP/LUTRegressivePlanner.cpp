// Copyright 2024 Isaac Hsu

#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "Action.h"
#include "LUTRegressivePlanner.h"
#include "Node.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CLUTRegressivePlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    std::cout << __FUNCTION__ << std::endl;
    std::cout << "START {" << StartingState.ToString() << "}" << std::endl;
    std::cout << "GOAL  {" << GoalState.ToString() << "}" << std::endl;

    using FactPair = std::pair<std::string, PFact>;
    std::unordered_multimap<FactPair, const CAction*, PairHasher<std::string, PFact>> EffectMap;
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

        bool Unsatisfiable = false;
        FeasibleActions.clear();
        for (const auto& DesiredFact : *CurrNode.ConstState)
        {            
            auto Range = EffectMap.equal_range(DesiredFact);
            if (Range.first == Range.second) // No matched effects
            {
                std::optional<PFact> StartingValue = StartingState.GetFact(DesiredFact.first);
                if (StartingValue != DesiredFact.second)
                {
                    Unsatisfiable = true; // It's impossible to satisfy this fact because neither effects nor the starting value matches it.
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
            Explore(OpenMap, Nodes, CurrIdx, *Action, StartingState);
        }
    }

    return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
