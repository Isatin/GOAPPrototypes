// Copyright 2024 Isaac Hsu

#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <typeinfo>
#include <unordered_set>

#include "Action.h"
#include "Node.h"
#include "BackwardPlanner.h"
#include "Hash.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CBackwardPlanner::SNode::IsSatisfied() const
{
    return DesiredState->CountUnsatisfiedProperties(*CurrentState) == 0;
}

std::string CBackwardPlanner::SNode::ToString() const
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

    Stream << " Depth=" << Depth << " {";
    if (DesiredState)
    {
        if (CurrentState)
        {
            Stream << CurrentState->Stringize(*DesiredState);
        }
        else
        {
            Stream << DesiredState->ToString();
        }
    }
    Stream << "}}";

    return Stream.str();
}

bool CBackwardPlanner::SNode::CheckPostcondition(const CAction& Action) const
{
    for (auto& [Name, Constraint] : Action.GetPrecondition())
    {
        std::optional<BProperty> Current = CurrentState->GetProperty(Name);
        std::optional<BProperty> Target = DesiredState->GetProperty(Name);
        if (Target == Current)
        {
            continue; // Skip satisfied properties.
        }

        if (Target != Constraint)
        {
            std::optional<BProperty> Effect = Action.GetEffect().GetProperty(Name);
            if (Target != Effect)
            {
                // This action is infeasible, because its precondition conflicts with a desired property and its effect can't satisfy that property.
                return false;
            }
        }
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CBackwardPlanner::Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
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

    std::unordered_set<const CAction*> CandidateActions;
    CandidateActions.reserve(Actions.size());

    int Step = 0;
    oSteps.clear();
    MaxDepth = std::max(MaxDepth, 0);

    std::vector<SNode> Nodes;
    Nodes.reserve(Actions.size() * MaxDepth);

    SNode& RootNode = Nodes.emplace_back();
    RootNode.DesiredState = GoalState.Clone();
    RootNode.CurrentState = std::make_unique<CState>();
    RootNode.CurrentState->CopyProperties(StartingState, GoalState);
    RootNode.BaseHeuristicCost = static_cast<float>(GoalState.CountUnsatisfiedProperties(StartingState));
    RootNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(GoalState);

    std::multimap<float, int> OpenMap; // The open set in A*
    OpenMap.emplace(RootNode.GetTotalCost(), 0);

    while (!OpenMap.empty())
    {
        auto itCurr = OpenMap.begin();
        const int CurrIndex = itCurr->second;
        SNode& CurrNode = Nodes[CurrIndex];

        bool Reached = CurrNode.IsSatisfied();
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
        for (const auto& DesiredFact : *CurrNode.DesiredState)
        {
            std::optional<BProperty> Value = CurrNode.CurrentState->GetProperty(DesiredFact.first);
            assert(Value.has_value());
            if (Value == DesiredFact.second)
            {
                continue; // Skip satisfied properties.
            }

            auto Range = EffectMap.equal_range(DesiredFact);
            for (auto it = Range.first; it != Range.second; ++it)
            {
                CandidateActions.insert(it->second);
            }
        }

        for (const CAction* Action : CandidateActions)
        {
            Explore(OpenMap, Nodes, CurrIndex, *Action, StartingState);
        }
    }

    return false;
}

void CBackwardPlanner::Explore(std::multimap<float, int>& oOpenMap, std::vector<SNode>& Nodes, int NodeIndex, const CAction& Action, const CState& StartingState)
{
    if (!Nodes[NodeIndex].CheckPostcondition(Action))
    {
        return;
    }

    int ChildIndex = static_cast<int>(Nodes.size());
    SNode& ChildNode = Nodes.emplace_back();
    SNode& CurrNode = Nodes[NodeIndex];
    ChildNode.Action = &Action;
    ChildNode.CurrentState = CurrNode.CurrentState->Clone();
    ChildNode.CurrentState->CopyProperties(*CurrNode.DesiredState, Action.GetEffect());     // Copy matching values into the current state.
    ChildNode.DesiredState = CurrNode.DesiredState->Clone();
    Action.GetPrecondition().Overwrite(*ChildNode.DesiredState);                            // Add the preconditions to the desired state as new constraints.
    ChildNode.CurrentState->InitializeProperties(StartingState, *ChildNode.DesiredState);   // Copy starting values into the current state for unset properties.
    Action.Affect(*ChildNode.DesiredState);
    ChildNode.Parent = NodeIndex;
    ChildNode.Depth = CurrNode.Depth + 1;
    ChildNode.PreviousCost = CurrNode.GetActualCost();
    ChildNode.CurrentCost = Action.GetCost(*CurrNode.DesiredState, *ChildNode.DesiredState);
    ChildNode.BaseHeuristicCost = static_cast<float>(ChildNode.DesiredState->CountUnsatisfiedProperties(*ChildNode.CurrentState));
    ChildNode.ExtraHeuristicCost = StartingState.GetExtraHeuristicCost(*ChildNode.DesiredState);
    float TotalCost = ChildNode.GetTotalCost();
    oOpenMap.emplace(TotalCost, ChildIndex);
}

void CBackwardPlanner::BuildPlan(std::vector<const CAction*>& oSteps, const std::vector<SNode>& Nodes, int NodeIndex)
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

std::string CBackwardPlanner::StringizePath(const std::vector<SNode>& Nodes, int NodeIndex) const
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
