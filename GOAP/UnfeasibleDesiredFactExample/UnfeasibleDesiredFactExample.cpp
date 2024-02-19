// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is the monkey and banana problem from 
// https://en.wikipedia.org/wiki/Stanford_Research_Institute_Problem_Solver.
// It shows with effect lookup tables the regressive algorithm can be more performant.
// As proof, compared with the second search, the regression from actions of TakeBanana at A and C 
// (Ta & Tc) is discontinued in the third search with lookup tables.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>

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
    GoalState.SetFact("HasBanana", true);    

    CState StartingState;
    StartingState.SetFact("At", EPlace::A);
    StartingState.SetFact("Level", ELevel::low);
    StartingState.SetFact("BoxAt", EPlace::C);
    StartingState.SetFact("BananaAt", EPlace::B);
    StartingState.SetFact("HasBanana", false);

    std::vector<CAction> Actions;

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        for (int j = EPlace::A; j <= EPlace::C; j++)
        {
            if (i != j)
            {
                std::stringstream Stream;
                Stream << "G" << char('a' + i) << char('a' + j);
                CAction& Goto = Actions.emplace_back(Stream.str());
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
                std::stringstream Stream;
                Stream << "M" << char('a' + i) << char('a' + j);
                CAction& MoveBox = Actions.emplace_back(Stream.str());
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
        std::stringstream Stream;
        Stream << "U" << char('a' + i);
        CAction& ClimbUp = Actions.emplace_back(Stream.str());
        ClimbUp.SetPrecondition("At", i);
        ClimbUp.SetPrecondition("BoxAt", i);
        ClimbUp.SetPrecondition("Level", ELevel::low);
        ClimbUp.SetEffect("Level", ELevel::high);
    }

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        std::stringstream Stream;
        Stream << "D" << char('a' + i);
        CAction& ClimbDown = Actions.emplace_back(Stream.str());
        ClimbDown.SetPrecondition("At", i);
        ClimbDown.SetPrecondition("BoxAt", i);
        ClimbDown.SetPrecondition("Level", ELevel::high);
        ClimbDown.SetEffect("Level", ELevel::low);
    }

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        std::stringstream Stream;
        Stream << "T" << char('a' + i);
        CAction& TakeBanana = Actions.emplace_back(Stream.str());
        TakeBanana.SetPrecondition("At", i);
        TakeBanana.SetPrecondition("BananaAt", i);
        TakeBanana.SetPrecondition("Level", ELevel::high);
        TakeBanana.SetEffect("HasBanana", true);
    }

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
