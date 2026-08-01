// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is modified from the monkey and banana problem detailed on the following web page:
// https://en.wikipedia.org/wiki/Stanford_Research_Institute_Problem_Solver.
// It shows that the effect lookup tables can improve the efficiency of the regressive search.
// For instance, when compared with the second search, the ensuing branches for the TakeBanana actions 
// at A and C (Ta & Tc) are pruned in the final search tree, because their desired constraints, the
// banana at A and the banana at C, are missing from the tables and not in the starting state.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace EPlace
{
    enum
    {
        A,
        B,
        C,
    };
}
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace ELevel
{
    enum 
    {
        low,
        high,
    };
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    CState GoalState;
    GoalState.SetProperty("HasBanana", true);    

    CState StartingState;
    StartingState.SetProperty("At", EPlace::A);
    StartingState.SetProperty("Level", ELevel::low);
    StartingState.SetProperty("BoxAt", EPlace::C);
    StartingState.SetProperty("BananaAt", EPlace::B);
    StartingState.SetProperty("HasBanana", false);

    std::vector<CAction> Actions;

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        for (int j = EPlace::A; j <= EPlace::C; j++)
        {
            if (i != j)
            {
                std::string Name = "G";
                Name += char('a' + i);
                Name += char('a' + j);
                CAction& Goto = Actions.emplace_back(Name);
                Goto.SetPrecondition("Level", ELevel::low);
                Goto.SetPrecondition("At", i);
                Goto.SetEffect("At", j);
            }
        }
    }

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        for (int j = EPlace::A; j <= EPlace::C; j++)
        {
            if (i != j)
            {
                std::string Name = "M";
                Name += char('a' + i);
                Name += char('a' + j);
                CAction& MoveBox = Actions.emplace_back(Name);
                MoveBox.SetPrecondition("Level", ELevel::low);
                MoveBox.SetPrecondition("At", i);
                MoveBox.SetPrecondition("BoxAt", i);
                MoveBox.SetEffect("At", j);
                MoveBox.SetEffect("BoxAt", j);
            }
        }
    }

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        std::string Name = "U";
        Name += char('a' + i);
        CAction& ClimbUp = Actions.emplace_back(Name);
        ClimbUp.SetPrecondition("At", i);
        ClimbUp.SetPrecondition("BoxAt", i);
        ClimbUp.SetPrecondition("Level", ELevel::low);
        ClimbUp.SetEffect("Level", ELevel::high);
    }

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        std::string Name = "D";
        Name += char('a' + i);
        CAction& ClimbDown = Actions.emplace_back(Name);
        ClimbDown.SetPrecondition("At", i);
        ClimbDown.SetPrecondition("BoxAt", i);
        ClimbDown.SetPrecondition("Level", ELevel::high);
        ClimbDown.SetEffect("Level", ELevel::low);
    }

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        std::string Name = "T";
        Name += char('a' + i);
        CAction& TakeBanana = Actions.emplace_back(Name);
        TakeBanana.SetPrecondition("At", i);
        TakeBanana.SetPrecondition("BananaAt", i);
        TakeBanana.SetPrecondition("Level", ELevel::high);
        TakeBanana.SetEffect("HasBanana", true);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
