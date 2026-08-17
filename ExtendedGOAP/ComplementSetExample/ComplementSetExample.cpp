// Copyright 2026 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows two cases using set complement.
// In the first case, the universal set includes all ordered floating-point numbers.
// In the second case, the universal set includes only the numbers listed in the starting state, and
// the complements are converted to the differences between the universal set and the original sets.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

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
    StartingState.SetProperty(G, ~CSet{1, 2, 4}); // The operator ~ here denotes set complement.

    CCondition GoalCondition(Definition);
    GoalCondition.SetConstraint(Z == G);

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
        CAction& Action = Actions.emplace_back("Z=~Z", Definition);
        Action.AddEffect(Z = ~Z); // The operator ~ here denotes set complement.
    }

    {
        std::cout << "    THE UNIVERSAL SET INCLUDES ALL " << sizeof(CNumber::BValue)*8 << "-BIT FLOATING-POINT NUMBERS.\n";
        CSet UniversalSet = StartingState.GetUnionSet();
        G.SetGapWeight(Definition.GetBaseRelationCost() / UniversalSet.GetSize());

        RunGOAPs(StartingState, GoalCondition, Actions, Actions.size());
    }

    {
        CSet UniversalSet = StartingState.GetUnionSetIC();
        std::cout << "    THE UNIVERSAL SET INCLUDES ONLY the LISTED NUMBERS: " << UniversalSet.ToString() << ".\n";
        G.SetGapWeight(Definition.GetBaseRelationCost() / UniversalSet.GetSize());

        auto& U = *Definition.DefineSet("U");
        StartingState.SetProperty(U, UniversalSet);
        StartingState.ConvertComplementSets(UniversalSet);
       
        // Change the last action from set complement to set difference.
        {
            CAction& Action = Actions.back();
            Action.SetName("Z=U-Z");
            Action.ClearEffect();
            Action.AddEffect(Z = U - Z); // Z' = U∖Z
        }

        RunGOAPs(StartingState, GoalCondition, Actions, Actions.size());
    }

    return 0;
}

