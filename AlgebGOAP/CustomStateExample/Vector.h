// Copyright 2025 Isaac Hsu

#pragma once

#include <string>


///////////////////////////////////////////////////////////////////////////////////////////////////
struct SVector // 2D vector
{
    union
    {
        struct
        {
            float X, Y;
        };
        float Elements[2];
    };

public:
    SVector() = default;
    SVector(float iX, float iY) : X(iX), Y(iY) {}
    SVector(float Value) : X(Value), Y(Value) {}

    float& operator() (int i) { return Elements[i]; }
    float operator() (int i) const { return Elements[i]; }

    SVector operator + (const SVector& Another) const;
    SVector operator - (const SVector& Another) const;

    bool IsValid() const;
    std::string ToString() const;
    float Length() const;
    float SquaredLength() const;

    void Set(float iX, float iY);
};
///////////////////////////////////////////////////////////////////////////////////////////////////
inline float SVector::SquaredLength() const
{
    return Elements[0] * Elements[0] + Elements[1] * Elements[1];
}

inline SVector SVector::operator + (const SVector& Another) const
{
    return SVector(X + Another.X, Y + Another.Y);
}

inline SVector SVector::operator - (const SVector& Another) const
{
    return SVector(X - Another.X, Y - Another.Y);
}

inline void SVector::Set(float iX, float iY)
{
    X = iX;
    Y = iY;
}