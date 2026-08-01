// Copyright 2024 Isaac Hsu

#include <unordered_set>

#include "Action.h"
#include "AdvRegressionPlanner.h"
#include "BackwardPlanner.h"
#include "ForwardPlanner.h"
#include "RegressionPlanner.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
namespace GOAP
{
    bool CheckInput(const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions)
    {
        std::unordered_set<std::string> UsedFacts;
        UsedFacts.reserve(GoalState.GetPropertyCount());

        for (auto& [Name, Value] : GoalState)
        {
            UsedFacts.insert(Name);
        }

        for (const CAction* Action : Actions)
        {
            for (auto& [Name, Value] : Action->GetPrecondition())
            {
                UsedFacts.insert(Name);
            }

            for (auto& [Name, Value] : Action->GetEffect())
            {
                UsedFacts.insert(Name);
            }
        }

        for (const std::string& Name : UsedFacts)
        {
            if (!StartingState.GetProperty(Name)) // Check if all used starting values have been set.
            {
                return false;
            }
        }

        return true;
    }

    bool ForwardSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
    {
        if (!CheckInput(StartingState, GoalState, Actions))
        {
            return false;
        }

        CForwardPlanner Planner;
        return Planner.Plan(oSteps, StartingState, GoalState, Actions, MaxDepth);
    }

    bool BackwardSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
    {
        if (!CheckInput(StartingState, GoalState, Actions))
        {
            return false;
        }

        CBackwardPlanner Planner;
        return Planner.Plan(oSteps, StartingState, GoalState, Actions, MaxDepth);
    }

    bool RegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
    {
        if (!CheckInput(StartingState, GoalState, Actions))
        {
            return false;
        }

        CRegressionPlanner Planner;
        return Planner.Plan(oSteps, StartingState, GoalState, Actions, MaxDepth);
    }

    bool AdvRegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
    {
        if (!CheckInput(StartingState, GoalState, Actions))
        {
            return false;
        }

        CAdvRegressionPlanner Planner;
        return Planner.Plan(oSteps, StartingState, GoalState, Actions, MaxDepth);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////