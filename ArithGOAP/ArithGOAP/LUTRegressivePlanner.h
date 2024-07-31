// Copyright 2024 Isaac Hsu

#pragma once

#include <set>
#include <unordered_map>
#include <unordered_set>

#include "RegressivePlanner.h"


namespace ArithGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EEffectDirection // For a fact, whether the effects are incremental/decremental/assigned
    {
    public:
        enum Type
        {
            none        = 0,
            increment   = 1 << 0,
            decrement   = 1 << 1,
            assignment  = 1 << 2,
        };

        EEffectDirection() = default;
        EEffectDirection(int Value) : mValue(static_cast<Type>(Value)) {}

        operator Type() const { return mValue; }

    private:
        Type mValue = none;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CLUTRegressivePlanner : public CRegressivePlanner // Regressive GOAP with effect lookup tables
    {
        using CEffectActionMap = std::unordered_multimap<int, const CAction*>;
        using CEffectValueMap = std::unordered_map<int, std::set<SNumber>>;
        using CEffectDirectionMap = std::unordered_map<int, EEffectDirection>;
    public:
        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);

    protected:
        // Build effect lookup tables
        void BuildEffectMaps(CEffectActionMap& oEffectActionMap, CEffectValueMap& oEffectValueMap, CEffectDirectionMap& oEffectDirectionMap, const std::vector<const CAction*>& Actions);
        // Look up feasible actions in effect lookup tables
        bool FindFeasibleActions(std::unordered_set<const CAction*>& oActions, const CState& DesiredState, const CState& StartingState, const CEffectActionMap& EffectActionMap, const CEffectValueMap& EffectValueMap, const CEffectDirectionMap& EffectDirectionMap);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
