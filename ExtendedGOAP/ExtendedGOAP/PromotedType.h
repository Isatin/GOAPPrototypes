// Copyright 2026 Isaac Hsu

#pragma once


namespace ExtendedGOAP
{
    // Type trait for providing the promoted type of a given floating-point type
    template <typename T>
    struct SPromotedType;

    template <>
    struct SPromotedType<float>
    {
        using Type = double;

        static_assert(sizeof(SPromotedType<float>::Type) > sizeof(float));
    };

    //template <>
    //struct SPromotedType<double>
    //{
    //    using Type = long double;

    //    static_assert(sizeof(SPromotedType<double>::Type) > sizeof(double));
    //};
}