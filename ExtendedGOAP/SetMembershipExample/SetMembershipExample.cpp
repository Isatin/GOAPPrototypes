// Copyright 2026 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows set membership operations in goal constraints.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CSet SetX = {1, 2, 3};
    CSet SetY = ~CSet{4, 5, 6};
    CSet SetZ = ~CSet{4, 8, 16};

    CFactDefinition Definition;
    auto& A = *Definition.DefineNumber("A");
    auto& B = *Definition.DefineNumber("B");
    auto& C = *Definition.DefineNumber("C");
    auto& X = *Definition.DefineSet("X", 1.f / SetX.GetSize());
    auto& Y = *Definition.DefineSet("Y", 1.f / SetY.GetSize());
    auto& Z = *Definition.DefineSet("Z", 1.f / SetZ.GetSize());

    CState StartingState(Definition);
    StartingState.SetProperty(A, 6);
    StartingState.SetProperty(B, 0);
    StartingState.SetProperty(C, 8);
    StartingState.SetProperty(X, SetX);
    StartingState.SetProperty(Y, SetY);
    StartingState.SetProperty(Z, SetZ);

    CCondition GoalCondition(Definition);
    // Here the operator >> denotes ∋, and the operator << denotes ∈.
    GoalCondition.SetConstraint((X >> A || !(B << Y)) && C << Z);

    std::vector<CAction> Actions;
    {
        CAction& Action = Actions.emplace_back("A--", Definition);
        Action.AddEffect(A -= 1);
    }
    {
        CAction& Action = Actions.emplace_back("B+=2", Definition);
        Action.AddEffect(B += 2);
    }
    {
        CAction& Action = Actions.emplace_back("C*=2", Definition);
        Action.AddEffect(C *= 2);
    }

    RunGOAPs(StartingState, GoalCondition, Actions);
    return 0;
}

