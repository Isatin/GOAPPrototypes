// Copyright 2024 Isaac Hsu

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>


namespace GOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    using PFact = int; // Primitive type of fact value
    ///////////////////////////////////////////////////////////////////////////////////////////////    
    class CState // World state supporting Boolean and enumerations    
    {
    public:     
        virtual ~CState() {}
        virtual std::unique_ptr<CState> Clone() const { return std::make_unique<CState>(*this); }
        virtual std::string ToString() const; // For debug
        virtual float GetExtraHeuristicCost(const CState& Another) const { return 0.f; } // Custom heuristic cost

        auto begin() { return mFactMap.begin(); }
        auto begin() const { return mFactMap.begin(); }
        auto end() { return mFactMap.end(); }
        auto end() const { return mFactMap.end(); }

        // World properties are call facts here.
        int GetFactCount() const { return (int) mFactMap.size(); }
        std::optional<PFact> GetFact(const std::string& Key) const;
        void SetFact(const std::string& Key, PFact Value);

        // Check if the facts match the other state's. The other could have more Facts.
        bool IsSatisfiedBy(const CState& Another) const;
        // How many different facts from the other state?
        int GetUnsatisfactionCount(const CState& Another) const;
        // Copy the facts to the other state
        void Overwrite(CState& Another) const;
        // Remove the facts equal to the other state's
        void RemoveMatch(const CState& Another);

    private:
        std::unordered_map<std::string, PFact> mFactMap;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}