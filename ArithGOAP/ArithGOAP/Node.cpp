// Copyright 2024 Isaac Hsu

#include <sstream>

#include "Node.h"
#include "State.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string SNode::ToString() const
{
    std::stringstream Stream;
    Stream << "{Cost=" << GetTotalCost() << "=(" << PreviousCost << "+" << CurrentCost << ")+";

    if (ExtraHeuristicCost == 0.f)
    {
        Stream << BaseHeuristicCost;
    }
    else
    {
        Stream << "(" << BaseHeuristicCost << "+" << ExtraHeuristicCost << ")";
    }

    Stream << " Depth=" << Depth << " {" << (ConstState ? ConstState->ToString() : "") << "}}";
    return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////