// Copyright 2026 Isaac Hsu

#pragma once

#include <vector>

#include "Connective.h"
#include "Relation.h"
#include "Utility.h"


namespace ExtendedGOAP
{
    class CBitVector;
    class CFactDefinition;
    class CRange;
    class CState;
    struct SDebugInfo;
    struct SSpan;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    enum class EPredicate : unsigned char // Enum of operand types in logical operations
    {
        null,
        boolean,    // Truth value
        atomic,     // Atomic formula
        compound,   // Compound formula
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CPredicate // Operand of logical operations that can be a truth value, a relation reference, or a compound reference
    {
        friend class CFormula;
        friend struct SCompound;
        friend struct SRepeatedLogicalOperation;
    public:
        CPredicate() = default;

        bool operator == (const CPredicate& Another) const;

        EPredicate GetType() const { return mType; }
        bool GetBool() const;
        int GetRelationIndex() const;
        int GetCompoundIndex() const;

        bool IsNull() const                 { return mType == EPredicate::null; }
        bool IsBool() const                 { return mType == EPredicate::boolean; }
        bool IsAtomic() const               { return mType == EPredicate::atomic; }
        bool IsAtomic(int Index) const      { return IsAtomic() && mIndex == Index; }
        bool IsCompound() const             { return mType == EPredicate::compound; }
        bool IsCompound(int Index) const    { return IsCompound() && mIndex == Index; }

        void SetRelationIndex(int RelationIndex);
        void SetCompoundIndex(int CompoundIndex);

    private:
        CPredicate(bool Value);
        CPredicate(EPredicate Type, int Index);
        CPredicate(EPredicate Type, size_t Index);

        void SetNull();
        void SetBool(bool Value);
        void SetAtom(int RelationIndex);
        void SetCompound(int CompoundIndex);
        void ShiftIndexes(int CompoundOffset, int RelationOffset);

    public:
        static const CPredicate Null;

    private:
        EPredicate mType = EPredicate::null;
        union
        {
            int mIndex = InvalidIndex;
            bool mBool;
        };
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct SCompound // Logical operation storing a logical connective, a left operand, and a right operand
    {
        EConnective Connective;
        CPredicate Left;    // NOTE: The left predicate must be null in a unary logical operation.
        CPredicate Right;   // NOTE: The right predicate must not be null in an logical operation.

    public:
        SCompound() = default;
        explicit SCompound(bool Value);
        SCompound(EPredicate Type, int Index);
        SCompound(EConnective Connective, const CPredicate& Predicate);
        SCompound(EConnective Connective, const CPredicate& Left, const CPredicate& Right);

        bool operator == (const SCompound& Another) const;

        bool HasConnective() const { return Connective.IsNonEmpty(); }
        bool IsOnlyOneBool() const;
        bool IsOnlyOneAtom() const;
        // PREREQUISITE: IsOnlyOneAtom returns true.
        int GrabTheOnlyRelationIndex() const;

        void SetTo(bool Value);
        void SetTo(const CPredicate& Operand);
        void ShiftIndexes(int CompoundOffset, int RelationOffset);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Logical formula composed of truth values and relational expressions combined with logical connectives
    // TODO: Replace this class with one in a CAS library.
    class CFormula
    {
    public:
        static constexpr const char* GetEmptySymbol() { return "()"; }

        // Build a logical formula joined with a given connective, equivalent to the compound inequalities defined by a given expression and range.
        // Return true if succeeded.
        // PREREQUISITE: Call CRange::Sort on the given range beforehand.
        static bool BuildFormula(CFormula& oFormula, EConnective Connective, const CExpression& Expression, const CRange& Range);

    public:
        CFormula() = default;
        CFormula(bool Value);
        CFormula(const CRelation&  Relation);
        CFormula(      CRelation&& Relation);
        template <RBooleanArgument T> requires RBooleanOperation<T>
        CFormula(T&& Argument) : CFormula(CRelation(std::forward<T>(Argument))) {}
        CFormula(EConnective Connective, const CRelation&  Relation);
        CFormula(EConnective Connective,       CRelation&& Relation);
        CFormula(EConnective Connective, const CFormula&  Formula);
        CFormula(EConnective Connective,       CFormula&& Formula);
        CFormula(EConnective Connective, const CRelation&  Left, const CRelation&  Right);
        CFormula(EConnective Connective,       CRelation&& Left,       CRelation&& Right);
        CFormula(EConnective Connective, const CRelation&  Left, const CFormula&  Right);
        CFormula(EConnective Connective,       CRelation&& Left,       CFormula&& Right);
        CFormula(EConnective Connective, const CFormula&  Left, const CRelation&  Right);
        CFormula(EConnective Connective,       CFormula&& Left,       CRelation&& Right);
        CFormula(EConnective Connective, const CFormula&  Left, const CFormula&  Right);
        CFormula(EConnective Connective,       CFormula&& Left,       CFormula&& Right);

        bool operator == (const CFormula& Another) const { return IsEqual(Another); }

        std::string ToString(const CFactDefinition& Definition) const { return ToString_Variadic(Definition); }
        std::string ToString() const { return ToString_Variadic(); }
        std::string ToString(int StartCompoundIndex) const;

        bool IsEmpty() const { return mCompounds.empty(); }
        bool IsEqual(const CFormula& Another) const;
        bool IsEquivalent(const CFormula& Another) const;
        bool IsFactUsed(int FactIndex) const;
        bool IsOnlyOneRelation() const;
        bool IsOnlyOnePredicate() const;
        bool IsOnlyOneBool() const;
        bool IsBool(bool Value) const;
        int GetCompoundCount() const { return static_cast<int>(mCompounds.size()); }
        int GetRelationCount() const { return static_cast<int>(mRelations.size()); }
        // Return the common connective of these logical compounds if available. Otherwise, return invalid.
        EConnective GetCommonConnective() const;
        // PREREQUISITE: This formula is not empty.
        const SCompound& GrabLastCompound() const { return mCompounds.back(); }
        // PREREQUISITE: IsOnlyOneRelation returns true.
        const CRelation& GrabTheOnlyRelation() const;
        // PREREQUISITE: IsOnlyOneBool returns true.
        bool GrabTheOnlyBool() const;
        // Return yes/no if this is just a Boolean constant. Otherwise, return unknown.
        ETriState TryGetTheOnlyBool() const;
        bool ValidateCompound(const SCompound& Compound) const;
        [[nodiscard]] std::vector<int> GetInvalidCompoundIndexes() const;

        // Return a bit vector indicating which facts are referenced in this formula.
        CBitVector GetUsedFactBits() const;
        [[nodiscard]] CFormula GetSubFormula(int StartCompoundIndex) const;
        [[nodiscard]] CFormula GetRearranged(const CFactDefinition& Definition) const;
        // Return the heuristic cost of this formula for a given state.
        // PREREQUISITE: Call ApplyDeMorgansLaws beforehand.
        CNumber GetHeuristicCost(const CState& State, SDebugInfo* DebugInfo = nullptr) const;
        // Evaluate this formula for a given state.
        ETriState Evaluate(const CStateBase& State) const;
        int CountUnusedCompounds() const;
        int CountUnusedRelations() const { return CountUnusedCompoundsAndRelations().second; }
        // Return the index and reference count of the most referenced compound.
        std::pair<int, int> CountMostUsedCompound() const;
        // Return the index and reference count of the most referenced relation.
        std::pair<int, int> CountMostUsedRelation() const;
        // Return the numbers of the unreferenced compounds and relations.
        std::pair<int, int> CountUnusedCompoundsAndRelations() const;
        // Return the number of the referenced compounds without a connective.
        int CountUsedAtomicCompounds() const;
        // Return invalid compound reference indexes.
        std::vector<int> GetInvalidCompoundReferences() const;

        void Clear();
        void SetTo(bool Value);
        void Append(EConnective Connective, const CFormula&  Right);
        void Append(EConnective Connective,       CFormula&& Right);
        void ReplaceFact(const CFact& Fact, const COperand& Replacement);
        void ReplaceFact(const CFact& Fact, const CExpression& Replacement);
        void ReplaceFact(int FactIndex, const COperand& Replacement);
        void ReplaceFact(int FactIndex, const CExpression& Replacement);
        template <typename T>
        void ReplaceFacts(T&& Replacements);
        void ApplyDeMorgansLaws();
        // Removes redundant Boolean constants in this formula by applying identity and domination laws.
        void RemoveRedundantBoolsAndAtomicCompounds(const CFactDefinition& Definition);
        // Rearrange the operands into a consistent order so that equivalent formulas yield the same form.
        void Rearrange(const CFactDefinition& Definition);
        // Split this formula into a list of simultaneous formulas.
        // PREREQUISITE: Call Rearrange beforehand.
        void SplitInto(std::vector<CFormula>& oFormulas);

    private:
        struct SRepeatedLogicalOperation;

        void AddRelations(const CExpression& Expression, const SSpan& Span);
        bool AddRelation(EComparer Comparer, const CExpression& Left, CNumber Right);
        int AddUpArity(const std::vector<int>& CompoundIndexes) const;
        // PREREQUISITE: IsOnlyOnePredicate returns true.
        const CPredicate& GrabTheOnlyPredicate() const;
        std::vector<int> GetUsedCompoundIndexes() const;
        std::vector<int> GetUsedCompoundIndexes(int StartCompoundIndex) const;
        void GetUsedCompoundAndRelationIndexes(std::vector<int>& oCompoundIndexes, std::vector<int>& oRelationIndexes) const;
        void GetUsedCompoundAndRelationIndexes(std::vector<int>& oCompoundIndexes, std::vector<int>& oRelationIndexes, int StartCompoundIndex) const;
        bool ValidateCompound(const SCompound& Compound, int CompoundLimit) const;
        template <typename... TArgs>
        std::string StringizePredicate(const CPredicate& Predicate, std::vector<std::string>& oTexts, TArgs&&... Args) const;
        template <typename... TArgs>
        std::string ToString_Variadic(TArgs&&... Args) const;
        std::string ToString(const std::vector<int>& CompoundIndexes) const;

        void ShiftIndexes(int CompoundOffset, int RelationOffset);
        void SortRelations();
        void SortRelations(std::vector<int>& RelationIndexes) const;
        void SortSubFormulas(std::vector<int>& StartCompoundIndexes) const;
        [[nodiscard]] std::unordered_map<int, int> ReplaceCompound(int CompoundIndex, CFormula&& Replacement);
        // PREREQUISITE: Call Rearrange on these relations beforehand.
        void ReduceUnconditionalAndUnsatisfiableInequalities();
        void RemoveUnusedCompoundsAndRelations();
        // PREREQUISITE: Call Rearrange on these relations and RemoveRedundantBoolsAndAtomicCompounds beforehand.
        void RearrangeConjunctionAndDisjunction(CNumber Tolerance);
        // Combine and simplify given relations using a given connective and return the combined formula.
        // PREREQUISITE: Call Rearrange on the given relations beforehand.
        CFormula CombineCompoundInequalities(EConnective Connective, const std::vector<int>& RelationIndexes, CNumber Tolerance);
        void SplitInto(std::vector<CFormula>& Formulas, int StartCompoundIndex);

    private:
        // NOTE: Compounds can only reference preceding compounds, not subsequent ones. 
        // NOTE: Each compound and relation can be referenced only once to simplify the implementation of De Morgan's laws.
        std::vector<SCompound> mCompounds;
        std::vector<CRelation> mRelations;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    concept RFormulaArgument =
        std::is_same_v<std::remove_cvref_t<T>, bool> ||
        std::is_same_v<std::remove_cvref_t<T>, CRelation> ||
        std::is_same_v<std::remove_cvref_t<T>, CFormula>;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <RFormulaArgument T>
    inline CFormula operator ! (T&& Argument) { return {EConnective::logicalNOT, std::forward<T>(Argument)}; }
    template <RFormulaArgument TLeft, RFormulaArgument TRight>
    inline CFormula operator && (TLeft&& Left, TRight&& Right) { return {EConnective::logicalAND, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }
    template <RFormulaArgument TLeft, RFormulaArgument TRight>
    inline CFormula operator || (TLeft&& Left, TRight&& Right) { return {EConnective::logicalOR, std::forward<TLeft>(Left), std::forward<TRight>(Right)}; }

    template <typename... TArgs>
    std::string CFormula::ToString_Variadic(TArgs&&... Args) const
    {
        if (IsOnlyOneRelation())
        {
            return GrabTheOnlyRelation().ToString(std::forward<TArgs>(Args)...);
        }

        std::vector<std::string> Texts(mCompounds.size());

        for (int i = 0; i < static_cast<int>(mCompounds.size()); i++)
        {
            const SCompound& Compound = mCompounds[i];
            std::string LeftString = StringizePredicate(Compound.Left, Texts, std::forward<TArgs>(Args)...);
            std::string RightString = StringizePredicate(Compound.Right, Texts, std::forward<TArgs>(Args)...);
            Texts[i] = Compound.Connective.Stringize(LeftString, RightString);

            if (!Compound.IsOnlyOneBool() && i < static_cast<int>(mCompounds.size()) - 1) // Parenthesize inner compounds except only one bool.
            {
                Parenthesize(Texts[i]);
            }
        }

        return Texts.empty() ? GetEmptySymbol() : Texts.back();
    }

    template <typename... TArgs>
    std::string CFormula::StringizePredicate(const CPredicate& Predicate, std::vector<std::string>& oTexts, TArgs&&... Args) const
    {
        if (Predicate.IsBool())
        {
            return std::string(Predicate.GetBool() ? ETriState::Yes.GetSymbol() : ETriState::No.GetSymbol());
        }
        else if (Predicate.IsAtomic())
        {
            const CRelation& Relation = mRelations[Predicate.GetRelationIndex()];
            return Parenthesize(Relation.ToString(std::forward<TArgs>(Args)...));
        }
        else if (Predicate.IsCompound())
        {
            return oTexts[Predicate.GetCompoundIndex()];
        }

        return std::string();
    }

    template <typename T>
    void CFormula::ReplaceFacts(T&& Replacements)
    {
        for (CRelation& Relation : mRelations)
        {
            Relation.GetLeft().ReplaceFacts(std::forward<T>(Replacements));
            Relation.GetRight().ReplaceFacts(std::forward<T>(Replacements));
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
