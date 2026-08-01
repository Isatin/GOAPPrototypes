// Copyright 2024 Isaac Hsu

#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "Segment.h"


namespace ArithGOAP
{
    using CHeuristicFunctor = std::function<CNumber(const SSegment&, const SSegment&)>;
    class CState;
    struct SFactOperation;
    struct SVariableRange;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    enum class EFactType // Enum class of fact types
    {
        none,
        boolean,
        enumeration,
        number,
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CFact // Base class for a world property definition. Fact values are stored in world states.
    {
        friend class CBooleanFact;
        friend class CEnumerationFact;
        friend class CFactDefinition;
        friend class CNumericFact;
    public:
        virtual ~CFact() = 0 {}

        const std::string& GetName() const { return mName; }
        EFactType GetType() const { return mType; }
        int GetIndex() const { return mIndex; } // Index of this fact in the owner's fact list
        const SSegment& GetRange() const { return mRange; }
        void SetRange(const SSegment& Value) { mRange = Value; }
        const CHeuristicFunctor& GetHeuristicFunctor() const { return mHeuristicFunctor; }
        void SetHeuristicFunctor(const CHeuristicFunctor& Value) { mHeuristicFunctor = Value; }
        void SetGapWeight(CNumber GapWeight);
        const CFactDefinition& GetOwner() const { return mOwner; }

    private:
        CFact(CFactDefinition& Owner, int Index, const std::string& Name, EFactType Type, const SSegment& Range, const CHeuristicFunctor& HeuristicFunctor);

    private:
        std::string mName;
        EFactType mType = EFactType::none;
        int mIndex = -1;
        SSegment mRange;
        CHeuristicFunctor mHeuristicFunctor;
        CFactDefinition& mOwner;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CBooleanFact : public CFact // Boolean fact used to define preconditions and effects with C++ operators
    {
        friend class CFactDefinition;
    public:
        SFactOperation operator = (bool Value) const;

    private:
        CBooleanFact(CFactDefinition& Owner, int Index, const std::string& Name, const SSegment& Range, const CHeuristicFunctor& HeuristicFunctor);

    public:
        static constexpr EFactType StaticFactType = EFactType::boolean;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CEnumerationFact : public CFact // Enumeration fact used to define preconditions and effects with C++ operators
    {
        friend class CFactDefinition;
    public:
        SFactOperation operator = (int Value) const;

    private:
        CEnumerationFact(CFactDefinition& Owner, int Index, const std::string& Name, const SSegment& Range, const CHeuristicFunctor& HeuristicFunctor);

    public:
        static constexpr EFactType StaticFactType = EFactType::enumeration;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CNumericFact : public CFact // Numeric fact for used to define preconditions and effects with C++ operators
    {
        friend class CFactDefinition;
    public:
        SFactOperation operator = (CNumber Value) const;
        SFactOperation operator += (CNumber Value) const;
        SFactOperation operator -= (CNumber Value) const;
        SFactOperation operator *= (CNumber Value) const;
        SFactOperation operator /= (CNumber Value) const;

    private:
        CNumericFact(CFactDefinition& Owner, int Index, const std::string& Name, const SSegment& Range, const CHeuristicFunctor& HeuristicFunctor);

    public:
        static constexpr EFactType StaticFactType = EFactType::number;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CFactDefinition // Collection of fact definitions
    {
        friend CFact;
    public:
        explicit CFactDefinition(CNumber BaseRelationCost = 1, CNumber Tolerance = CNumber::DefaultTolerance);

        std::string StringizeBoundedRanges() const; // For debugging

        auto begin()        { return mFacts.begin(); }
        auto begin() const  { return mFacts.begin(); }
        auto end()          { return mFacts.end(); }
        auto end() const    { return mFacts.end(); }

        CBooleanFact* DefineBoolean(const std::string& Name);
        CEnumerationFact* DefineEnumeration(const std::string& Name, const SSegment& Range = SSegment::Boundless);
        CNumericFact* DefineNumber(const std::string& Name, const SSegment& Range = SSegment::Boundless, CNumber GapWeight = 1);
        CNumericFact* DefineNumber(const std::string& Name, CNumber GapWeight, const SSegment& Range = SSegment::Boundless);
        CNumericFact* DefineNumber(const SVariableRange& Range, CNumber GapWeight = 1);
        CNumericFact* DefineNumber(const SVariableRange& Range, const CHeuristicFunctor& HeuristicFunctor);

        int GetFactCount() const { return static_cast<int>(mFacts.size()); }
        const CFact* GetFact(int Index) const;
        const CFact* GetFact(const std::string& Name) const;

        // Do any facts have a lower or upper bound? 
        bool HasAnyRange() const;
        CNumber GetBaseRelationCost() const { return mBaseRelationCost; }        
        CNumber GetTolerance() const { return mTolerance; }
        // Calculate the heuristic cost from the source state to the desired state.
        CNumber GetHeuristicCost(const CState& SourceState, const CState& DesiredState) const;
        // Clamp a given state to these fact ranges.
        void Clamp(CState& State) const;

    private:
        template <typename TFact>
        TFact* Define(const std::string& Name, const SSegment& Range = SSegment::Boundless, const CHeuristicFunctor& HeuristicFunctor = CHeuristicFunctor());
        bool ValidateDefinitionParameters(const std::string& Name, EFactType Type, SSegment& Range, CHeuristicFunctor& HeuristicFunctor);
        CHeuristicFunctor GenerateHeuristicFunctor(EFactType Type, CNumber GapWeight);

    private:
        std::vector<std::unique_ptr<CFact>> mFacts; // Allocate the facts on the heap to ensure that existing facts are not invalidated by adding new on.
        std::unordered_map<std::string, int> mNameMap; // Mapping of fact names to fact indexes
        CNumber mBaseRelationCost = 1; // Base cost for a mismatched comparison
        CNumber mTolerance = CNumber::DefaultTolerance; // Absolute tolerance used for floating-point comparison
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TFact>
    TFact* CFactDefinition::Define(const std::string& Name, const SSegment& Range, const CHeuristicFunctor& HeuristicFunctor)
    {
        SSegment ValidRange = Range;
        CHeuristicFunctor ValidHeuristicFunctor = HeuristicFunctor;
        if (!ValidateDefinitionParameters(Name, TFact::StaticFactType, ValidRange, ValidHeuristicFunctor))
        {
            return nullptr;
        }

        TFact* Fact = new TFact(*this, static_cast<int>(mFacts.size()), Name, ValidRange, ValidHeuristicFunctor);
        mFacts.emplace_back(Fact);
        mNameMap.emplace(Name, Fact->GetIndex());
        return Fact;
    }
}

