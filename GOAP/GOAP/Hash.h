// Copyright 2024 Isaac Hsu

#pragma once

#include <string>


namespace std
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TKey, typename TValue> // Hash functor for std::pair
    struct hash< pair<TKey, TValue> >
    {
        size_t operator () (const pair<TKey, TValue>& Source) const noexcept
        {
            size_t Hash1 = std::hash<TKey>{}(Source.first);
            size_t Hash2 = std::hash<TValue>{}(Source.second);
            return Hash1 ^ (Hash2 + 0x8d194be2 + (Hash1 << 6) + (Hash1 >> 2));
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
