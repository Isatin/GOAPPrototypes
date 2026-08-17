// Copyright 2026 Isaac Hsu

#include <cassert>

#include "TriState.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const ETriState ETriState::Unknown(unknown);
const ETriState ETriState::No(no);
const ETriState ETriState::Yes(yes);

ETriState ETriState::operator ! () const
{
    switch (mValue)
    {
    case unknown:   return unknown;
    case yes:       return no;
    case no:        return yes;
    }

    return unknown;
}

bool ETriState::ToBool() const
{
    switch (mValue)
    {
    case yes:       return true;
    case no:        return false;
    }

    assert(!"Can't convert a non-truth value to a bool");
    return false;
}

const char* ETriState::GetName() const
{
    switch (mValue)
    {
    case unknown:   return "unknown";
    case yes:       return "yes";
    case no:        return "no";
    }

    return "UNDEF";
}

const char* ETriState::GetSymbol() const
{
    switch (mValue)
    {
    case unknown:   return "?";
#ifdef USE_UNICODE_SYMBOLS
    case yes:       return "⊤";
    case no:        return "⊥";
#else
    case yes:       return "T";
    case no:        return "F"; 
#endif
    }

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////