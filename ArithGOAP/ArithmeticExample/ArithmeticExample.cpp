// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows four effects corresponding to the basic arithmetic operations.
// Also, it demonstrates that floating-point numbers require tolerance for equality comparisons. 
// If you try to turn off CNumber::IsEquivalenceApproximate, all planners will fail.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    const auto& X = *Definition.DefineNumber("X");
    const auto& AdditionCount       = *Definition.DefineNumber("AddCount");
    const auto& SubtractionCount    = *Definition.DefineNumber("SubCount");
    const auto& MultiplicationCount = *Definition.DefineNumber("MulCount");
    const auto& DivisionCount       = *Definition.DefineNumber("DivCount");

    CState StartingState(Definition);
    StartingState.SetProperty(X, 0.1);
    StartingState.SetProperty(AdditionCount, 0);
    StartingState.SetProperty(SubtractionCount, 0);
    StartingState.SetProperty(MultiplicationCount, 0);
    StartingState.SetProperty(DivisionCount, 0);

    CState GoalState(Definition);
    GoalState.SetProperty(X == 1);

    std::vector<CAction> Actions;
    {
        CAction& Addition = Actions.emplace_back("+0.3", Definition);
        Addition.SetPrecondition(AdditionCount == 0);
        Addition.SetEffect(AdditionCount += 1);
        Addition.SetEffect(X += 0.3);
    }
    {
        CAction& Subtraction = Actions.emplace_back("-0.1", Definition);
        Subtraction.SetPrecondition(SubtractionCount == 0);
        Subtraction.SetPrecondition(X >= 0.2);
        Subtraction.SetEffect(SubtractionCount += 1);
        Subtraction.SetEffect(X -= 0.1);
    }
    {
        CAction& Multiplication = Actions.emplace_back("*100", Definition);
        Multiplication.SetPrecondition(MultiplicationCount == 0);
        Multiplication.SetPrecondition(X <= 0.1);
        Multiplication.SetEffect(MultiplicationCount += 1);
        Multiplication.SetEffect(X *= 100);
    }
    {
        CAction& Division = Actions.emplace_back("/30", Definition);
        Division.SetPrecondition(DivisionCount == 0);
        Division.SetEffect(DivisionCount += 1);
        Division.SetEffect(X /= 30);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
