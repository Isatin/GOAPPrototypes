// Copyright 2024 Isaac Hsu

#include <cassert>
#include <iostream>
#include <sstream>

#include "Action.h"
#include "Fact.h"
#include "LUTRegressivePlanner.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CLUTRegressivePlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    std::cout << __FUNCTION__ << std::endl;
    std::cout << "RANGE {" << StartingState.GetDefinition().StringizeBoundedRanges() << "}" << std::endl;
    std::cout << "START {" << StartingState.ToString() << "}" << std::endl;
    std::cout << "GOAL  {" << GoalState.ToString() << "}" << std::endl;

    CEffectActionMap EffectActionMap;       // Mappings from fact to actions
    CEffectValueMap EffectValueMap;         // Mappings from fact to effect values
    CEffectDirectionMap EffectDirectionMap; // Mappings from fact to effect directions
    BuildEffectMaps(EffectActionMap, EffectValueMap, EffectDirectionMap, Actions);

    std::unordered_set<const CAction*> FeasibleActions;
    FeasibleActions.reserve(Actions.size());

    int Step = 0;
    oSteps.clear();

    std::vector<SNode> Nodes;
    Nodes.reserve(Actions.size() * MaxDepth);

    SNode& RootNode = Nodes.emplace_back();
    RootNode.ConstState = &GoalState;
    RootNode.BasicHeuristicCost = StartingState.GetBasicHeuristicCost(GoalState);
    RootNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(GoalState);

    std::multimap<float, int> OpenMap; // a.k.a open set in A*
    OpenMap.emplace(RootNode.GetTotalCost(), 0);

    while (!OpenMap.empty())
    {
        auto itCurr = OpenMap.begin();
        const int CurrIdx = itCurr->second;
        SNode& CurrNode = Nodes[CurrIdx];

        bool Reached = CurrNode.ConstState->HasIntersection(StartingState);
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

        FeasibleActions.clear();
        if (!FindFeasibleActions(FeasibleActions, *CurrNode.ConstState, StartingState, EffectActionMap, EffectValueMap, EffectDirectionMap))
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

void CLUTRegressivePlanner::BuildEffectMaps(CEffectActionMap& oEffectActionMap, CEffectValueMap& oEffectValueMap, CEffectDirectionMap& oEffectDirectionMap, const std::vector<const CAction*>& Actions)
{
    if (Actions.size() == 0)
    {
        return;
    }

    const int FactAmount = Actions.front()->GetDefinition().GetFactAmount();
    oEffectActionMap.reserve(FactAmount);
    oEffectValueMap.reserve(FactAmount);
    oEffectDirectionMap.reserve(FactAmount);

    for (const CAction* Action : Actions)
    {
        for (auto& Fact : Action->GetDefinition())
        {
            const int Index = Fact->GetIndex();
            const CTransform& Transform = Action->GetEffect().GetTransform(Index);
            if (!Transform) // Check if there's valid transformation for the current fact index
            {
                continue;
            }

            oEffectActionMap.emplace(Index, Action);

            EEffectDirection NewDir = EEffectDirection::none;
            switch (Transform.GetOperator())
            {
            case EOperator::assign:
                NewDir = EEffectDirection::assignment;
                oEffectValueMap[Index].insert(Transform.GetOperand());
                break;

            case EOperator::negation: // Boolean negation can result in true or false.
                NewDir = EEffectDirection::assignment;
                oEffectValueMap[Index].insert(false);
                oEffectValueMap[Index].insert(true);
                break;

            case EOperator::addition:
                if (Transform.GetOperand() > 0)
                {
                    NewDir = EEffectDirection::increment;
                }
                else if (Transform.GetOperand() < 0)
                {
                    NewDir = EEffectDirection::decrement;
                }
                break;

            case EOperator::multiplication:
                if (Transform.GetOperand() == 0)
                {
                    NewDir = EEffectDirection::assignment;
                    oEffectValueMap[Index].insert(0); // Any number times zero equals zero.
                }
                else if (Transform.GetOperand() != 1)
                {
                    // For any multiplicand except 0 & 1, we can always find some multipliers so that the product will increase and some that it will decrease.
                    NewDir = EEffectDirection::increment | EEffectDirection::decrement;
                }
                break;
            }

            oEffectDirectionMap[Index] = oEffectDirectionMap[Index] | NewDir;
        }
    }
}

bool CLUTRegressivePlanner::FindFeasibleActions(std::unordered_set<const CAction*>& oActions, const CState& DesiredState, const CState& StartingState, const CEffectActionMap& EffectActionMap, const CEffectValueMap& EffectValueMap, const CEffectDirectionMap& EffectDirectionMap)
{
    for (auto& Fact : StartingState.GetDefinition())
    {
        const int Index = Fact->GetIndex();
        const SInterval& Aim = DesiredState.GetFact(Index);
        if (!Aim) // Check if this fact is desired
        {
            continue;
        }

        const SInterval& StartingValue = StartingState.GetFact(Index);
        if (!Aim.HasIntersection(StartingValue)) // Check if the desire value mismatches the starting value
        {
            auto itDir = EffectDirectionMap.find(Index);
            if (itDir == EffectDirectionMap.end() || itDir->second == EEffectDirection::none) // No matching or only ineffective effects
            {
                return false; // It's impossible to satisfy this fact because neither effects nor the starting value matches it.
            }

            if (itDir->second == EEffectDirection::assignment) // When all effects are assignment-like only
            {
                // Check if any effect values can satisfy the desired value
                bool Satisfied = false;
                auto itValues = EffectValueMap.find(Index);
                if (itValues != EffectValueMap.end())
                {
                    for (const auto& Value : itValues->second)
                    {
                        if (Aim.Contain(Value))
                        {
                            Satisfied = true;
                            break;
                        }
                    }
                }

                if (!Satisfied)
                {
                    return false; // It's impossible to satisfy this fact because neither effects nor the starting value matches it.
                }
            }        
            else if (itDir->second == EEffectDirection::increment) // When all effects are incremental only
            {
                // In regressive search, we're looking for successive inverse effects to reach the starting state from the goal state.
                // So the starting state is the goal and the effect direction should be considered in reverse.
                // When the desired value is less than the starting value but the reverse effect direction is decremental only,
                // it's impossible for this fact to reach the starting value.
                if (Aim < StartingValue) 
                {
                    return false;
                }
            }
            else if (itDir->second == EEffectDirection::decrement) // When all effects are decremental only
            {
                if (Aim > StartingValue)
                {
                    return false;
                }
            }
        }

        auto Range = EffectActionMap.equal_range(Index);
        for (auto it = Range.first; it != Range.second; ++it)
        {
            oActions.insert(it->second);
        }
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
