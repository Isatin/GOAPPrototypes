// Copyright 2026 Isaac Hsu

#include "BitVector.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CBitVector CBitVector::operator | (const CBitVector& Another)
{
    CBitVector Return = *this;
    if (Return.size() < Another.size())
    {
        Return.resize(Another.size());
    }

    for (int i = 0; i < Another.size(); i++)
    {
        if (Another[i])
        {
            Return[i] = true;
        }
    }

    return Return;
}

CBitVector& CBitVector::operator |= (const CBitVector& Another)
{
    if (size() < Another.size())
    {
        resize(Another.size());
    }

    for (int i = 0; i < Another.size(); i++)
    {
        if (Another[i])
        {
            at(i) = true;
        }
    }

    return *this;
}

CBitVector::reference CBitVector::GetOrAdd(int Index, bool NewValue)
{
    if (size() <= Index)
    {
        resize(Index + 1, NewValue);
    }

    return at(Index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////