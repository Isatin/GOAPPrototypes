// Copyright 2024 Isaac Hsu

#include <map>

#include "State.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string CState::ToString() const
{
    std::string Return;
    bool Successive = false;

    for (const auto& [Name, Value] : mPropertyMap)
    {
        if (Successive)
        {
            Return += ", ";
        }
        else
        {
            Successive = true;
        }

        Return += Name;
        Return += "=";
        Return += std::to_string(Value);
    }

    return Return;
}

std::string CState::Stringize(const CState& Another) const
{
    std::string Return;
    bool Successive = false;

    for (const auto& [Name, Value] : mPropertyMap)
    {
        if (Successive)
        {
            Return += ", ";
        }
        else
        {
            Successive = true;
        }

        Return += Name;
        Return += "=";

        std::optional<BProperty> Other = Another.GetProperty(Name);
        if (Other)
        {
            Return += "(";
            Return += std::to_string(Value);
            Return += Value == Other ? "==" : "!=";
            Return += std::to_string(*Other);
            Return += ")";
        }
        else
        {
            Return += std::to_string(Value);
        }
    }

    return Return;
}

std::optional<BProperty> CState::GetProperty(const std::string& Name) const
{
    auto it = mPropertyMap.find(Name);
    if (it == mPropertyMap.end())
    {
        return {};
    }

    return it->second;
}

void CState::SetProperty(const std::string& Name, BProperty Value)
{
    mPropertyMap.emplace(Name, Value);
}

bool CState::IsSatisfiedBy(const CState& Another) const
{
    for (auto& [Name, Target] : mPropertyMap)
    {
        std::optional<BProperty> Other = Another.GetProperty(Name);
        if (Other != Target)
        {
            return false;
        }
    }

    return true;
}

int CState::CountUnsatisfiedProperties(const CState& Another) const
{
    int Count = 0;

    for (auto& [Name, Target] : mPropertyMap)
    {
        std::optional<BProperty> Other = Another.GetProperty(Name);
        if (Other != Target)
        {
            Count++;
        }
    }

    return Count;
}

void CState::Overwrite(CState& Another) const
{
    for (const auto& [Name, Value] : mPropertyMap)
    {
        Another.mPropertyMap[Name] = Value;
    }
}

void CState::CopyProperties(const CState& Source, const CState& Filter)
{
    for (auto& [Name, Target] : Filter)
    {
        if (std::optional<BProperty> Value = Source.GetProperty(Name))
        {
            mPropertyMap[Name] = *Value;
        }
    }
}

void CState::InitializeProperties(const CState& Source, const CState& Filter)
{
    for (auto [Name, Target] : Filter)
    {
        if (std::optional<BProperty> Value = Source.GetProperty(Name))
        {
            mPropertyMap.emplace(Name, *Value); // unordered_map::emplace doesn't replace existing elements.
        }
    }
}

void CState::RemoveMatch(const CState& Another)
{
    for (auto it = mPropertyMap.begin(); it != mPropertyMap.end();)
    {
        std::optional<BProperty> Other = Another.GetProperty(it->first);
        if (Other == it->second)
        {
            it = mPropertyMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////