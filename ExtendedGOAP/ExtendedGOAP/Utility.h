// Copyright 2026 Isaac Hsu

#pragma once

#include <string>

#include "Common.h"


namespace ExtendedGOAP
{
    bool IsParenthetic(const std::string& String);
    std::string Parenthesize(const std::string& String);
    std::string& Parenthesize(std::string& String);
    size_t CountCodepoints(const std::string& u8String);
    std::string Pad(const std::string& u8String, size_t Length, char Padding = ' ');
    std::string& Pad(std::string& u8String, size_t Length, char Padding = ' ');
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SIndexedText
    {
        int Index = InvalidIndex;
        std::string Text; // TODO: Try std::pmr::string with std::pmr::monotonic_buffer_resource for performance.

    public:
        static bool Compare(const SIndexedText& Left, const SIndexedText& Right);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SIndexedSizeAndText
    {
        int Index = InvalidIndex;
        size_t Size = 0;
        std::string Text; // TODO: Try std::pmr::string with std::pmr::monotonic_buffer_resource for performance.

    public:
        static bool Compare(const SIndexedSizeAndText& Left, const SIndexedSizeAndText& Right);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}