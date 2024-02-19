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


using namespace ArithGOAP;
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
    CStateDefinition Definition;
    const auto& HasBananaFact = *Definition.DefineBoolean("HasBanana");
    const auto& AtFact = *Definition.DefineEnumeration("At");
    const auto& LevelFact = *Definition.DefineEnumeration("Level");
    const auto& BoxAtFact = *Definition.DefineEnumeration("BoxAt");
    const auto& BananaAtFact = *Definition.DefineEnumeration("BananaAt");

    CState GoalState(Definition);
    GoalState.SetFact(HasBananaFact, true);    

    CState StartingState(Definition);
    StartingState.SetFact(AtFact, EPlace::A);
    StartingState.SetFact(LevelFact, ELevel::low);
    StartingState.SetFact(BoxAtFact, EPlace::C);
    StartingState.SetFact(BananaAtFact, EPlace::B);
    StartingState.SetFact(HasBananaFact, false);

    std::vector<CAction> Actions;

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        for (int j = EPlace::A; j <= EPlace::C; j++)
        {
            if (i != j)
            {
                std::stringstream Stream;
                Stream << "G" << char('a' + i) << char('a' + j);
                CAction& Goto = Actions.emplace_back(Stream.str(), Definition);
                Goto.SetPrecondition(LevelFact, ELevel::low);
                Goto.SetPrecondition(AtFact, i);
                Goto.SetEffect(AtFact, j);
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
                CAction& MoveBox = Actions.emplace_back(Stream.str(), Definition);
                MoveBox.SetPrecondition(LevelFact, ELevel::low);
                MoveBox.SetPrecondition(AtFact, i);
                MoveBox.SetPrecondition(BoxAtFact, i);
                MoveBox.SetEffect(AtFact, j);
                MoveBox.SetEffect(BoxAtFact, j);
            }
        }
    }

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        std::stringstream Stream;
        Stream << "U" << char('a' + i);
        CAction& ClimbUp = Actions.emplace_back(Stream.str(), Definition);
        ClimbUp.SetPrecondition(AtFact, i);
        ClimbUp.SetPrecondition(BoxAtFact, i);
        ClimbUp.SetPrecondition(LevelFact, ELevel::low);
        ClimbUp.SetEffect(LevelFact, ELevel::high);
    }

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        std::stringstream Stream;
        Stream << "D" << char('a' + i);
        CAction& ClimbDown = Actions.emplace_back(Stream.str(), Definition);
        ClimbDown.SetPrecondition(AtFact, i);
        ClimbDown.SetPrecondition(BoxAtFact, i);
        ClimbDown.SetPrecondition(LevelFact, ELevel::high);
        ClimbDown.SetEffect(LevelFact, ELevel::low);
    }

    for (int i = EPlace::A; i <= EPlace::C; i++)
    {
        std::stringstream Stream;
        Stream << "T" << char('a' + i);
        CAction& TakeBanana = Actions.emplace_back(Stream.str(), Definition);
        TakeBanana.SetPrecondition(AtFact, i);
        TakeBanana.SetPrecondition(BananaAtFact, i);
        TakeBanana.SetPrecondition(LevelFact, ELevel::high);
        TakeBanana.SetEffect(HasBananaFact, true);
    }

    //{
    //    CAction& MoveBanana = Actions.emplace_back("Bba", Definition);
    //    MoveBanana.SetPrecondition(AtFact, EPlace::A);
    //    MoveBanana.SetPrecondition(LevelFact, ELevel::high);
    //    MoveBanana.SetPrecondition(BananaAtFact, EPlace::B);
    //    MoveBanana.SetEffect(BananaAtFact, EPlace::A);
    //}

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
