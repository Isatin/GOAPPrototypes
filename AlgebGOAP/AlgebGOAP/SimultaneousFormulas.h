// Copyright 2025 Isaac Hsu

#pragma once

#include "Formula.h"


namespace AlgebGOAP
{
    struct SDebugInfo;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // System of simultaneous logical formulas ANDed together
    // TODO: Replace this class with one in a CAS library.
    class CSimultaneousFormulas 
    {
    public:
        static constexpr const char* GetEmptySymbol() { return "{}"; }

    public:
        CSimultaneousFormulas() = default;
        CSimultaneousFormulas(const CFormula&  Formula);
        CSimultaneousFormulas(      CFormula&& Formula);
        // PREREQUISITE: Call CRange::Sort beforehand.
        CSimultaneousFormulas(const CExpression& Expression, const CRange& Range);        

        std::string ToString(const CFactDefinition& Definition, bool Enclose = true) const;
        bool IsEmpty() const { return mFormulas.empty(); }
        bool IsOnlyOneBool() const;
        // PREREQUISITE: IsOnlyOneBool returns true.
        bool GrabTheOnlyBool() const;
        int CountRelations() const;
        int GetFormulaCount() const { return static_cast<int>(mFormulas.size()); }
        CBitVector GetUsedFactBits() const;
        ETriState Evaluate(const CStateBase& State) const;
        CNumber GetHeuristicCost(const CState& State, SDebugInfo* DebugInfo = nullptr) const;
        [[nodiscard]] CSimultaneousFormulas GetRearranged(const CFactDefinition& Definition) const;

        void Clear();
        void SetTo(bool Value);
        void Append(const CSimultaneousFormulas& Right);
        void Unite();
        void Unite(const CSimultaneousFormulas& Right);
        void ReplaceFact(const CFact& Fact, const COperand& Replacement);
        void ReplaceFact(const CFact& Fact, const CExpression& Replacement);
        void ReplaceFact(int FactIndex, const COperand& Replacement);
        void ReplaceFact(int FactIndex, const CExpression& Replacement);
        template <typename T>
        void ReplaceFacts(T&& Replacements);
        void Rearrange(const CFactDefinition& Definition);
        void RemoveRedundancy(const CFactDefinition& Definition);
        void ApplyDeMorgansLaws();

    private:
        // PREREQUISITE: Call CRange::Sort on the given range beforehand.
        bool AddFormulas(const CExpression& Expression, const CRange& Range);
        void AddFormulas(const CExpression& Expression, const SSpan& Span);
        bool AddFormula(EComparer Comparer, const CExpression&  Left, CNumber Right);
        bool AddFormula(EComparer Comparer,       CExpression&& Left, CNumber Right);
        // PREREQUISITE: Call Rearrange on these formulas beforehand.
        void SimplifyCompoundInequalities(const CFactDefinition& Definition);
        void RemoveIdenticalFormulas();

    private:
        std::vector<CFormula> mFormulas;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    void CSimultaneousFormulas::ReplaceFacts(T&& Replacements)
    {
        for (CFormula& Formula : mFormulas)
        {
            Formula.ReplaceFacts(std::forward<T>(Replacements));
        }
    }
}