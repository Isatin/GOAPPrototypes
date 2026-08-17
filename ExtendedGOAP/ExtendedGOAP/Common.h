// Copyright 2026 Isaac Hsu

#pragma once


namespace ExtendedGOAP
{
    constexpr int InvalidIndex = -1;

    enum EGOAPFlag : unsigned
    {
        ShowMethodName          = 1 << 0,
        ShowStartAndGoal        = 1 << 1,
        ShowActions             = 1 << 2,
        ShowAlteredActions      = 1 << 3,
        ShowSteps               = 1 << 4,
        ShowFinalOpenNodes      = 1 << 5,
        ShowExcessiveHeuristic  = 1 << 6,
        ShowResult              = 1 << 7,

        ExtraGOAPFlag           = 1 << 8
    };
}