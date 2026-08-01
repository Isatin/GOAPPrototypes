// Copyright 2024 Isaac Hsu

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Segment.h"


namespace ArithGOAP
{
    class CFact;
    class CFactDefinition;
    struct SFactEquation;
    struct SNumericFactRange;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CState // World state storing Boolean, enumeration, and numeric properties
    {
        friend class CAction;
        friend class CEffect;
        friend class CFactDefinition;
    public:
        CState(const CFactDefinition& Definition);
        virtual ~CState() {}

        virtual std::unique_ptr<CState> Clone() const { return std::make_unique<CState>(*this); }
        virtual std::string ToString() const; // For debugging
        virtual float GetExtraHeuristicCost(const CState& Another) const { return 0.f; } // Custom heuristic cost

        float GetBaseHeuristicCost(const CState& Another) const; // Heuristic cost based on property comparisons
        const CFactDefinition& GetDefinition() const { return mDefinition; }

        bool IsEmpty() const { return CountProperties() <= 0; }
        int CountProperties() const; // Number of properties that are set
        int GetPropertyCapacity() const { return static_cast<int>(mProperties.size()); } // Total number of all properties, including unset ones
        const std::vector<SSegment>& GetProperties() const { return mProperties; }
        SSegment& GetProperty(const CFact& Fact);
        const SSegment& GetProperty(const CFact& Fact) const;
        const SSegment& GetProperty(int FactIndex) const;
        bool SetProperty(const CFact& Fact, const SSegment& Segment);
        bool SetProperty(const CFact& Fact, CNumber Value);
        bool SetProperty(const SFactEquation& Equation);
        bool SetProperty(const SNumericFactRange& Range);

        // Are any properties in this state contradictory to those in another state?
        bool IsContradictory(const CState& Another) const;
        // Clamp the property values to the fact ranges.
        void Clamp();

    protected:
        void Expand(int Size);
        SSegment& GetProperty(int FactIndex);
        bool SetProperty(int FactIndex, const SSegment& Segment);

    private:
        const CFactDefinition& mDefinition;
        std::vector<SSegment> mProperties;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
