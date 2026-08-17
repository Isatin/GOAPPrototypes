// Copyright 2025 Isaac Hsu

#pragma once

#include <memory>
#include <vector>

#include "Number.h"


namespace AlgebGOAP
{
    class CBooleanFact;
    class CCondition;
    class CEnumerationFact;
    class CFact;
    class CFactDefinition;
    class CNumericFact;
    struct SDebugInfo;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CStateBase // Base class for world state with no virtual members
    {
    public:
        explicit CStateBase(const CFactDefinition& Definition);

        const CFactDefinition& GetDefinition() const { return mDefinition; }

        bool IsEmpty() const { return CountProperties() <= 0; }
        int CountProperties() const; // Number of properties that are set
        int GetPropertyCapacity() const { return static_cast<int>(mProperties.size()); } // Total number of all properties, including unset ones
        CNumber GetProperty(const CFact& Fact) const;
        CNumber GetProperty(int FactIndex) const;
        void SetProperty(const CBooleanFact& Fact, bool Value);
        void SetProperty(const CEnumerationFact& Fact, int Value);
        void SetProperty(const CNumericFact& Fact, CNumber Value);

    protected:
        void Expand(int Size);
        void SetProperty(int FactIndex, CNumber Value);

    protected:
        const CFactDefinition& mDefinition;
        std::vector<CNumber> mProperties;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CState : public CStateBase // World state composed of world properties
    {
        using Super = CStateBase;
        friend class CEffect;
    public:
        explicit CState(const CFactDefinition& Definition);
        virtual ~CState() {}

        virtual std::unique_ptr<CState> Clone() const { return std::make_unique<CState>(*this); }
        virtual std::string ToString() const; // For debugging
        virtual float GetExtraHeuristicCost(const CCondition& Condition) const { return 0.f; } // Custom heuristic cost

        float GetBaseHeuristicCost(const CCondition& Condition, SDebugInfo* DebugInfo = nullptr) const; // Heuristic cost based on property comparisons
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
