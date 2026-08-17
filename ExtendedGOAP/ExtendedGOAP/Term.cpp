// Copyright 2026 Isaac Hsu

#include <cassert>
#include <map>

#include "BitVector.h"
#include "Fact.h"
#include "Matrix.h"
#include "Set.h"
#include "Term.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool STerm::operator == (const STerm& Another) const
{
    if (Coefficient != Another.Coefficient)
    {
        return false;
    }

    return Expression.IsEqual(Another.Expression);
}

CExpression STerm::ToExpression() const
{
    if (Coefficient == 0)
    {
        return CExpression(0);
    }
    else if (Coefficient == 1)
    {
        return Expression;
    }
    else if (Coefficient.IsNaN())
    {
        return CExpression(Coefficient);
    }
    else
    {
        CExpression Return = Expression;
        EOperator Operator = Expression.ReturnMatrix() ? EOperator::scalarMultiplication : EOperator::multiplication;
        if (Expression.IsOnlyOneOperand())
        {
            SOperation& Op = Return.mOperations.front();
            Op.Operator = Operator;
            Op.Left = Coefficient;
        }
        else
        {
            Return.Append(Operator, Coefficient);
        }

        return Return;
    }
}

CNumber STerm::SplitInto(std::vector<STerm>& oTerms) const
{
    return Expression.SplitInto(oTerms, Coefficient);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CTermList::CTermList(CNumber Constant)
{
    mConstantTerm = Constant;
}

CTermList::CTermList(const CExpression& Expression)
{
    if (!Expression.IsEmpty())
    {
        mTerms.emplace_back(Expression);
    }
}

CTermList CTermList::operator + (const CTermList& Right) const
{
    CTermList Return(*this);
    Return.mConstantTerm += Right.mConstantTerm;
    Return.mTerms.insert(Return.mTerms.end(), Right.mTerms.begin(), Right.mTerms.end());
    return Return;
}

CTermList CTermList::operator - (const CTermList& Right) const
{
    CTermList Return(*this);
    Return.mConstantTerm -= Right.mConstantTerm;

    Return.mTerms.reserve(Return.mTerms.size() + Right.mTerms.size());
    for (const STerm& RightTerm : Right.mTerms)
    {
        STerm& NewTerm = Return.mTerms.emplace_back(RightTerm);
        NewTerm.Coefficient *= -1;
    }

    return Return;
}

CTermList CTermList::operator * (CNumber Right) const
{
    if (Right == 0)
    {
        return CTermList(0);
    }

    CTermList Return(*this);
    Return.mConstantTerm *= Right;

    for (STerm& Term : Return.mTerms)
    {
        Term.Coefficient *= Right;
    }

    return Return;
}

bool CTermList::IsEqual(const CTermList& Another) const
{
    if (mConstantTerm != Another.mConstantTerm)
    {
        if (!mConstantTerm.IsNull() || !Another.mConstantTerm.IsNull()) // Check if either is set.
        {
            return false;
        }
    }

    return mTerms == Another.mTerms;
}

bool CTermList::IsEquivalent(const CTermList& Another) const
{
    return ToString() == Another.ToString();
}

CNumber CTermList::GetFirstCoefficient() const
{
    if (mTerms.empty())
    {
        return CNumber::Null;
    }

    return mTerms.front().Coefficient;
}

CNumber CTermList::GetUsableConstantTerm() const 
{ 
    if (mConstantTerm.IsNull())
    {
        return 0;
    }
    else if (mConstantTerm == static_cast<CNumber::BValue>(-0.0))
    {
        return 0;
    }
    else
    {
        return mConstantTerm;
    }
}

CBitVector CTermList::GetUsedFactBits() const
{
    CBitVector Return;

    for (const STerm& Term : mTerms)
    {
        Return |= Term.Expression.GetUsedFactBits();
    }

    return Return;
}

CVariant CTermList::Evaluate(const CStateBase& State) const
{
    CVariant Return;

    if (!mTerms.empty())
    {
        for (int TermIndex = 0; TermIndex < mTerms.size(); TermIndex++)
        {
            const STerm& Term = mTerms[TermIndex];
            CVariant Result = Term.Expression.Evaluate(State);
            switch (Result.GetVariantType())
            {
            case EVariantType::unset:
                return Result;

            case EVariantType::scalar:
                if (Return.IsUnset())
                {
                    Return = Result.GrabScalar() * Term.Coefficient;
                }
                else
                {
                    Return.GrabScalar() += Result.GrabScalar() * Term.Coefficient;
                }
                break;

            case EVariantType::matrix:
                if (Return.IsUnset())
                {
                    Return = Result.GrabMatrix() * Term.Coefficient;
                }
                else
                {
                    Return.GrabMatrix() += Result.GrabMatrix() * Term.Coefficient;
                }
                break;

            case EVariantType::set:
                assert(TermIndex == 0); // Only one term is allowed in set expressions since there is no addition.
                assert(Term.Coefficient == 1);
                Return = std::move(Result.GrabSet());
                break;

            default:                    
                assert(!"Invalid variant type"); 
                break;
            }
        }
    }

    if (!mConstantTerm.IsNull())
    {
        switch (Return.GetVariantType())
        {
        case EVariantType::unset:   Return = mConstantTerm; break;
        case EVariantType::scalar:  Return.GrabScalar() += mConstantTerm; break;
        case EVariantType::matrix:  assert(!"Matrix expression with a scalar constant term"); break;
        case EVariantType::set:     assert(!"Set expression with a scalar constant term"); break;
        default:                    assert(!"Invalid variant type"); break;
        }
    }

    return Return;
}

CExpression CTermList::ToExpression() const
{
    CExpression Return;

    if (!mTerms.empty())
    {
        const STerm& FirstTerm = mTerms.front();
        Return = FirstTerm.Expression;
        if (FirstTerm.Coefficient != 1)
        {
            EOperator Operator = FirstTerm.Expression.ReturnMatrix() ? EOperator::scalarMultiplication : EOperator::multiplication;
            Return.Prepend(Operator, FirstTerm.Coefficient);
        }

        for (int i = 1; i < mTerms.size(); i++)
        {
            const STerm& Term = mTerms[i];
            assert(Term.Coefficient.IsFinite());

            if (Term.Coefficient == 0)
            {
                continue;
            }

            bool IsMatrix = Term.Expression.ReturnMatrix();
            if (Term.Coefficient == 1)
            {
                Return.Append(IsMatrix ? EOperator::matrixAddition : EOperator::addition, Term.Expression);
            }
            else if (Term.Coefficient == -1)
            {
                Return.Append(IsMatrix ? EOperator::matrixSubtraction : EOperator::subtraction, Term.Expression);
            }
            else
            {
                CExpression Product(IsMatrix ? EOperator::scalarMultiplication : EOperator::multiplication, std::abs(Term.Coefficient), Term.Expression);
                EOperator Operator(Term.Coefficient.Get() >= 0 ? 
                    (IsMatrix ? EOperator::matrixAddition : EOperator::addition) :
                    (IsMatrix ? EOperator::matrixSubtraction : EOperator::subtraction));
                Return.Append(Operator, Product);
            }
        }
    }

    if (Return.IsEmpty())
    {
        if (!mConstantTerm.IsNull())
        {
            Return.SetTo(mConstantTerm);
        }
    }
    else if (!IsConstantTermEmpty())
    {
        assert(Return.ReturnScalar());

        EOperator Operator(mConstantTerm.Get() >= 0 ? EOperator::addition : EOperator::subtraction);
        Return.Append(Operator, std::abs(mConstantTerm));
    }

    return Return;
}

void CTermList::ReplaceFact(const CNumericFact& Fact, const COperand& Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CTermList::ReplaceFact(const CNumericFact& Fact, const CExpression& Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CTermList::ReplaceFact(int FactIndex, const COperand& Replacement)
{
    for (STerm& Term : mTerms)
    {
        Term.Expression.ReplaceFact(FactIndex, Replacement);
    }
}

void CTermList::ReplaceFact(int FactIndex, const CExpression& Replacement)
{
    for (STerm& Term : mTerms)
    {
        Term.Expression.ReplaceFact(FactIndex, Replacement);
    }
}

CTermList CTermList::GetRearranged() const
{
    CTermList Return = *this;
    Return.Rearrange();
    return Return;
}

void CTermList::Rearrange()
{
    if (IsEmpty())
    {
        return; // Early return to preserve empty terms.
    }

    // Make sure to include a constant term of 0 in case everything cancels out, since these terms were not originally empty. 
    CNullStableNumber MergedConstant = GetUsableConstantTerm();
    std::vector<STerm> Terms;

    for (STerm& Term : mTerms)
    {
        if (Term.Coefficient == 0)
        {
            continue;
        }

        Term.Expression.Rearrange();
        MergedConstant += Term.SplitInto(Terms); // The return value may be null, so CNullStableNumber::operator += is used.
    }

    mTerms = std::move(Terms);
    mConstantTerm = MergedConstant;

    CombineLikeTermsAndSortTerms();
}

void CTermList::CombineLikeTermsAndSortTerms()
{
    // TODO: We should combine not only outer like terms, but also inner like terms, e.g., (x+2x) % y = 3x % y, 
    // because equivalence comparison is lexicographical (see IsEquivalent).    
    
    std::map<std::string, STerm> TermMap; // To rearrange terms in a fixed order, use map instead of unordered_map.

    for (STerm& Term : mTerms)
    {
        // Two expressions may have different orders of operations and references but produce the same text.
        // Hence, their texts are used as the map's keys.
        std::string Text = Term.Expression.ToString();
        auto it = TermMap.find(Text);
        if (it == TermMap.end())
        {
            TermMap.emplace(Text, std::move(Term));
        }
        else
        {
            it->second.Coefficient += Term.Coefficient;
        }
    }

    std::vector<STerm> Terms;
    for (auto& [Text, Term] : TermMap)
    {
        if (Term.Coefficient == 0)
        {
            continue;
        }

        Terms.emplace_back(std::move(Term));
    }

    mTerms = std::move(Terms);
}

bool CTermList::PositivizeFirstCoefficient()
{
    CNumber FirstCoefficient = GetFirstCoefficient();
    if (FirstCoefficient.Get() >= 0)
    {
        return false;
    }
    else if (!FirstCoefficient.IsFinite())
    {
        return false;
    }

    CNumber Multiplier = -1;
    for (int i = 0; i < mTerms.size(); i++)
    {
        mTerms[i].Coefficient *= Multiplier;
    }

    mConstantTerm *= Multiplier;
    return true;
}

CNumber CTermList::NormalizeFirstCoefficientAndScaleOthers()
{
    CNumber FirstCoefficient = GetFirstCoefficient();
    if (FirstCoefficient == 0)
    {
        return CNumber::Null;
    }
    else if (FirstCoefficient == 1)
    {
        return CNumber::Null;
    }
    else if (!FirstCoefficient.IsFinite())
    {
        return CNumber::Null;
    }

    CNumber Multiplier = 1 / FirstCoefficient;
    mTerms.front().Coefficient = 1;

    for (int i = 1; i < mTerms.size(); i++)
    {
        mTerms[i].Coefficient *= Multiplier;
    }

    mConstantTerm *= Multiplier;
    return Multiplier;
}

const char* CTermList::GetCoefficientTimesSymbol()
{
#ifdef USE_UNICODE_SYMBOLS
    return "·";
#else
    return "*";
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////