// Copyright 2024 Isaac Hsu

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "Interval.h"


namespace ArithGOAP
{
    using CHeuristicFunctor = std::function<SNumber(const SInterval&, const SInterval&)>;
    class CState;
    struct SBooleanFactOperation;
    struct SFactOperation;
    struct SFactRange;
    struct SNumericFactOperation;
    struct SNumericFactRange;
    struct SVariableRange;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    enum class EFactType
    {
        none,
        boolean,
        enumeration,
        number,
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CFact // Definition of a world property. Fact values are stored in the state class.
    {
        friend class CBooleanFact;
        friend class CNumericFact;
        friend class CStateDefinition;
    public:
        virtual ~CFact() {}

        const std::string& GetName() const { return mName; }
        EFactType GetType() const { return mType; }
        int GetIndex() const { return mIndex; } // Index of this fact in fact list of the owner
        const SInterval& GetRange() const { return mRange; }
        void SetRange(const SInterval& Value) { mRange = Value; }
        const CHeuristicFunctor& GetHeuristicFunctor() const { return mHeuristicFunctor; }
        void SetHeuristicFunctor(const CHeuristicFunctor& Value) { mHeuristicFunctor = Value; }
        void SetDistanceWeight(SNumber DistanceWeight);
        const CStateDefinition& GetOwner() const { return mOwner; }

        SFactRange operator == (SNumber Value) const;
        SFactOperation operator = (SNumber Value) const;

    private:
        CFact(CStateDefinition& Owner, int Index, const std::string& Name, EFactType Type, const SInterval& Range, const CHeuristicFunctor& HeuristicFunctor);

    private:
        std::string mName;
        EFactType mType = EFactType::none;
        int mIndex = -1;
        SInterval mRange;
        CHeuristicFunctor mHeuristicFunctor;
        CStateDefinition& mOwner;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CBooleanFact : public CFact // Boolean fact to define operations using C++ operators
    {
        friend class CStateDefinition;
    public:
        SBooleanFactOperation operator = (SNumber Value) const;
        SBooleanFactOperation operator ! () const;

    private:
        CBooleanFact(CStateDefinition& Owner, int Index, const std::string& Name, EFactType Type, const SInterval& Range, const CHeuristicFunctor& HeuristicFunctor);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CNumericFact : public CFact // Numeric fact to define ranges/operations using C++ operators
    {
        friend class CStateDefinition;
    public:
        SNumericFactRange operator == (SNumber Value) const;
        SNumericFactRange operator <= (SNumber Value) const;
        SNumericFactRange operator >= (SNumber Value) const;
        friend SNumericFactRange operator <= (SNumber Value, const CNumericFact& Fact);
        friend SNumericFactRange operator >= (SNumber Value, const CNumericFact& Fact);

        SNumericFactOperation operator = (SNumber Value) const;
        SNumericFactOperation operator += (SNumber Value) const;
        SNumericFactOperation operator -= (SNumber Value) const;
        SNumericFactOperation operator *= (SNumber Value) const;
        SNumericFactOperation operator /= (SNumber Value) const;

        SNumericFactOperation operator - () const;

    private:
        CNumericFact(CStateDefinition& Owner, int Index, const std::string& Name, EFactType Type, const SInterval& Range, const CHeuristicFunctor& HeuristicFunctor);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CStateDefinition // Collection of fact definitions
    {
        friend CFact;
    public:
        explicit CStateDefinition(SNumber BaseCost = 1) : mBaseCost(BaseCost) {}

        std::string StringizeBoundedRanges() const; // For debug

        auto begin() { return mFacts.begin(); }
        auto begin() const { return mFacts.begin(); }
        auto end() { return mFacts.end(); }
        auto end() const { return mFacts.end(); }

        CBooleanFact* DefineBoolean(const std::string& Name);
        CFact* DefineEnumeration(const std::string& Name, const SInterval& Range = SInterval::Boundless);
        CNumericFact* DefineNumber(const std::string& Name, const SInterval& Range = SInterval::Boundless, SNumber DistanceWeight = 1);
        CNumericFact* DefineNumber(const std::string& Name, SNumber DistanceWeight, const SInterval& Range = SInterval::Boundless);
        CNumericFact* DefineNumber(const SVariableRange& Range, SNumber DistanceWeight = 1);
        CNumericFact* DefineNumber(const SVariableRange& Range, const CHeuristicFunctor& HeuristicFunctor);

        int GetFactAmount() const { return (int) mFacts.size(); }
        const CFact* GetFact(int Index) const;
        const CFact* GetFact(const std::string& Name) const;

        // Calculate heuristic cost from the source state to desired state
        SNumber GetHeuristicCost(const CState& SourceState, const CState& DesiredState) const;
        // Clamp the given state by range definition
        void Clamp(CState& State) const;

    private:
        template <typename TFact>
        TFact* Define(const std::string& Name, EFactType Type, const SInterval& Range = SInterval::Boundless, const CHeuristicFunctor& HeuristicFunctor = CHeuristicFunctor());
        bool ValidateDefinitionParameters(const std::string& Name, EFactType Type, SInterval& Range, CHeuristicFunctor& HeuristicFunctor);
        CHeuristicFunctor GenerateHeuristicFunctor(EFactType Type, SNumber DistanceWeight);

    private:
        std::vector<std::unique_ptr<CFact>> mFacts;
        std::unordered_map<std::string, int> mNameMap; // mappings from fact names to fact indices
        SNumber mBaseCost = 1; // base cost of a mismatched fact
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TFact>
    TFact* CStateDefinition::Define(const std::string& Name, EFactType Type, const SInterval& Range, const CHeuristicFunctor& HeuristicFunctor)
    {
        SInterval ValidRange = Range;
        CHeuristicFunctor ValidHeuristicFunctor = HeuristicFunctor;
        if (!ValidateDefinitionParameters(Name, Type, ValidRange, ValidHeuristicFunctor))
        {
            return nullptr;
        }

        TFact* Fact = new TFact(*this, (int) mFacts.size(), Name, Type, ValidRange, ValidHeuristicFunctor);
        mFacts.emplace_back(Fact);
        mNameMap.emplace(Name, Fact->GetIndex());
        return Fact;
    }
}

