// Copyright 2024 Isaac Hsu

#include <sstream>

#include "GOAP.h"
#include "Node.h"
#include "State.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string SNode::ToString() const
{
    std::stringstream Stream;
    Stream << "{Cost=" << GetTotalCost() << "=(" << CurrentCost << "+" << PreviousCost << ")+";

    if (ExtraHeuristicCost == 0.f)
    {
        Stream << BasicHeuristicCost;
    }
    else
    {
        Stream << "(" << BasicHeuristicCost << "+" << ExtraHeuristicCost << ")";
    }
    
    Stream << " Depth=" << Depth << " {" << (ConstState ? ConstState->ToString() : "") << "}}";
    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////