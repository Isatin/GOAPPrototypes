// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows a case of the four arithmetic operations on the same world property.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CStateDefinition Definition;
    const auto& NumberFact = *Definition.DefineNumber("Number", 0.01);
    const auto& AdditionCountFact = *Definition.DefineNumber("AddCount");
    const auto& SubtractCountFact = *Definition.DefineNumber("SubCount");
    const auto& MultiplicationCountFact = *Definition.DefineNumber("MulCount");
    const auto& DivisionCountCountFact = *Definition.DefineNumber("DivCount");

    CState StartingState(Definition);
    StartingState.SetFact(NumberFact, -100);
    StartingState.SetFact(AdditionCountFact, 0);
    StartingState.SetFact(SubtractCountFact, 0);
    StartingState.SetFact(MultiplicationCountFact, 0);
    StartingState.SetFact(DivisionCountCountFact, 0);

    CState GoalState(Definition);
    GoalState.SetFact(NumberFact == 249);

    std::vector<CAction> Actions;
    {
        CAction& Zeroize = Actions.emplace_back("=0", Definition);
        Zeroize.SetEffect(NumberFact = 0);
    }
    {
        CAction& Add100 = Actions.emplace_back("+100", Definition);
        Add100.SetPrecondition(AdditionCountFact == 0);
        Add100.SetEffect(AdditionCountFact += 1);
        Add100.SetEffect(NumberFact += 100);
    }
    {
        CAction& Decrement = Actions.emplace_back("-1", Definition);
        Decrement.SetPrecondition(SubtractCountFact == 0);
        Decrement.SetEffect(SubtractCountFact += 1);
        Decrement.SetEffect(NumberFact -= 1);
    }
    {
        CAction& TenTimes = Actions.emplace_back("*10", Definition);
        TenTimes.SetPrecondition(MultiplicationCountFact == 0);
        TenTimes.SetEffect(MultiplicationCountFact += 1);
        TenTimes.SetEffect(NumberFact *= 10);
    }
    {
        CAction& Quarter = Actions.emplace_back("/4", Definition);
        Quarter.SetPrecondition(DivisionCountCountFact == 0);
        Quarter.SetEffect(DivisionCountCountFact += 1);
        Quarter.SetEffect(NumberFact /= 4);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
