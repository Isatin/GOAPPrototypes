// Copyright 2024 Isaac Hsu

#include <unordered_set>

#include "Action.h"
#include "AdvRegressionPlanner.h"
#include "ArithGOAP.h"
#include "ForwardPlanner.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
namespace ArithGOAP
{
    bool CheckInput(const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions)
    {
        if (&GoalState.GetDefinition() != &StartingState.GetDefinition())
        {
            return false;
        }

        std::unordered_set<int> UsedFactIndexes;
        UsedFactIndexes.reserve(GoalState.GetPropertyCapacity());

        for (int FactIndex = 0; FactIndex < GoalState.GetPropertyCapacity(); FactIndex++)
        {
            if (GoalState.GetProperty(FactIndex).IsSet())
            {
                UsedFactIndexes.insert(FactIndex);
            }
        }

        for (const CAction* Action : Actions)
        {
            if (&Action->GetDefinition() != &StartingState.GetDefinition())
            {
                return false;
            }

            const auto& Precondition = Action->GetPrecondition();
            for (int FactIndex = 0; FactIndex < Precondition.GetPropertyCapacity(); FactIndex++)
            {
                if (Precondition.GetProperty(FactIndex).IsSet())
                {
                    UsedFactIndexes.insert(FactIndex);
                }
            }

            const auto& Effect = Action->GetEffect();
            for (int FactIndex = 0; FactIndex < Effect.GetTransformCapacity(); FactIndex++)
            {
                if (!Effect.GetTransform(FactIndex).IsNil())
                {
                    UsedFactIndexes.insert(FactIndex);
                }
            }
        }

        for (int FactIndex : UsedFactIndexes)
        {
            if (StartingState.GetProperty(FactIndex).IsUnset()) // Check if all used starting values have been set.
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