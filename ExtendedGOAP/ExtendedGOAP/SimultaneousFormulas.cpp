// Copyright 2026 Isaac Hsu

#include <algorithm>
#include <cassert>

#include "BitVector.h"
#include "Fact.h"
#include "Range.h"
#include "SimultaneousFormulas.h"
#include "State.h"


using namespace ExtendedGOAP;
/////////////////////////////////////////////////////////////////////////////////////////////////////
CSimultaneousFormulas::CSimultaneousFormulas(const CFormula& Formula)
{
    mFormulas.emplace_back(Formula);
}

CSimultaneousFormulas::CSimultaneousFormulas(CFormula&& Formula)
{
    mFormulas.emplace_back(std::move(Formula));
}

CSimultaneousFormulas::CSimultaneousFormulas(const CExpression& Expression, const CRange& Range)
{
    if (Range.IsEmpty())
    {
        SetTo(false);
        return;
    }
    if (Range.IsFull())
    {
        SetTo(true);
        return;
    }
    if (Expression.IsEmpty())
    {
        return;
    }

    if (!AddFormulas(Expression, Range)) // First try joining simultaneous formulas.
    {
        // If combining formulas with logical AND fails, build a single formula connected with logical OR instead.
        CFormula Formula;
        bool Ok = CFormula::BuildFormula(Formula, EConnective::logicalOR, Expression, Range);
        assert(Ok);
        mFormulas.emplace_back(std::move(Formula));
    }
}

bool CSimultaneousFormulas::AddFormulas(const CExpression& Expression, const CRange& Range)
{
    if (Range.GetSpanCount() == 1)
    {
        AddFormulas(Expression, Range.GrabFirstSpan());
    }
    else if (Range.GetSpanCount() > 1)
    {
        assert(Range.IsSorted());
        // Multiple non-overlapping spans might be ANDed together if they share the same approximation-exclusive endpoints in the middle.

        CSimultaneousFormulas TempFormulas;
        const SSpan& FirstSpan = Range.GrabFirstSpan();
        const SSpan& LastSpan = Range.GrabLastSpan();
        CNumber PrevSupremum = FirstSpan.Supremum;
        ESpanType PrevSupremumType = FirstSpan.SupremumType;
        if (PrevSupremumType.IsInclusive())
        {
            return false;
        }

        for (int i = 1; i < Range.GetSpanCount(); i++)
        {
            const SSpan& Span = Range.GetSpan(i);
            if (!PrevSupremum.IsEqual(Span.Infimum))
            {
                return false;
            }
            if (Span.InfimumType.IsInclusive())
            {
                return false;
            }

            ESpanType DecidingSpanType = (PrevSupremumType.Get() < Span.InfimumType.Get()) ? PrevSupremumType : Span.InfimumType;
            EComparer Comparer = DecidingSpanType.IsApproximationExclusive() ? EComparer::NotAlmostEqual : EComparer::NotEqual;
            TempFormulas.AddFormula(Comparer, Expression, PrevSupremum);
            PrevSupremum = Span.Supremum;
            PrevSupremumType = Span.SupremumType;
        }

        AddFormula(FirstSpan.InfimumType.GetInfimumComparer(), Expression, FirstSpan.Infimum);
        AddFormula(LastSpan.SupremumType.GetSuprenumComparer(), Expression, LastSpan.Supremum);
        std::ranges::move(TempFormulas.mFormulas, std::back_inserter(mFormulas));
    }

    return true;
}

void CSimultaneousFormulas::AddFormulas(const CExpression& Expression, const SSpan& Span)
{
    if (Span.Infimum.IsEqual(Span.Supremum) && Span.InfimumType == Span.SupremumType)
    {
        switch (Span.InfimumType)
        {
        case ESpanType::open:                   return; // The span is empty.
        case ESpanType::closed:                 AddFormula(EComparer::Equal, Expression, Span.Infimum); return;
        case ESpanType::approximate:            AddFormula(EComparer::AlmostEqual, Expression, Span.Infimum); return;
        case ESpanType::approximationExclusive: return; // The span is empty.
        }

        assert(!"Invalid span type");
        return;
    }

    AddFormula(Span.InfimumType.GetInfimumComparer(), Expression, Span.Infimum);
    AddFormula(Span.SupremumType.GetSuprenumComparer(), Expression, Span.Supremum);
}

bool CSimultaneousFormulas::AddFormula(EComparer Comparer, const CExpression& Left, CNumber Right)
{
    return AddFormula(Comparer, CExpression(Left), Right);
}

bool CSimultaneousFormulas::AddFormula(EComparer Comparer, CExpression&& Left, CNumber Right)
{
    assert(!Left.IsEmpty());
    assert(!Right.IsNull());

    if (Right.IsPositiveInfinity())
    {
        if (Comparer == EComparer::LessOrEqual || Comparer == EComparer::LessOrAlmostEqual || Comparer == EComparer::NotGreater)
        {
            return false;
        }
    }
    else if (Right.IsNegativeInfinity())
    {
        if (Comparer == EComparer::GreaterOrEqual || Comparer == EComparer::GreaterOrAlmostEqual || Comparer == EComparer::NotLess)
        {
            return false;
        }
    }

    mFormulas.emplace_back(CRelation(Comparer, std::move(Left), Right));
    return true;
}

std::string CSimultaneousFormulas::ToString(const CFactDefinition& Definition, bool Enclose) const
{
    std::string Text;

    if (Enclose)
    {
        Text += "{";
    }

    for (int i = 0; i < mFormulas.size(); i++)
    {
        if (i > 0)
        {
            Text += ", ";
        }

        Text += mFormulas[i].ToString(Definition);
    }

    if (Enclose)
    {
        Text += "}";
    }

    return Text.empty() ? GetEmptySymbol() : Text;
}

bool CSimultaneousFormulas::IsOnlyOneBool() const
{
    return mFormulas.size() == 1 && mFormulas.front().IsOnlyOneBool();
}

bool CSimultaneousFormulas::GrabTheOnlyBool() const
{
    assert(IsOnlyOneBool());

    return mFormulas.front().GrabTheOnlyBool();
}

int CSimultaneousFormulas::CountRelations() const
{
    int Return = 0;

    for (const CFormula& Formula : mFormulas)
    {
        Return += Formula.GetRelationCount();
    }

    return Return;
}

CBitVector CSimultaneousFormulas::GetUsedFactBits() const
{
    CBitVector Return;

    for (const CFormula& Formula : mFormulas)
    {
        Return |= Formula.GetUsedFactBits();
    }

    return Return;
}

ETriState CSimultaneousFormulas::Evaluate(const CStateBase& State) const
{
    if (mFormulas.empty())
    {
        return ETriState::unknown;
    }

    ETriState Return = ETriState::yes;

    for (const CFormula& Formula : mFormulas)
    {
        ETriState Result = Formula.Evaluate(State);
        if (Result == ETriState::unknown)
        {
            Return = ETriState::unknown;
        }
        else if (Result == ETriState::no)
        {
            return ETriState::no;
        }
    }

    return Return;
}

CNumber CSimultaneousFormulas::GetHeuristicCost(const CState& State, SDebugInfo* DebugInfo) const
{
    CNumber Return = 0;

    for (const CFormula& Formula : mFormulas)
    {
        Return += Formula.GetHeuristicCost(State, DebugInfo);
    }

    return Return;
}

void CSimultaneousFormulas::Clear()
{
    mFormulas.clear();
}

void CSimultaneousFormulas::SetTo(bool Value)
{
    mFormulas.resize(1);
    mFormulas.front().SetTo( Value);
}

void CSimultaneousFormulas::Append(const CSimultaneousFormulas& Right)
{
    mFormulas.insert(mFormulas.end(), Right.mFormulas.begin(), Right.mFormulas.end());
}

void CSimultaneousFormulas::Unite()
{
    if (mFormulas.empty())
    {
        return;
    }

    CFormula& FirstFormula = mFormulas.front();
    for (int i = 1; i < mFormulas.size(); i++)
    {
        FirstFormula.Append(EConnective::logicalAND, mFormulas[i]);
    }

    mFormulas.erase(mFormulas.begin() + 1, mFormulas.end());
}

void CSimultaneousFormulas::Unite(const CSimultaneousFormulas& Right)
{
    Unite();

    if (Right.IsEmpty())
    {
        return;
    }

    if (mFormulas.empty())
    {
        mFormulas.resize(1); // Make sure there's a head to append to.
    }

    for (const CFormula& Formula : Right.mFormulas)
    {
        mFormulas.front().Append(EConnective::logicalAND, Formula);
    }
}

void CSimultaneousFormulas::ReplaceFact(const CFact& Fact, const COperand& Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CSimultaneousFormulas::ReplaceFact(int FactIndex, const COperand& Replacement)
{
    for (CFormula& Formula : mFormulas)
    {
        Formula.ReplaceFact(FactIndex, Replacement);
    }
}

void CSimultaneousFormulas::ReplaceFact(const CFact& Fact, const CExpression& Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CSimultaneousFormulas::ReplaceFact(int FactIndex, const CExpression& Replacement)
{
    for (CFormula& Formula : mFormulas)
    {
        Formula.ReplaceFact(FactIndex, Replacement);
    }
}

CSimultaneousFormulas CSimultaneousFormulas::GetRearranged(const CFactDefinition& Definition) const
{
    CSimultaneousFormulas Return = *this;
    Return.Rearrange(Definition);
    return Return;
}

void CSimultaneousFormulas::Rearrange(const CFactDefinition& Definition)
{
    std::vector<CFormula> SplitFormulas;
    ETriState Result;

    for (CFormula& Formula : mFormulas)
    {
        Formula.Rearrange(Definition);

        switch (Formula.Evaluate(Definition.GetEmptyState()))
        {
        case ETriState::unknown:
            Result = ETriState::unknown;
            Formula.SplitInto(SplitFormulas);
            assert(!SplitFormulas.empty());
            break;

        case ETriState::no:
            Result = ETriState::no;
            SetTo(false);
            return;

        case ETriState::yes:
            if (SplitFormulas.empty()) // Check if no previous formulas evaluate to unknown.
            {
                Result = ETriState::yes;
            }           
            break;
        }
    }

    if (Result == ETriState::yes)
    {        
        SetTo(true); // Set to true if all formulas hold true.
    }
    else
    {
        mFormulas = std::move(SplitFormulas);
        RemoveIdenticalFormulas();
        SimplifyCompoundInequalities(Definition);
    }
}

void CSimultaneousFormulas::RemoveIdenticalFormulas()
{
    std::unordered_map<std::string, int> FormulaMap;
    std::vector<int> IndexesOfFormulaToRemove;
    FormulaMap.reserve(mFormulas.size());
    IndexesOfFormulaToRemove.reserve(mFormulas.size());
    for (int FormulaIndex = 0; FormulaIndex < mFormulas.size(); FormulaIndex++)
    {
        CFormula& Formula = mFormulas[FormulaIndex];
        std::string FormulaText = Formula.ToString();
        if (FormulaMap.find(FormulaText) == FormulaMap.end())
        {
            FormulaMap.emplace(FormulaText, FormulaIndex);
        }
        else
        {
            IndexesOfFormulaToRemove.push_back(FormulaIndex);
        }
    }

    for (int i = static_cast<int>(IndexesOfFormulaToRemove.size()) - 1; i >= 0; i--)
    {
        int FormulaIndex = IndexesOfFormulaToRemove[i];
        mFormulas.erase(mFormulas.begin() + FormulaIndex);
    }
}

void CSimultaneousFormulas::SimplifyCompoundInequalities(const CFactDefinition& Definition)
{
    // TODO: Simplify not only atomic formulas but also compound formulas like the following ones. 
    // (x<-1∨x>2)∧(x<-2∨x>1) ≡ x<-2∨x>2 and (x>1∧y≠0)∨(x>2∧y≠0) ≡ X>1∧y≠0

    if (mFormulas.size() <= 1)
    {
        return;
    }

    struct SIndexedRange
    {
        int Index = InvalidIndex;
        CRange Range;
    };

    std::unordered_map<std::string, SIndexedRange> FormulaMap;
    std::vector<int> IndexesOfFormulaToRemove;
    FormulaMap.reserve(mFormulas.size());
    IndexesOfFormulaToRemove.reserve(mFormulas.size());
    for (int FormulaIndex = 0; FormulaIndex < mFormulas.size(); FormulaIndex++)
    {
        CFormula& Formula = mFormulas[FormulaIndex];
        if (!Formula.IsOnlyOneRelation())
        {
            continue;
        }

        // Assuming that the terms can be transposed and the atomic formula is already rearranged, 
        // the right side should be the constant term and the rest should be on the left side.
        const CRelation& Relation = Formula.GrabTheOnlyRelation();
        if (!Relation.GetRight().IsOnlyOneConstant())
        {
            continue; // Skip if the assumption is broken.
        }

        std::string LeftText = Relation.GetLeft().ToString();
        auto it = FormulaMap.find(LeftText);
        if (it == FormulaMap.end())
        {
            CRange Range(Relation.GetComparer(), Relation.GetRight().GrabTheOnlyConstant(), Definition.GetTolerance());
            FormulaMap.emplace(LeftText, SIndexedRange{FormulaIndex, std::move(Range)});
        }
        else
        {
            // Try to find a common range for the compound inequality.
            it->second.Range.Intersect(Relation.GetComparer(), Relation.GetRight().GrabTheOnlyConstant());
            if (it->second.Range.IsEmpty())
            {
                SetTo(false); // There is one or more conflicts between these formulas, so set this formula to false.
                return;
            }
        }

        IndexesOfFormulaToRemove.push_back(FormulaIndex);
    }

    for (auto& [Text, IndexedRange] : FormulaMap)
    {
        auto& [FormulaIndex, Range] = IndexedRange;
        const CFormula& Formula = mFormulas[FormulaIndex];
        assert(Formula.IsOnlyOneRelation());
        const CRelation& Relation = Formula.GrabTheOnlyRelation();
        assert(Relation.GetRight().IsOnlyOneConstant());

        Range.Sort();
        CSimultaneousFormulas SimplifiedFormulas(Relation.GetLeft(), Range);
        std::ranges::move(SimplifiedFormulas.mFormulas, std::back_inserter(mFormulas));
    }

    for (int i = static_cast<int>(IndexesOfFormulaToRemove.size()) - 1; i >= 0; i--)
    {
        int FormulaIndex = IndexesOfFormulaToRemove[i];
        mFormulas.erase(mFormulas.begin() + FormulaIndex);
    }
}

void CSimultaneousFormulas::RemoveRedundancy(const CFactDefinition& Definition)
{
    for (CFormula& Formula : mFormulas)
    {
        Formula.RemoveRedundantBoolsAndAtomicCompounds(Definition);
    }
}

void CSimultaneousFormulas::ApplyDeMorgansLaws()
{
    for (CFormula& Formula : mFormulas)
    {
        Formula.ApplyDeMorgansLaws();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////