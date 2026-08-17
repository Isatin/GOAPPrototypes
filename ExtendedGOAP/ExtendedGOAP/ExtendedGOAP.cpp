// Copyright 2026 Isaac Hsu

#include "Action.h"
#include "AdvRegressionPlanner.h"
#include "BackwardPlanner.h"
#include "ExtendedGOAP.h"
#include "Fact.h"
#include "ForwardPlanner.h"
#include "State.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
namespace ExtendedGOAP
{
    int GetMaxPreconditionRelations(const std::vector<CAction>& Actions)
    {
        int Max = 0;

        for (const CAction& Action : Actions)
        {
            Max = std::max(Max, Action.GetPrecondition().GetConstraint().CountRelations());
        }

        return Max;
    }

    int GetMaxPreconditionRelations(const std::vector<const CAction*>& Actions)
    {
        int Max = 0;

        for (const CAction* Action : Actions)
        {
            Max = std::max(Max, Action->GetPrecondition().GetConstraint().CountRelations());
        }

        return Max;
    }

    bool CheckInput(const CState& StartingState, const CCondition& GoalCondition, const std::vector<const CAction*>& Actions)
    {
        if (&GoalCondition.GetDefinition() != &StartingState.GetDefinition())
        {
            return false;
        }

        CBitVector UsedFactBits = GoalCondition.GetUsedFactBits();

        for (const CAction* Action : Actions)
        {
            if (&Action->GetDefinition() != &StartingState.GetDefinition())
            {
                return false;
            }

            UsedFactBits |= Action->GetUsedFactBits();
        }

        for (int FactIndex = 0; FactIndex < UsedFactBits.GetSize(); FactIndex++)
        {
            if (UsedFactBits[FactIndex])
            {
                const CVariant& Variant = StartingState.GetProperty(FactIndex);
                if (Variant.IsUnset()) // Check if all used starting values have been set.
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool ForwardSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition,
        const std::vector<const CAction*>& Actions, int MaxDepth, unsigned Flags)
    {
        if (!CheckInput(StartingState, GoalCondition, Actions))
        {
            return false;
        }

        CForwardPlanner Planner(Flags);
        return Planner.Plan(oSteps, StartingState, GoalCondition, Actions, MaxDepth);
    }

    bool BackwardSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition,
        const std::vector<const CAction*>& Actions, int MaxDepth, unsigned Flags)
    {
        if (!CheckInput(StartingState, GoalCondition, Actions))
        {
            return false;
        }

        CBackwardPlanner Planner(Flags);
        return Planner.Plan(oSteps, StartingState, GoalCondition, Actions, MaxDepth);
    }

    bool RegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition,
        const std::vector<const CAction*>& Actions, int MaxDepth, unsigned Flags)
    {
        if (!CheckInput(StartingState, GoalCondition, Actions))
        {
            return false;
        }

        CRegressionPlanner Planner(Flags);
        return Planner.Plan(oSteps, StartingState, GoalCondition, Actions, MaxDepth);
    }

    bool AdvRegressiveSearch(std::vector<const CAction*>& oSteps, const CState& StartingState, const CCondition& GoalCondition,
        const std::vector<const CAction*>& Actions, int MaxDepth, unsigned Flags)
    {
        if (!CheckInput(StartingState, GoalCondition, Actions))
        {
            return false;
        }

        CAdvRegressionPlanner Planner(Flags);
        return Planner.Plan(oSteps, StartingState, GoalCondition, Actions, MaxDepth);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////