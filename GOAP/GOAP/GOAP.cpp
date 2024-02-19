// Copyright 2024 Isaac Hsu

#include <unordered_set>

#include "Action.h"
#include "ForwardPlanner.h"
#include "LUTRegressivePlanner.h"
#include "RegressivePlanner.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
namespace GOAP
{
    bool CheckInput(const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions)
    {
        std::unordered_set<std::string> UsedFacts;

        for (auto& [Key, Value] : GoalState)
        {
            UsedFacts.insert(Key);
        }

        for (const CAction* Action : Actions)
        {
            for (auto& [Key, Value] : Action->GetPrecondition())
            {
                UsedFacts.insert(Key);
            }

            for (auto& [Key, Value] : Action->GetEffect())
            {
                UsedFacts.insert(Key);
            }
        }

        for (const std::string& Key : UsedFacts)
        {
            if (!StartingState.GetFact(Key)) // Check if all used starting values are set
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

    bool RegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
    {
        if (!CheckInput(StartingState, GoalState, Actions))
        {
            return false;
        }

        CRegressivePlanner Planner;
        return Planner.Plan(oSteps, StartingState, GoalState, Actions, MaxDepth);
    }

    bool LUTRegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth)
    {
        if (!CheckInput(StartingState, GoalState, Actions))
        {
            return false;
        }

        CLUTRegressivePlanner Planner;
        return Planner.Plan(oSteps, StartingState, GoalState, Actions, MaxDepth);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////