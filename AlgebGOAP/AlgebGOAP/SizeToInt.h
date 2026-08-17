// Copyright 2025 Isaac Hsu

#pragma once

#include <cstdint>


namespace AlgebGOAP
{
    // Type trait for providing a signed integer type based on a given byte size
    template <int N>
    struct SSizeToInt;

    template <>
    struct SSizeToInt<1>
    {
        using Type = int8_t;
    };

    template <>
    struct SSizeToInt<2>
    {
        using Type = int16_t;
    };

    template <>
    struct SSizeToInt<4>
    {
        using Type = int32_t;
    };

    template <>
    struct SSizeToInt<8>
    {
        using Type = int64_t;
    };

    // Type trait for providing an unsigned integer type based on a given byte size
    template <int N>
    struct SSizeToUInt;

    template <>
    struct SSizeToUInt<1>
    {
        using Type = uint8_t;
    };

    template <>
    struct SSizeToUInt<2>
    {
        using Type = uint16_t;
    };

    template <>
    struct SSizeToUInt<4>
    {
        using Type = uint32_t;
    };

    template <>
    struct SSizeToUInt<8>
    {
        using Type = uint64_t;
    };
}