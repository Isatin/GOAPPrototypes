// Copyright 2024 Isaac Hsu

#include <unordered_set>

#include "Action.h"
#include "ArithGOAP.h"
#include "ForwardPlanner.h"
#include "LUTRegressivePlanner.h"
#include "RegressivePlanner.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
namespace ArithGOAP
{
    bool CheckInput(const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions)
    {
        if (&GoalState.GetDefinition() != &StartingState.GetDefinition())
        {
            return false;
        }

        std::unordered_set<int> UsedFactIndices;

        for (int i = 0; i < GoalState.GetFactAmount(); i++)
        {
            if (GoalState.GetFact(i))
            {
                UsedFactIndices.insert(i);
            }
        }

        for (const CAction* Action : Actions)
        {
            if (&Action->GetDefinition() != &StartingState.GetDefinition())
            {
                return false;
            }

            const auto& Precondition = Action->GetPrecondition();
            for (int i = 0; i < Precondition.GetFactAmount(); i++)
            {
                if (Precondition.GetFact(i))
                {
                    UsedFactIndices.insert(i);
                }
            }

            const auto& Effect = Action->GetEffect();
            for (int i = 0; i < Effect.GetTransformAmount(); i++)
            {
                if (Effect.GetTransform(i))
                {
                    UsedFactIndices.insert(i);
                }
            }
        }

        for (int Index : UsedFactIndices)
        {
            if (!StartingState.GetFact(Index)) // Check if all used starting values are set
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