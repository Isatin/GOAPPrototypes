// Copyright 2026 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows constraints using set inclusion and effects using basic set operations: 
// union, intersection, difference, and symmetric difference.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExampleUtility/ExampleUtility.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CFactDefinition Definition;
    auto& X = *Definition.DefineSet("X");
    auto& Y = *Definition.DefineSet("Y");
    auto& Z = *Definition.DefineSet("Z");
    auto& W = *Definition.DefineSet("W");
    auto& S = *Definition.DefineSet("S");
    auto& T = *Definition.DefineSet("T");
    auto& G = *Definition.DefineSet("G");

    CState StartingState(Definition);
    StartingState.SetProperty(X, {1});
    StartingState.SetProperty(Y, {3, 4, 5});
    StartingState.SetProperty(Z, {1, 2, 3, 4});
    StartingState.SetProperty(W, {1, 4});
    StartingState.SetProperty(S, {5});
    StartingState.SetProperty(T, {4, 5});
    StartingState.SetProperty(G, {1, 2, 4});

    G.SetGapWeight(Definition.GetBaseRelationCost() / StartingState.GetUnionSet().GetSize());

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(Z <= G); // The operator <= here denotes ⊆.

    std::vector<CAction> Actions;
    {
        CAction& Action = Actions.emplace_back("X|=Y", Definition);
        Action.SetPrecondition(!(Y > S));   // The operator > here denotes ⊃.
        Action.AddEffect(X |= Y);           // The operator | here denotes ∪.
    }
    {
        CAction& Action = Actions.emplace_back("Y&=Z", Definition);
        Action.SetPrecondition(S <= T);     // The operator <= here denotes ⊆.
        Action.AddEffect(Y &= Z);           // The operator & here denotes ∩.
    }
    {
        CAction& Action = Actions.emplace_back("Z-=W", Definition);
        Action.AddEffect(Z -= W); // The operator - here denotes set difference.
    }
    {
        CAction& Action = Actions.emplace_back("W^=X", Definition);
        Action.AddEffect(W ^= X); // The operator ^ here denotes set symmetric difference.
    }
    {
        CAction& Action = Actions.emplace_back("X=X&Y|Z-W^S", Definition);
        Action.SetPrecondition(S != S && T >= T); // The operator >= here denotes ⊇.
        Action.AddEffect(X = X & Y | Z - W ^ S);
    }

    RunGOAPs(StartingState, GoalCondition, Actions);
    return 0;
}

