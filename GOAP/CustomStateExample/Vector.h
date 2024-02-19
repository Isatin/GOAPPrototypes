// Copyright 2024 Isaac Hsu

#pragma once


///////////////////////////////////////////////////////////////////////////////////////////////////
struct SVector // 2D vector
{
    union
    {
        struct
        {
            float X, Y;
        };
        float elements[2];
    };

    SVector() = default;
    SVector(float iX, float iY) : X(iX), Y(iY) {}
    SVector(float Value) : X(Value), Y(Value) {}

    float& operator() (int i) { return elements[i]; }
    float operator() (int i) const { return elements[i]; }

    SVector operator + (const SVector& Another) const;
    SVector operator - (const SVector& Another) const;

    bool IsValid() const;

    float Length() const;
    float SquaredLength() const;

    void Set(float iX, float iY);
};
///////////////////////////////////////////////////////////////////////////////////////////////////
inline float SVector::SquaredLength() const
{
    return elements[0] * elements[0] + elements[1] * elements[1];
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