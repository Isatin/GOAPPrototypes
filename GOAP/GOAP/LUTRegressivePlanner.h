// Copyright 2024 Isaac Hsu

#pragma once

#include <vector>

#include "RegressivePlanner.h"


namespace GOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CLUTRegressivePlanner : public CRegressivePlanner // Regressive GOAP with effect lookup tables
    {
        using Super = CRegressivePlanner;
    public:
        // Formulate a plan according to the input if possible. Otherwise, return false.
        bool Plan(std::vector<const CAction*>& oSteps, const CState& StartingState, const CState& GoalState, const std::vector<const CAction*>& Actions, int MaxDepth);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////    
    template <typename TKey, typename TValue> // Hash functor for std::pair
    struct PairHasher
    {
        size_t operator() (const std::pair<TKey, TValue>& Pair) const
        {
            size_t Hash1 = std::hash<TKey>{}(Pair.first);
            size_t Hash2 = std::hash<TValue>{}(Pair.second);
            return Hash1 ^ (Hash2 + 0x9e3779b9 + (Hash1 << 6) + (Hash1 >> 2)); // from boost::hash_combine
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
