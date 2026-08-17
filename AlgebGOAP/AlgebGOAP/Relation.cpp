// Copyright 2025 Isaac Hsu

#include <algorithm>
#include <cassert>

#include "BitVector.h"
#include "Fact.h"
#include "Notation.h"
#include "Relation.h"
#include "State.h"
#include "Term.h"
#include "TriState.h"
#include "Utility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string CRelation::ToString() const
{ 
    return mComparer.Stringize(mLeft.ToString(), mRight.ToString()); 
}

std::string CRelation::ToString(const CFactDefinition& Definition) const
{
    return mComparer.Stringize(mLeft.ToString(Definition), mRight.ToString(Definition));
}

bool CRelation::IsEqual(const CRelation& Another) const
{
    if (mComparer != Another.mComparer)
    {
        return false;
    }

    if (!mLeft.IsEqual(Another.mLeft))
    {
        return false;
    }

    if (!mRight.IsEqual(Another.mRight))
    {
        return false;
    }

    return true;
}

bool CRelation::IsEquivalent(const CRelation& Another) const
{ 
    return ToString() == Another.ToString();
}

bool CRelation::IsTautological() const
{ 
    assert(mRight.IsOnlyOneConstant());

    return mComparer.IsTautological(mRight.GrabTheOnlyConstant());
}

ETriState CRelation::UnconditionallyEvaluate() const
{
    assert(mRight.IsOnlyOneConstant());

    return mComparer.UnconditionallyEvaluate(mRight.GrabTheOnlyConstant());
}

int CRelation::AddUpArity() const
{
    return mComparer.GetArity() + mLeft.AddUpArity() + mRight.AddUpArity();
}

CBitVector CRelation::GetUsedFactBits() const
{
    return mLeft.GetUsedFactBits() | mRight.GetUsedFactBits();
}

CNumber CRelation::GetGap(const CStateBase& State) const
{
    std::pair<CNumber, CNumber> Pair = EvaluateBothSides(State);
    return mComparer.GetGap(Pair.first, Pair.second, State.GetDefinition());
}

std::pair<CNumber, CNumber> CRelation::EvaluateBothSides(const CStateBase& State) const
{
    return {mLeft.Evaluate(State), mRight.Evaluate(State)};
}

ETriState CRelation::Evaluate(const CStateBase& State) const
{
    CNumber LeftValue = mLeft.Evaluate(State);
    if (LeftValue.IsNull())
    {
        return ETriState::unknown;
    }

    CNumber RightValue = mRight.Evaluate(State);
    if (RightValue.IsNull())
    {
        return ETriState::unknown;
    }

    return mComparer(LeftValue, RightValue, State.GetDefinition().GetTolerance()) ? ETriState::yes : ETriState::no;
}

void CRelation::ReplaceFact(const CNumericFact& Fact, CNumber Replacement)
{
    mLeft.ReplaceFact(Fact, Replacement);
    mRight.ReplaceFact(Fact, Replacement);
}

void CRelation::ReplaceFact(const CNumericFact& Fact, const CNumericFact& Replacement)
{
    mLeft.ReplaceFact(Fact, Replacement);
    mRight.ReplaceFact(Fact, Replacement);
}

void CRelation::ReplaceFact(const CNumericFact& Fact, const CExpression& Replacement)
{
    mLeft.ReplaceFact(Fact, Replacement);
    mRight.ReplaceFact(Fact, Replacement);
}

void CRelation::ReplaceFact(const CBooleanFact& Fact, const CBooleanExpression& Replacement)
{
    mLeft.ReplaceFact(Fact, Replacement.ToBase());
    mRight.ReplaceFact(Fact, Replacement.ToBase());
}

CRelation CRelation::GetRearranged() const
{
    CRelation Return = *this;
    Return.Rearrange();
    return Return;
}

void CRelation::Rearrange()
{
    if (IsEmpty())
    {
        return; // Early return to preserve empty relations.
    }

    RearrangeNegations();
    CancelOutNegationsOnBothSides();
    ConvertNegationToInequation();

    if (mLeft.IsTransposable() && mRight.IsTransposable())
    {
        RearrangeTerms();
    }
    else // Non-transposable expressions can be positive or negative infinities.
    {
        RearrangeNonTransposableSides();
    }
}

void CRelation::RearrangeTerms()
{
    assert(mLeft.IsTransposable());
    assert(mRight.IsTransposable());

    // Combine the left and right expressions in the form of terms.
    CTermList LeftTerms(mLeft);
    CTermList RightTerms(mRight);
    CTermList CombinedTerms = LeftTerms - RightTerms;
    CombinedTerms.Rearrange();

    // x < y is equivalent to -2x > -2y. 
    // To make implementing the comparison simpler, transform them into the same form.
    // TODO: Check if the scaling introduces glitches due to floating-point imprecision.
    CNumber Scale = CombinedTerms.NormalizeFirstCoefficientAndScaleOthers();
    if (!Scale.IsNull() && Scale < 0)
    {
        mComparer.Reverse();
    }

    // Transpose the constant term to the right side and the rest to the left.
    mRight.SetTo(-CombinedTerms.GetUsableConstantTerm());
    CombinedTerms.SetConstantTerm(0);
    mLeft = CombinedTerms.ToExpression();
}

void CRelation::RearrangeNonTransposableSides()
{
    mLeft.Rearrange();
    mRight.Rearrange();

    const std::string LeftText = mLeft.ToString();
    const std::string RightText = mRight.ToString();
    if (LeftText == RightText)
    {
        mLeft.SetTo(1);
        mRight.SetTo(1);
        return;
    }

    const bool LeftIsInfinity = mLeft.IsInfinity();
    const bool RightIsInfinity = mRight.IsInfinity();
    if (LeftIsInfinity)
    {
        if (!RightIsInfinity && !mRight.IsOnlyOneConstant())
        {
            SwapSides(); // Keep the only constant term on the right.
            return;
        }
    }
    else
    {
        if (RightIsInfinity && !mLeft.IsOnlyOneConstant())
        {
            return; // Keep the only constant term on the right.
        }
    }

    if (!mComparer.IsSymmetric())
    {
        if (!EComparer::IsForward(mComparer.GetType()))
        {
            SwapSides();
        }

        return;
    }

    const int LeftAritySum = mLeft.AddUpArity();
    const int RightAritySum = mRight.AddUpArity();
    if (LeftAritySum < RightAritySum)
    {
        SwapSides(); // Sort the two sides by number of operands.
    }
    else if (LeftAritySum == RightAritySum && LeftText > RightText)
    {
        SwapSides(); // Sort the two sides lexicographically.
    }
}

void CRelation::RearrangeNegations()
{
    mLeft.FactorOutNegations();
    mLeft.RemoveDoubleNegations();

    mRight.FactorOutNegations();
    mRight.RemoveDoubleNegations();
}

void CRelation::CancelOutNegationsOnBothSides()
{
    int Iteration = 0;
    int MaxIterations = std::min(mLeft.GetOperationCount(), mRight.GetOperationCount());
    while (Iteration++ < MaxIterations)
    {
        if (!mLeft.mOperations.back().IsBooleanNOT())
        {
            break;
        }
        if (!mRight.mOperations.back().IsBooleanNOT())
        {
            break;
        }

        // If the last operation of both sides are a negation, cancel them out.
        mLeft.EraseLastOperator();
        mRight.EraseLastOperator();
    }
}

void CRelation::ConvertNegationToInequation()
{
    // x≠y, x==¬y, and ¬x==y are equivalent for Boolean expressions. 
    // To make implementing the comparison simpler, transform them into the same form.
    if (mComparer == EComparer::AlmostEqual)
    {
        if (!mLeft.IsEmpty() && mLeft.mOperations.back().IsBooleanNOT())
        {
            mLeft.EraseLastOperator();
            mComparer = EComparer::NotAlmostEqual;
        }
        else if (!mRight.IsEmpty() && mRight.mOperations.back().IsBooleanNOT())
        {
            mRight.EraseLastOperator();
            mComparer = EComparer::NotAlmostEqual;
        }
    }
}

void CRelation::SortComparands()
{
    std::vector<SIndexedSizeAndText> ComparandTuples;
    ComparandTuples.reserve(2);
    ComparandTuples.emplace_back(0, mLeft.AddUpArity(), mLeft.ToString());
    ComparandTuples.emplace_back(1, mRight.AddUpArity(), mRight.ToString());

    std::ranges::sort(ComparandTuples, &SIndexedSizeAndText::Compare);

    if (ComparandTuples[0].Index == 1)
    {
        SwapSides();
    }
}

void CRelation::Negate()
{
    mComparer.Negate();
}

void CRelation::SwapSides()
{
    mComparer.Reverse();
    std::swap(mLeft, mRight);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
