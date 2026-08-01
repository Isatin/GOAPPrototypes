// Copyright 2024 Isaac Hsu

#include <cassert>
#include <iostream>
#include <typeinfo>

#include "Action.h"
#include "AdvRegressionPlanner.h"
#include "Fact.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CAdvRegressionPlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
{
    std::cout << typeid(*this).name() << std::endl;
    if (StartingState.GetDefinition().HasAnyRange())
    {
        std::cout << "RANGE: {" << StartingState.GetDefinition().StringizeBoundedRanges() << "}" << std::endl;
    }
    std::cout << "START: {" << StartingState.ToString() << "}" << std::endl;
    std::cout << "GOAL : {" << GoalState.ToString() << "}" << std::endl;

    CFactActionMap FactActionMap;           // Mapping of facts to actions
    CConstantEffectMap ConstantEffectMap;   // Mapping of facts to effect values
    CEffectDirectionMap EffectDirectionMap; // Mapping of facts to effect directions
    BuildEffectMaps(FactActionMap, ConstantEffectMap, EffectDirectionMap, Actions);

    std::unordered_set<const CAction*> CandidateActions;
    CandidateActions.reserve(Actions.size());

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

        CandidateActions.clear();
        if (!FindFeasibleActions(CandidateActions, *CurrNode.ConstState, StartingState, FactActionMap, ConstantEffectMap, EffectDirectionMap))
        {
            continue;
        }

        for (const CAction* Action : CandidateActions)
        {
            Explore(OpenMap, Nodes, CurrIndex, *Action, StartingState);
        }
    }

    return false;
}

void CAdvRegressionPlanner::BuildEffectMaps(CFactActionMap& oFactActionMap, CConstantEffectMap& oConstantEffectMap, CEffectDirectionMap& oEffectDirectionMap, const std::vector<const CAction*>& Actions)
{
    if (Actions.size() == 0)
    {
        return;
    }

    const int FactCount = Actions.front()->GetDefinition().GetFactCount();
    oFactActionMap.reserve(FactCount);
    oConstantEffectMap.reserve(FactCount);
    oEffectDirectionMap.reserve(FactCount);

    for (const CAction* Action : Actions)
    {
        for (auto& Fact : Action->GetDefinition())
        {
            const int FactIndex = Fact->GetIndex();
            const CTransform& Transform = Action->GetEffect().GetTransform(FactIndex);
            if (Transform.IsNil())
            {
                continue; // Skip unset effects.
            }

            oFactActionMap.emplace(FactIndex, Action);

            EEffectDirection NewDir = EEffectDirection::none;
            switch (Transform.GetOperator())
            {
            case EOperator::assignment:
                NewDir = EEffectDirection::assignment;
                oConstantEffectMap[FactIndex].insert(Transform.GetOperand());
                break;

            case EOperator::negation: 
                // Boolean negation evaluates to true or false.
                NewDir = EEffectDirection::assignment;
                oConstantEffectMap[FactIndex].insert(false);
                oConstantEffectMap[FactIndex].insert(true);
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
                    oConstantEffectMap[FactIndex].insert(0); // Any number times zero equals zero.
                }
                else if (Transform.GetOperand() != 1)
                {
                    // For any multiplicand except 0 and 1, we can always find multipliers that increase the product and others that decrease it.
                    NewDir = EEffectDirection::increment | EEffectDirection::decrement;
                }
                break;
            }

            oEffectDirectionMap[FactIndex] = oEffectDirectionMap[FactIndex] | NewDir;
        }
    }
}

bool CAdvRegressionPlanner::FindFeasibleActions(std::unordered_set<const CAction*>& oActions, const CState& DesiredState, const CState& StartingState, const CFactActionMap& EffectActionMap, const CConstantEffectMap& EffectValueMap, const CEffectDirectionMap& EffectDirectionMap)
{
    const CNumber Tolerance = StartingState.GetDefinition().GetTolerance();

    for (auto& Fact : StartingState.GetDefinition())
    {
        const int FactIndex = Fact->GetIndex();
        const SSegment& Target = DesiredState.GetProperty(FactIndex);
        if (Target.IsUnset()) 
        {
            continue; // Skip unset properties.
        }

        const SSegment& StartingValue = StartingState.GetProperty(FactIndex);
        if (!Target.HasIntersection(StartingValue, Tolerance)) // Check if the desired property does not match its starting value.
        {
            auto itDir = EffectDirectionMap.find(FactIndex);
            if (itDir == EffectDirectionMap.end() || itDir->second == EEffectDirection::none) // Check if there are no matches or only neutral effects.
            {
                return false; // It's impossible to satisfy this property because the effects and the starting value don't match it.
            }

            if (itDir->second == EEffectDirection::assignment) // Check if all effects are assignments.
            {
                // Check if any effect can satisfy the desired value.
                bool Satisfied = false;
                auto itValues = EffectValueMap.find(FactIndex);
                if (itValues != EffectValueMap.end())
                {
                    for (const auto& Value : itValues->second)
                    {
                        if (Target.Contain(Value, Tolerance))
                        {
                            Satisfied = true;
                            break;
                        }
                    }
                }

                if (!Satisfied)
                {
                    return false;
                }
            }        
            else if (itDir->second == EEffectDirection::increment) // Check if all effects are incremental.
            {
                // In the regressive search, we apply inverse effects to search backwards from the goal state to the starting state.
                // So, the effect directions should be considered in reverse.
                // If the desired value is less than the starting value, but the reverse effect direction is decremental only,
                // the property cannot reach its starting value.
                if (Target < StartingValue) 
                {
                    return false;
                }
            }
            else if (itDir->second == EEffectDirection::decrement) // Check if all effects are decremental.
            {
                if (Target > StartingValue)
                {
                    return false;
                }
            }
        }

        auto Range = EffectActionMap.equal_range(FactIndex);
        for (auto it = Range.first; it != Range.second; ++it)
        {
            oActions.insert(it->second);
        }
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
