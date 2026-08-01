// Copyright 2024 Isaac Hsu

#include <cmath>
#include <format>

#include "Vector.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
bool SVector::IsValid() const
{
    return !std::isnan(X) && !std::isnan(Y);
}

float SVector::Length() const
{
    return sqrtf(SquaredLength());
}

std::string SVector::ToString() const
{
    return std::format("({},{})", X, Y);
}
///////////////////////////////////////////////////////////////////////////////////////////////////