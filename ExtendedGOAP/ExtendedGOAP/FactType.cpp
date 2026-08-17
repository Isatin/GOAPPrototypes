// Copyright 2026 Isaac Hsu

#pragma once

#include <cassert>

#include "FactType.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool EFactType::IsScalar() const
{
    switch (mValue)
    {
    case none:          return false;
    case boolean:       return true;
    case enumeration:   return true;
    case number:        return true;
    case matrix:        return false;
    case set:           return false;
    }

    assert(!"Invalid fact type");
    return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
