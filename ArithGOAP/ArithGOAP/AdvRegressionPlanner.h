// Copyright 2024 Isaac Hsu

#pragma once

#include <set>
#include <unordered_map>
#include <unordered_set>

#include "RegressionPlanner.h"


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EEffectDirection // Mock enum of effect directions to indicate whether the effects on a fact are incremental/decremental/substitutional
    {
    public:
        enum Type
        {
            none        = 0,
            increment   = 1 << 0,
            decrement   = 1 << 1,
            assignment  = 1 << 2,
        };

    public:
        EEffectDirection() = default;
        EEffectDirection(int Value) : mValue(static_cast<Type>(Value)) {}

        operator Type() const { return mValue; }

    private:
        Type mValue = none;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CAdvRegressionPlanner : public CRegressionPlanner // Regressive arithmetic GOAP with effect lookup tables
    {
        using CFactActionMap        = std::unordered_multimap<int, const CAction*>;
        using CConstantEffectMap    = std::unordered_map<int, std::set<CNumber>>;
        using CEffectDirectionMap   = std::unordered_map<int, EEffectDirection>;
    public:
        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);

    protected:
        // Build effect lookup tables.
        void BuildEffectMaps(CFactActionMap& oFactActionMap, CConstantEffectMap& oConstantEffectMap, CEffectDirectionMap& oEffectDirectionMap, const std::vector<const CAction*>& Actions);
        // Look up feasible actions in given lookup tables.
        bool FindFeasibleActions(std::unordered_set<const CAction*>& oActions, const CState& DesiredState, const CState& StartingState, const CFactActionMap& EffectActionMap, const CConstantEffectMap& EffectValueMap, const CEffectDirectionMap& EffectDirectionMap);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
