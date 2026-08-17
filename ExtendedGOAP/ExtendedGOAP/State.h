// Copyright 2026 Isaac Hsu

#pragma once

#include <memory>
#include <vector>

#include "Variant.h"


namespace ExtendedGOAP
{
    class CBooleanFact;
    class CCondition;
    class CEnumerationFact;
    class CFact;
    class CFactDefinition;
    class CMatrixFact;
    class CNumericFact;
    class CSet;
    class CSetFact;
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
        const CVariant& GetProperty(const CFact& Fact) const;
        const CVariant& GetProperty(int FactIndex) const;
        void SetProperty(const CBooleanFact& Fact, bool Value);
        void SetProperty(const CEnumerationFact& Fact, int Value);
        void SetProperty(const CNumericFact& Fact, CNumber Value);
        void SetProperty(const CMatrixFact& Fact, const CMatrix& Value);
        void SetProperty(const CMatrixFact& Fact, CMatrix&& Value);
        void SetProperty(const CSetFact& Fact, const CSet& Value);
        void SetProperty(const CSetFact& Fact, CSet&& Value);
        // PREREQUISITE: The variant type is compatible with the fact type.
        void SetProperty(int FactIndex, const CVariant& Value) { SetProperty(FactIndex, CVariant(Value)); }
        // PREREQUISITE: The variant type is compatible with the fact type.
        void SetProperty(int FactIndex, CVariant&& Value);

        // Compose a union set from set properties.
        CSet GetUnionSet() const;
        CSet GetUnionSetIC() const; // IC stands for "ignore complement".
        // Convert complement sets to original sets for set properties.
        void ConvertComplementSets(const CSet& UniversalSet);

    protected:
        void Expand(int Size);

    protected:
        const CFactDefinition& mDefinition;
        std::vector<CVariant> mProperties;
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
