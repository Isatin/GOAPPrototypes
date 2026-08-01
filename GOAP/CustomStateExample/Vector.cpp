// Copyright 2024 Isaac Hsu

#include <charconv>
#include <cmath>
#include <system_error>

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
    auto StringizeValue = [](float Value)
    {
        const size_t BufferSize = 16;
        char Buffer[BufferSize] = { 0 };
        auto [End, Error] = std::to_chars(Buffer, Buffer + sizeof(Buffer), Value);

        if (Error == std::errc{})
        {
            return std::string(Buffer, End);
        }
        else
        {
            return std::make_error_code(Error).message();
        }
    };

    std::string Text = "(";
    Text += StringizeValue(X);
    Text += ",";
    Text += StringizeValue(Y);
    Text += ")";
    return Text;
}
///////////////////////////////////////////////////////////////////////////////////////////////////