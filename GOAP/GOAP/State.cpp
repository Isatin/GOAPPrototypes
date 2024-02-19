// Copyright 2024 Isaac Hsu

#include <map>
#include <sstream>

#include "State.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string CState::ToString() const
{
    std::stringstream Stream;
    bool First = true;

    for (const auto [Key, Value] : mFactMap)
    {
        if (First)
        {
            First = false;
        }
        else
        {
            Stream << ", ";
        }

        Stream << Key << "=" << Value;
    }

    return Stream.str();
}

std::optional<PFact> CState::GetFact(const std::string& Key) const
{
    auto it = mFactMap.find(Key);
    if (it == mFactMap.end())
    {
        return {};
    }

    return it->second;
}

void CState::SetFact(const std::string& Key, PFact Value)
{
    mFactMap.emplace(Key, Value);
}

bool CState::IsSatisfiedBy(const CState& Another) const
{
    for (auto& [Key, Aim] : mFactMap)
    {
        std::optional<PFact> Other = Another.GetFact(Key);
        if (Other != Aim)
        {
            return false;
        }
    }

    return true;
}

int CState::GetUnsatisfactionCount(const CState& Another) const
{
    int Count = 0;

    for (auto& [Key, Aim] : mFactMap)
    {
        std::optional<PFact> Other = Another.GetFact(Key);
        if (Other != Aim)
        {
            Count++;
        }
    }

    return Count;
}

void CState::Overwrite(CState& Another) const
{
    for (const auto [Key, Value] : mFactMap)
    {
        Another.mFactMap[Key] = Value;
    }
}

void CState::RemoveMatch(const CState& Another)
{
    for (auto it = mFactMap.begin(); it != mFactMap.end();)
    {
        std::optional<PFact> Other = Another.GetFact(it->first);
        if (Other == it->second)
        {
            it = mFactMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////