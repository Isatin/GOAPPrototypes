// Copyright 2024 Isaac Hsu

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Interval.h"


namespace ArithGOAP
{
    class CFact;
    class CStateDefinition;
    struct SFactRange;
    struct SNumericFactRange;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CState // World state supporting Boolean, enumerations, and numbers 
    {
        friend class CAction;
        friend class CEffect;
        friend class CStateDefinition;
    public:
        CState(const CStateDefinition& Def);
        CState(const CStateDefinition& Def, std::vector<SInterval>&& Facts);

        virtual ~CState() {}
        virtual std::string ToString() const; // For debug
        virtual float GetExtraHeuristicCost(const CState& Another) const { return 0.f; } // Custom heuristic cost
        virtual std::unique_ptr<CState> Clone(std::vector<SInterval>&& Facts) const { return std::make_unique<CState>(mDefinition, std::move(Facts)); }

        std::unique_ptr<CState> Clone() const;
        float GetBasicHeuristicCost(const CState& Another) const; // Heuristic cost based on fact differences
        const CStateDefinition& GetDefinition() const { return mDefinition; }

        int GetFactCount() const { return (int) mValues.size(); }
        const std::vector<SInterval>& GetFacts() const { return mValues; }
        SInterval& GetFact(const CFact& Fact);
        const SInterval& GetFact(const CFact& Fact) const;
        const SInterval& GetFact(int Index) const;
        bool SetFact(const CFact& Fact, const SInterval& Interval);
        bool SetFact(const CFact& Fact, SNumber Value);
        bool SetFact(const SFactRange& Range);
        bool SetFact(const SNumericFactRange& Range);

        // Check if all facts of this state has intersection with another state's
        bool HasIntersection(const CState& Another) const;
        // Clamp self by defined ranges
        void Clamp();

    protected:
        void Expand(int Size);
        SInterval& GetFact(int Index);
        bool SetFact(int Index, const SInterval& Interval);

    private:
        const CStateDefinition& mDefinition;
        std::vector<SInterval> mValues;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
