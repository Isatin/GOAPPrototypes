// Copyright 2026 Isaac Hsu

#pragma once

#include <string>
#include <vector>

#include "Number.h"


namespace ExtendedGOAP
{
    class CFormula;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SDebugHeuristicTuple // Used to keep track of the heuristic maximum
    {
        int NodeIndex = InvalidIndex;
        CNumber MaxHeuristic = CNumber::Null;
        CNumber GapWeightOfMaxHeuristic = CNumber::Null;        
        std::string Constraint;
        std::string State;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SDebugInfo // Used to store debug information during search
    {
        int CurrentNodeIndex = InvalidIndex;                // Temporary index indicating the current node during search
        std::vector<SDebugHeuristicTuple> HeuristicTuples;  // List of heuristic information for each fact

    public:
        void Clear();
        void UpdateHeuristicValue(const std::pair<CNumber, CNumber>& ValueAndWeight, const CFormula& Constraint, const CState& State);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}