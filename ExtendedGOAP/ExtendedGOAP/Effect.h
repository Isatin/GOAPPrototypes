// Copyright 2026 Isaac Hsu

#pragma once

#include <vector>

#include "Expression.h"


namespace ExtendedGOAP
{
    class CBitVector;
    class CFact;
    class CFactAssignment;
    class CFactDefinition;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct STransform
    {
        int FactIndex = InvalidIndex;
        CExpression Expression;

    public:
        STransform() = default;
        STransform(int FactIndex, const CExpression&  Expression);
        STransform(int FactIndex,       CExpression&& Expression);

        bool IsIdentityFunction() const { return Expression.IsOnlyOneFact() && Expression.GrabTheOnlyFactIndex() == FactIndex; }
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CEffect // Collection of fact transformations
    {
        friend class CAction;
    public:
        CEffect(const CFactDefinition& Definition);

        auto begin()        { return mTransforms.begin(); }
        auto begin() const  { return mTransforms.begin(); }
        auto end()          { return mTransforms.end(); }
        auto end() const    { return mTransforms.end(); }

        bool HasAnyTransform(const CBitVector& FactBits) const;
        int GetTransformCount() const { return static_cast<int>(mTransforms.size()); }
        STransform& GetTransform(int TransformIndex);
        const STransform& GetTransform(int TransformIndex) const { return const_cast<CEffect*>(this)->GetTransform(TransformIndex); }
        const STransform* GetFirstTransform(const CFact& Fact) const;
        bool AddTransform(const CNumericFact& Fact, const CExpression&  Expression);
        bool AddTransform(const CNumericFact& Fact,       CExpression&& Expression);
        bool AddTransform(CFactAssignment&& Assignment);
        void ClearTransforms() { mTransforms.clear(); }

        std::string ToString() const; // For debugging
        bool IsEmpty() const { return mTransforms.empty(); }
        // PREREQUISITE: Call Rearrange beforehand.
        bool IsNeutral() const;
        const CFactDefinition& GetDefinition() const { return mDefinition; }
        // Return a bit vector where each bit indicates whether the corresponding fact is referenced in this effect.
        CBitVector GetUsedFactBits() const;
        // Return a bit vector where each bit indicates whether the corresponding fact can be affected by this effect.
        CBitVector GetAffectedFactBits() const;
        // Return a bit vector where each bit indicates whether the corresponding fact can affect given facts via this effect.
        CBitVector GetAffectingFactBits(const CBitVector& FactBits) const;
        // Apply this effect to a given state.
        bool ApplyTo(CState& oState) const;

        template <typename T>
        void ReplaceFacts(T&& Replacements);
        void Rearrange();

    private:
        bool AddTransform(int FactIndex, const CExpression&  Expression) { return AddTransform(FactIndex, CExpression(Expression)); }
        bool AddTransform(int FactIndex,       CExpression&& Expression);

    private:
        const CFactDefinition& mDefinition;
        std::vector<STransform> mTransforms;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    void CEffect::ReplaceFacts(T&& Replacements)
    {
        for (STransform& Transform : mTransforms)
        {
            Transform.Expression.ReplaceFacts(std::forward<T>(Replacements));
        }
    }
}