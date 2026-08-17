// Copyright 2025 Isaac Hsu

#pragma once

#include <memory>
#include <unordered_map>

#include "Notation.h"
#include "Number.h"
#include "OperationConcept.h"
#include "State.h"


namespace AlgebGOAP
{
    class CBitVector;
    class CFactAssignment;
    class COperand;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EFactType // Mock enum of fact types, also used as bit flags
    {
    public:
        enum Type
        {
            none        = 0,
            boolean     = 1 << 0,
            enumeration = 1 << 1,
            number      = 1 << 2,
        };

    public:
        EFactType() = default;
        EFactType(Type Value) : mValue(Value) {}

        operator Type() const { return mValue; }
        EFactType& operator |= (Type Right);

    private:
        Type mValue = none;
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

        const std::string& GetName() const      { return mName; }
        EFactType GetType() const               { return mType; }
        int GetIndex() const                    { return mIndex; } // Index of this fact in the owner's fact list
        const CFactDefinition& GetOwner() const { return mOwner; }

        virtual bool IsBoolean() const          { return false; }
        virtual bool IsEnumeration() const      { return false; }
        virtual bool IsNumeric() const          { return false; }
        virtual CNumber GetGapWeight() const    { return 1; }

    private:
        CFact(CFactDefinition& Owner, int Index, const std::string& Name, EFactType Type);

    private:
        std::string mName;
        EFactType mType = EFactType::none;
        int mIndex = InvalidIndex;
        CFactDefinition& mOwner;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CBooleanFact : public CFact // Boolean fact used to define preconditions and effects with C++ operators
    {
        friend class CFactDefinition;
    public:
        CFactAssignment operator = (bool Right) const;
        CFactAssignment operator = (const CBooleanFact& Right) const;
        CFactAssignment operator = (const CBooleanExpression& Right) const;

        bool IsBoolean() const override { return true; }

    private:
        CBooleanFact(CFactDefinition& Owner, int Index, const std::string& Name);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CEnumerationFact : public CFact // Enumeration fact used to define preconditions and effects with C++ operators
    {
        friend class CFactDefinition;
    public:
        CFactAssignment operator = (int Right) const;
        CFactAssignment operator = (const CEnumerationFact& Right) const;

        bool IsEnumeration() const override { return true; }

    private:
        CEnumerationFact(CFactDefinition& Owner, int Index, const std::string& Name);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CNumericFact : public CFact // Numeric fact used to define preconditions and effects with C++ operators
    {
        friend class CFactDefinition;
    public:
        CFactAssignment operator = (CNumber Right) const            { return AssignedTo(Right); }
        CFactAssignment operator = (const CNumericFact& Right) const{ return AssignedTo(Right); }
        CFactAssignment operator = (const CExpression& Right) const { return AssignedTo(Right); }

        template <RNumericArgument T>
        CFactAssignment operator += (T&& Right) const { return {*this, EOperator::addition, std::forward<T>(Right)}; }
        template <RNumericArgument T>
        CFactAssignment operator -= (T&& Right) const { return {*this, EOperator::subtraction, std::forward<T>(Right)}; }
        template <RNumericArgument T>
        CFactAssignment operator *= (T&& Right) const { return {*this, EOperator::multiplication, std::forward<T>(Right)}; }
        template <RNumericArgument T>
        CFactAssignment operator /= (T&& Right) const { return {*this, EOperator::division, std::forward<T>(Right)}; }
        template <RNumericArgument T>
        CFactAssignment operator %= (T&& Right) const { return {*this, EOperator::modulo, std::forward<T>(Right)}; }

        bool IsNumeric() const override { return true; }

        CNumber GetGapWeight() const override { return mGapWeight; }
        void SetGapWeight(CNumber GapWeight);

    private:
        CNumericFact(CFactDefinition& Owner, int Index, const std::string& Name, CNumber GapWeight);

        template <RNumericArgument T>
        CFactAssignment AssignedTo(T&& Right) const { return {*this, EOperator::nil, std::forward<T>(Right)}; }

    private:
        CNumber mGapWeight = 1;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CFactDefinition // Collection of fact definitions
    {
        friend CFact;
    public:
        explicit CFactDefinition(CNumber BaseRelationCost = 1, CNumber Tolerance = CNumber::GetDefaultTolerance(), CNumber GapOffset = 0);

        auto begin()        { return mFacts.begin(); }
        auto begin() const  { return mFacts.begin(); }
        auto end()          { return mFacts.end(); }
        auto end() const    { return mFacts.end(); }

        CBooleanFact* DefineBoolean(const std::string& Name);
        CEnumerationFact* DefineEnumeration(const std::string& Name);
        CNumericFact* DefineNumber(const std::string& Name, CNumber GapWeight = 1);

        int GetFactCount() const { return static_cast<int>(mFacts.size()); }
        const CFact* GetFact(int Index) const;
        const CFact* GetFact(const std::string& Name) const;

        CNumber GetBaseRelationCost() const { return mBaseRelationCost; }
        void SetBaseRelationCost(CNumber Value) { mBaseRelationCost = Value; }

        CNumber GetGapOffset() const { return mGapOffset; }
        void SetGapOffset(CNumber Value) { mGapOffset = Value; }

        // Return fact types for given facts.
        EFactType GetFactTypes(const CBitVector& FactBits) const;
        // Return the concatenated names of given facts.
        std::string StringizeFactBits(const CBitVector& FactBits, const char* Delimiter = " ") const;
        CNumber GetTolerance() const { return mTolerance; }
        // Return the smallest difference weight among given facts.
        CNumber GetMinGapWeight(const CBitVector& FactBits) const;
        // Return the heuristic cost and weight for given facts based on a given difference value.
        std::pair<CNumber, CNumber> GetHeuristicCost(CNumber Gap, const CBitVector& FactBits) const;
        // Return an empty state based on this fact definition.
        const CStateBase& GetEmptyState() const { return mEmptyState; }

    private:
        template <typename TFact, typename... TArgs>
        TFact* Define(const std::string& Name, TArgs&&... Args);
        bool ValidateDefinition(const std::string& Name);

    private:
        std::vector<std::unique_ptr<CFact>> mFacts; // Allocate the facts on the heap to ensure that existing facts are not invalidated by adding new on.
        std::unordered_map<std::string, int> mNameMap; // Mapping of fact names to fact indexes
        CNumber mBaseRelationCost = 1; // Base cost for a mismatched comparison
        CNumber mTolerance = CNumber::GetDefaultTolerance(); // Absolute tolerance used in floating-point comparison
        CNumber mGapOffset = 0; // Optional positive offset for strict inequality to avoid a zero heuristic when the two sides are equal
        const CStateBase mEmptyState; // Since empty states are used in many places, an instance is defined here for reuse. 
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename TFact, typename... TArgs>
    TFact* CFactDefinition::Define(const std::string& Name, TArgs&&... Args)
    {
        if (!ValidateDefinition(Name))
        {
            return nullptr;
        }

        TFact* Fact = new TFact(*this, static_cast<int>(mFacts.size()), Name, std::forward<TArgs>(Args)...);
        mFacts.emplace_back(Fact);
        mNameMap.emplace(Name, Fact->GetIndex());
        return Fact;
    }
}
