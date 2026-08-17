// Copyright 2026 Isaac Hsu

#pragma once

#include <vector>

#include "Common.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////    
    class CBitVector : private std::vector<bool> // A wrapper for std::vector<bool> with some utility functions
    {
        using Super = std::vector<bool>;
    public:
        using Super::vector;
        using Super::operator[];

        CBitVector operator | (const CBitVector& Another);
        CBitVector& operator |= (const CBitVector& Another);

        auto begin()        { return Super::begin(); }
        auto begin() const  { return Super::begin(); }
        auto end()          { return Super::end(); }
        auto end() const    { return Super::end(); }

        int GetSize() const { return static_cast<int>(size()); }

        reference GetOrAdd(int Index, bool NewValue = false);
        void Reserve(int NewSize) { reserve(static_cast<int>(NewSize)); }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}

