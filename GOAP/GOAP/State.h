// Copyright 2024 Isaac Hsu

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>


namespace GOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    using BProperty = int; // The value type of world properties
    ///////////////////////////////////////////////////////////////////////////////////////////////    
    class CState // World state storing Boolean and enumeration properties    
    {
    public:     
        virtual ~CState() {}
        virtual std::unique_ptr<CState> Clone() const { return std::make_unique<CState>(*this); }
        virtual std::string ToString() const; // For debugging
        virtual std::string Stringize(const CState& Another) const; // For debugging
        virtual float GetExtraHeuristicCost(const CState& Another) const { return 0.f; } // Custom heuristic cost

        auto begin()        { return mPropertyMap.begin(); }
        auto begin() const  { return mPropertyMap.begin(); }
        auto end()          { return mPropertyMap.end(); }
        auto end() const    { return mPropertyMap.end(); }

        int GetPropertyCount() const { return static_cast<int>(mPropertyMap.size()); }
        std::optional<BProperty> GetProperty(const std::string& Name) const;
        void SetProperty(const std::string& Name, BProperty Value);

        // Are there no properties set in this state?
        bool IsEmpty() const { return mPropertyMap.empty(); }
        // Do all properties in this state match another?
        bool IsSatisfiedBy(const CState& Another) const;
        // How many properties in this state differ from those in another?
        int CountUnsatisfiedProperties(const CState& Another) const;
        // Copy the properties to another state.
        void Overwrite(CState& Another) const;
        // Copy the property values from the source if those properties exist in the filter.
        void CopyProperties(const CState& Source, const CState& Filter);
        // Initialize unset properties from the source if they exist in the filter.
        void InitializeProperties(const CState& Source, const CState& Filter);
        // Remove properties that match the other state's.
        void RemoveMatch(const CState& Another);

    private:
        std::unordered_map<std::string, BProperty> mPropertyMap;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}