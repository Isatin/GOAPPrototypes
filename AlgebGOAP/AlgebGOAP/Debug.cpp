// Copyright 2025 Isaac Hsu

#include "BitVector.h"
#include "Debug.h"
#include "Formula.h"
#include "State.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
void SDebugInfo::Clear()
{
    CurrentNodeIndex = InvalidIndex;
    HeuristicTuples.clear();
}

void SDebugInfo::UpdateHeuristicValue(const std::pair<CNumber, CNumber>& ValueAndWeight, const CFormula& Constraint, const CState& State)
{
    CBitVector FactBits = Constraint.GetUsedFactBits();

    if (HeuristicTuples.size() < FactBits.GetSize())
    {
        HeuristicTuples.resize(FactBits.GetSize());
    }

    for (int FactIndex = 0; FactIndex < FactBits.GetSize(); FactIndex++)
    {
        if (!FactBits[FactIndex])
        {
            continue;
        }

        SDebugHeuristicTuple& Tuple = HeuristicTuples[FactIndex];
        if (!Tuple.MaxHeuristic.IsNull() && Tuple.MaxHeuristic.Get() >= ValueAndWeight.first.Get())
        {
            continue;
        }
         
        Tuple.NodeIndex = CurrentNodeIndex;
        Tuple.MaxHeuristic = ValueAndWeight.first;
        Tuple.GapWeightOfMaxHeuristic = ValueAndWeight.second;
        Tuple.Constraint = Constraint.ToString(State.GetDefinition());
        Tuple.State = State.ToString();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////