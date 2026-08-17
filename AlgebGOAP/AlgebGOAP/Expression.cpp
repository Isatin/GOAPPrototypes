// Copyright 2025 Isaac Hsu

#include <algorithm>
#include <cassert>

#include "BitVector.h"
#include "Expression.h"
#include "Fact.h"
#include "State.h"
#include "Term.h"
#include "Utility.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
struct CExpression::SRepeatedOperation
{
    EOperator Operator;
    int SourceOperationIndex = InvalidIndex;    // Operation index of the source expression from which these repeated operations are tracked 
    CNumber Constatnt = CNumber::Null;          // Combined constant from these repeated operations
    std::vector<int> FactIndexes;               // Indexes of referenced facts 
    std::vector<int> OperationIndexes;          // Indexes of referenced operations

public:
    static bool IsSourceOperationIndexGreaterThanAnother(const SRepeatedOperation& Left, const SRepeatedOperation& Right);

    CExpression ToExpression() const;
    void UpdateOperationIndexes(const std::unordered_map<int, int>& OperationIndexMap);
};

CExpression CExpression::SRepeatedOperation::ToExpression() const
{
    return CExpression(Operator, FactIndexes, OperationIndexes, Constatnt, SourceOperationIndex);
}

bool CExpression::SRepeatedOperation::IsSourceOperationIndexGreaterThanAnother(const SRepeatedOperation& Left, const SRepeatedOperation& Right)
{
    return Left.SourceOperationIndex > Right.SourceOperationIndex;
}

void CExpression::SRepeatedOperation::UpdateOperationIndexes(const std::unordered_map<int, int>& OperationIndexMap)
{
    if (OperationIndexMap.empty())
    {
        return;
    }

    for (int& OpIndex : OperationIndexes)
    {
        auto it = OperationIndexMap.find(OpIndex);
        if (it != OperationIndexMap.end())
        {
            OpIndex = it->second;
        }
    }

    auto it = OperationIndexMap.find(SourceOperationIndex);
    if (it != OperationIndexMap.end())
    {
        SourceOperationIndex = it->second;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
struct CExpression::SSummation
{
    int SourceOperationIndex = InvalidIndex;        // Operation index of the source expression from which this summation is tracked
    CNumber Sum = 0;                                // Combined constant from this summation
    std::vector<int> AddendFactIndexes;             // Fact indexes of the addends 
    std::vector<int> AddendOperationIndexes;        // Operations indexes of the addends
    std::vector<int> SubtrahendFactIndexes;         // Fact indexes of the subtrahends 
    std::vector<int> SubtrahendOperationIndexes;    // Operations indexes of the subtrahends

public:
    static bool IsSourceOperationIndexGreaterThanAnother(const SSummation& Left, const SSummation& Right);

    CExpression ToExpression() const;
    void UpdateOperationIndexes(const std::unordered_map<int, int>& OperationIndexMap);
};

CExpression CExpression::SSummation::ToExpression() const
{
    CExpression Opposite;
    for (int i = 0; i < SubtrahendFactIndexes.size(); i++)
    {
        Opposite.mOperations.emplace_back(EOperator::minus, SFactIndex(SubtrahendFactIndexes[i]));
    }
    for (int i = 0; i < SubtrahendOperationIndexes.size(); i++)
    {
        Opposite.mOperations.emplace_back(EOperator::minus, SOperationIndex(SubtrahendOperationIndexes[i]));
    }

    CExpression OppositeSum;
    if (Opposite.mOperations.size() > 1)
    {
        OppositeSum.mOperations.emplace_back(EOperator::addition, 
            SOperationIndex(SourceOperationIndex), 
            SOperationIndex(SourceOperationIndex + 1));

        int NegationSumStartOpIndex = SourceOperationIndex + static_cast<int>(Opposite.mOperations.size());
        for (int OpIndex = 2; OpIndex < Opposite.mOperations.size(); OpIndex++)
        {
            OppositeSum.mOperations.emplace_back(EOperator::addition,
                SOperationIndex(NegationSumStartOpIndex + OppositeSum.mOperations.size() - 1),
                SOperationIndex(SourceOperationIndex + OpIndex));
        }
    } 

    CExpression Return = std::move(Opposite);
    std::ranges::move(OppositeSum.mOperations, std::back_inserter(Return.mOperations));

    int AdditionStartOpIndex = SourceOperationIndex + static_cast<int>(Return.mOperations.size());
    CExpression Addition(EOperator::addition, AddendFactIndexes, AddendOperationIndexes, Sum, AdditionStartOpIndex);
    if (!Addition.IsEmpty() && !Addition.IsConstant(0))
    {
        if (Return.IsEmpty())
        {
            Return = std::move(Addition);
        }
        else
        {
            std::ranges::move(Addition.mOperations, std::back_inserter(Return.mOperations));

            // Finally, sum the addends and the opposites of the subtrahends.
            int NegationSumLastOpIndex = AdditionStartOpIndex - 1;
            if (Return.GrabLastOperation().IsOnlyOneOperand())
            {
                Return.mOperations.back().Operator = EOperator::addition;
                Return.mOperations.back().Left = SOperationIndex(NegationSumLastOpIndex);
            }
            else
            {
                Return.mOperations.emplace_back(EOperator::addition,
                    SOperationIndex(SourceOperationIndex + Return.mOperations.size() - 1),
                    SOperationIndex(NegationSumLastOpIndex));
            }
        }
    }

    return Return;
}

bool CExpression::SSummation::IsSourceOperationIndexGreaterThanAnother(const SSummation& Left, const SSummation& Right)
{
    return Left.SourceOperationIndex > Right.SourceOperationIndex;
}

void CExpression::SSummation::UpdateOperationIndexes(const std::unordered_map<int, int>& OperationIndexMap)
{
    if (OperationIndexMap.empty())
    {
        return;
    }

    for (int& OpIndex : AddendOperationIndexes)
    {
        auto it = OperationIndexMap.find(OpIndex);
        if (it != OperationIndexMap.end())
        {
            OpIndex = it->second;
        }
    }

    for (int& OpIndex : SubtrahendOperationIndexes)
    {
        auto it = OperationIndexMap.find(OpIndex);
        if (it != OperationIndexMap.end())
        {
            OpIndex = it->second;
        }
    }

    auto it = OperationIndexMap.find(SourceOperationIndex);
    if (it != OperationIndexMap.end())
    {
        SourceOperationIndex = it->second;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
struct CExpression::SFraction
{
    int SourceOperationIndex = InvalidIndex;        // Operation index of the source expression from which this fraction is tracked
    CNumber Coefficient = 1;                        // Combined constant from this fraction
    std::vector<int> NumeratorFactIndexes;          // Fact indexes of the numerator 
    std::vector<int> NumeratorOperationIndexes;     // Operations indexes of the numerator
    std::vector<int> DenominatorFactIndexes;        // Fact indexes of the denominator 
    std::vector<int> DenominatorOperationIndexes;   // Operations indexes of the denominator

public:
    static bool IsSourceOperationIndexGreaterThanAnother(const SFraction& Left, const SFraction& Right);

    CExpression ToExpression() const;
    void UpdateOperationIndexes(const std::unordered_map<int, int>& OperationIndexMap);
};

CExpression CExpression::SFraction::ToExpression() const
{
    // Convert the denominator into multiplication by its reciprocal. 
    // Multiplication is commutative, allowing us to reorder factors for ease of equivalence comparison.

    CExpression Denominator(EOperator::multiplication, DenominatorFactIndexes, DenominatorOperationIndexes, CNumber::Null, SourceOperationIndex);
    int DenominatorOpCount = Denominator.GetOperationCount();
    if (DenominatorOpCount > 0)
    {
        if (Denominator.IsOnlyOneOperand())
        {
            Denominator.mOperations.back().Operator = EOperator::division;
            Denominator.mOperations.back().Left = 1;
        }
        else
        {
            Denominator.mOperations.emplace_back(EOperator::division, 1, SOperationIndex(SourceOperationIndex + DenominatorOpCount - 1));
            DenominatorOpCount++;
        }
    }

    CExpression Numerator(EOperator::multiplication, NumeratorFactIndexes, NumeratorOperationIndexes, Coefficient, SourceOperationIndex + DenominatorOpCount);
    if (Numerator.IsEmpty())
    {
        return {};
    }
    if (Numerator.IsConstant(1))
    {
        return Denominator;
    }

    CExpression Return = std::move(Denominator);
    bool IsNumeratorOnlyOneOperand = Numerator.IsOnlyOneOperand();
    std::ranges::move(Numerator.mOperations, std::back_inserter(Return.mOperations));

    if (DenominatorOpCount > 0)
    {
        if (IsNumeratorOnlyOneOperand)
        {
            Return.mOperations.back().Operator = EOperator::multiplication;
            Return.mOperations.back().Left = SOperationIndex(SourceOperationIndex + DenominatorOpCount - 1);
        }
        else
        {
            Return.mOperations.emplace_back(EOperator::multiplication,
                SOperationIndex(SourceOperationIndex + Return.mOperations.size() - 1),
                SOperationIndex(SourceOperationIndex + DenominatorOpCount - 1));
        }
    }

    return Return;
}

bool CExpression::SFraction::IsSourceOperationIndexGreaterThanAnother(const SFraction& Left, const SFraction& Right)
{
    return Left.SourceOperationIndex > Right.SourceOperationIndex;
}

void CExpression::SFraction::UpdateOperationIndexes(const std::unordered_map<int, int>& OperationIndexMap)
{
    if (OperationIndexMap.empty())
    {
        return;
    }

    for (int& OpIndex : NumeratorOperationIndexes)
    {
        auto it = OperationIndexMap.find(OpIndex);
        if (it != OperationIndexMap.end())
        {
            OpIndex = it->second;
        }
    }

    for (int& OpIndex : DenominatorOperationIndexes)
    {
        auto it = OperationIndexMap.find(OpIndex);
        if (it != OperationIndexMap.end())
        {
            OpIndex = it->second;
        }
    }

    auto it = OperationIndexMap.find(SourceOperationIndex);
    if (it != OperationIndexMap.end())
    {
        SourceOperationIndex = it->second;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const COperand COperand::Null;

COperand::COperand(SFactIndex Index)
    : mType(EOperand::fact)
    , mIndex(Index.Index)
{}

COperand::COperand(SOperationIndex Index)
    : mType(EOperand::operation)
    , mIndex(Index.Index)
{}

COperand::COperand(const CFact& Fact) 
    : mType(EOperand::fact)
    , mIndex(Fact.GetIndex()) 
{}

COperand::COperand(const CNumericFact& Fact)
    : mType(EOperand::fact)
    , mIndex(Fact.GetIndex())
{}

COperand::COperand(CNumber Value) 
    : mType(EOperand::constant)
    , mConstant(Value)
{}

bool COperand::operator == (const COperand& Another) const
{
    if (mType != Another.mType)
    {
        return false;
    }

    switch (mType)
    {
    case EOperand::null:        return true;
    case EOperand::fact:        return mIndex == Another.mIndex;
    case EOperand::operation:   return mIndex == Another.mIndex;
    case EOperand::constant:    return mConstant == Another.mConstant;
    }

    assert(!"Invalid operand type");
    return false;
}

int COperand::GrabOperationIndex() const 
{ 
    assert(IsOperation());

    return mIndex; 
}

int COperand::GrabFactIndex() const 
{ 
    assert(IsFact());

    return mIndex;
}

CNumber COperand::GrabConstant() const 
{ 
    assert(IsConstant());

    return mConstant;
}

void COperand::SetNull()
{
    mType = EOperand::null;
}

void COperand::SetFact(int FactIndex)
{
    mType = EOperand::fact;
    mIndex = FactIndex;
}

void COperand::SetOperation(int OperationIndex)
{
    mType = EOperand::operation;
    mIndex = OperationIndex;
}

void COperand::SetOperationIndex(int OperationIndex)
{
    assert(IsOperation());

    mIndex = OperationIndex;
}

void COperand::SetConstant(CNumber Value)
{
    mType = EOperand::constant;

    if (Value == static_cast<CNumber::BValue>(-0.0))
    {
        // -0 is printed with a minus sign and is different from positive zero.
        // However, printed expressions are compared lexicographically for equivalence comparison.
        // Hence, -0 is replaced with 0 here to avoid this issue.
        mConstant = 0;
    }
    else
    {
        mConstant = Value;
    }
}

void COperand::Set(const COperand& Operand)
{
    *this = Operand;

    if (IsConstant())
    {
        if (mConstant == static_cast<CNumber::BValue>(-0.0))
        {
            // -0 is printed with a minus sign and is different from positive zero.
            // However, printed expressions are compared lexicographically for equivalence comparison.
            // Hence, -0 is replaced with 0 here to avoid this issue.
            mConstant = 0;
        }
    }
}

void COperand::ShiftOperationIndex(int Offset)
{
    if (IsOperation())
    {
        mIndex += Offset;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool SOperation::operator == (const SOperation& Another) const
{
    return Operator == Another.Operator && Left == Another.Left && Right == Another.Right;
}

bool SOperation::IsOnlyOneFact() const
{
    return Operator.IsNil() && Right.IsFact();
}

int SOperation::GrabTheOnlyFactIndex() const
{
    assert(IsOnlyOneFact());

    return Right.GrabFactIndex();
}

bool SOperation::IsOnlyOneOperationReference() const
{
    return Operator.IsNil() && Right.IsOperation();
}

bool SOperation::IsOperation(int OperationIndex) const
{
    if (HasOperator())
    {
        return false;
    }

    return Right.IsOperation(OperationIndex);
}

bool SOperation::IsOnlyOneConstant() const
{
    return Operator.IsNil() && Right.IsConstant();
}

CNumber SOperation::GrabTheOnlyConstant() const
{
    assert(IsOnlyOneConstant());

    return Right.GrabConstant();
}

bool SOperation::IsOnlyOneOperand() const
{
    assert(!Right.IsNull());

    return !HasOperator();
}

void SOperation::MarkOperationFlags(std::vector<bool>& oFlags)
{
    if (Left.IsOperation())
    {
        oFlags[Left.GrabOperationIndex()] = true;
    }
    if (Right.IsOperation())
    {
        oFlags[Right.GrabOperationIndex()] = true;
    }
}

void SOperation::SetTo(const COperand& Operand)
{
    Operator.SetNil();
    Left.SetNull();
    Right.Set(Operand);
}

void SOperation::SetTo(CNumber Value)
{
    Operator.SetNil();
    Left.SetNull();
    Right.SetConstant(Value);
}

void SOperation::ShiftOperationIndex(int Offset)
{
    Left.ShiftOperationIndex(Offset);
    Right.ShiftOperationIndex(Offset);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const CExpression CExpression::Empty;

CExpression::CExpression(const COperand& Operand)
{
    assert(!Operand.IsNull());

    mOperations.emplace_back(EOperator::nil, COperand::Null, Operand);
}

CExpression::CExpression(EOperator Operator, const COperand& Operand)
    : CExpression(Operator, COperand::Null, Operand)
{}

CExpression::CExpression(EOperator Operator, const CExpression& Expression)
    : CExpression(Operator, COperand::Null, Expression)
{}

CExpression::CExpression(EOperator Operator, CExpression&& Expression)
    : CExpression(Operator, COperand::Null, std::move(Expression))
{}

CExpression::CExpression(EOperator Operator, const COperand& Left, const COperand& Right)
{
    assert((Operator.GetArity() == 2 && !Left.IsNull() && !Right.IsNull()) || (Operator.GetArity() == 1 && Left.IsNull() && !Right.IsNull()));

    mOperations.emplace_back(Operator, Left, Right);
}

CExpression::CExpression(EOperator Operator, const COperand& Left, const CExpression& Right)
    : CExpression(Operator, Left, CExpression(Right))
{}

CExpression::CExpression(EOperator Operator, const COperand& Left, CExpression&& Right)
    : mOperations(std::move(Right.mOperations))
{
    assert(!mOperations.empty());
    assert((Operator.GetArity() == 2 && !Left.IsNull()) || (Operator.GetArity() == 1 && Left.IsNull()));

    if (IsOnlyOneOperand())
    {
        mOperations.front().Operator = Operator;
        mOperations.front().Left = Left;
    }
    else
    {
        mOperations.emplace_back(Operator, Left, SOperationIndex(mOperations.size() - 1));
    }
}

CExpression::CExpression(EOperator Operator, const CExpression& Left, const COperand& Right)
    : CExpression(Operator, CExpression(Left), Right)
{}

CExpression::CExpression(EOperator Operator, CExpression&& Left, const COperand& Right)
    : mOperations(std::move(Left.mOperations))
{
    assert((Operator.GetArity() == 2 && !IsEmpty() && !Right.IsNull()) || (Operator.GetArity() == 1 && IsEmpty() && !Right.IsNull()));

    if (IsOnlyOneOperand())
    {
        SOperation& Op = mOperations.front();
        Op.Operator = Operator;
        Op.Left = Op.Right;
        Op.Right = Right;
    }
    else if (Operator.GetArity() == 1)
    {
        mOperations.emplace_back(Operator, Right);
    }
    else
    {
        mOperations.emplace_back(Operator, SOperationIndex(mOperations.size() - 1), Right);
    }
}

CExpression::CExpression(EOperator Operator, const CExpression& Left, const CExpression& Right)
    : mOperations(Left.mOperations)
{
    Append(Operator, Right);
}

CExpression::CExpression(EOperator Operator, CExpression&& Left, const CExpression& Right)
    : mOperations(std::move(Left.mOperations))
{
    Append(Operator, Right);
}

CExpression::CExpression(EOperator Operator, const CExpression& Left,  CExpression&& Right)
    : mOperations(std::move(Right.mOperations))
{
    Prepend(Operator, Left);
}

CExpression::CExpression(EOperator Operator, CExpression&& Left, CExpression&& Right)
    : mOperations(std::move(Left.mOperations))
{
    Append(Operator, std::move(Right));
}

CExpression::CExpression(EOperator Operator, const std::vector<int>& FactIndexes, const std::vector<int>& OperationIndexes, CNumber Constant, int StartOperationIndex)
{
    assert(Operator.GetArity() > 0);
    assert(Operator.GetArity() != 1 || FactIndexes.size() + OperationIndexes.size() + (Constant.IsNull() ? 0 : 1) <= 1);

    int IndexOfFactIndexes = 0;
    int IndexOfOpIndexes = 0;
    CNumber Value = (Constant == Operator.GetRightIdentity()) ? CNumber::Null : Constant;

    // Rearrange the operands in the following order: fact, operation, and constant.
    // Adding the constant last makes it the right operand of the last operation for ease of access.
    // Subexpressions are sorted in ascending order by the number of operands (see SortSubExpressions).
    // Facts are deemed unary operations without operators, so they are placed before operations.
    if (FactIndexes.size() >= 2)
    {
        mOperations.emplace_back(Operator,
            SFactIndex(FactIndexes[IndexOfFactIndexes]),
            SFactIndex(FactIndexes[IndexOfFactIndexes + 1]));
        IndexOfFactIndexes += 2;
    }
    else if (FactIndexes.size() == 1)
    {
        if (OperationIndexes.size() >= 1)
        {
            mOperations.emplace_back(Operator,
                SFactIndex(FactIndexes[IndexOfFactIndexes++]),
                SOperationIndex(OperationIndexes[IndexOfOpIndexes++]));
        }
        else if (!Value.IsNull())
        {
            mOperations.emplace_back(Operator, SFactIndex(FactIndexes[IndexOfFactIndexes++]), Value);
            Value = CNumber::Null;
        }
        else
        {
            mOperations.emplace_back(EOperator::nil, COperand::Null, SFactIndex(FactIndexes[IndexOfFactIndexes++]));
        }
    }
    else // If no fact references exist
    {
        if (OperationIndexes.size() >= 2)
        {
            mOperations.emplace_back(Operator,
                SOperationIndex(OperationIndexes[IndexOfOpIndexes]),
                SOperationIndex(OperationIndexes[IndexOfOpIndexes + 1]));
            IndexOfOpIndexes += 2;
        }
        else if (OperationIndexes.size() == 1)
        {
            if (!Value.IsNull())
            {
                mOperations.emplace_back(Operator, SOperationIndex(OperationIndexes[IndexOfOpIndexes++]), Value);
                Value = CNumber::Null;
            }
            else
            {
                mOperations.emplace_back(EOperator::nil, COperand::Null, SOperationIndex(OperationIndexes[IndexOfOpIndexes++]));
            }
        }
        else if (!Constant.IsNull()) // If only a constant is available
        {
            mOperations.emplace_back(EOperator::nil, COperand::Null, Constant);
            Value = CNumber::Null;
        }
    }

    for (; IndexOfFactIndexes < FactIndexes.size(); IndexOfFactIndexes++)
    {
        mOperations.emplace_back(Operator,
            SOperationIndex(StartOperationIndex + mOperations.size() - 1),
            SFactIndex(FactIndexes[IndexOfFactIndexes]));
    }

    for (; IndexOfOpIndexes < OperationIndexes.size(); IndexOfOpIndexes++)
    {
        mOperations.emplace_back(Operator,
            SOperationIndex(StartOperationIndex + mOperations.size() - 1),
            SOperationIndex(OperationIndexes[IndexOfOpIndexes]));
    }

    if (!Value.IsNull() && Value != Operator.GetRightIdentity())
    {
        mOperations.emplace_back(Operator,
            SOperationIndex(StartOperationIndex + mOperations.size() - 1),
            Value);
    }
}

std::string CExpression::ToString(const CFactDefinition& Definition) const
{
    std::vector<std::string> Texts(mOperations.size());

    auto StringizeOperand = [&](const COperand& Operand)
    {
        if (Operand.IsFact())
        {
            const CFact* Fact = Definition.GetFact(Operand.GrabFactIndex());
            return Fact->GetName();
        }
        else if (Operand.IsOperation())
        {
            const int RefOpIndex = Operand.GrabOperationIndex();
            if (mOperations[RefOpIndex].Operator.GetSymbol())
            {
                return Parenthesize(Texts[RefOpIndex]);
            }
            else 
            {
                return Texts[RefOpIndex]; // Operators without a symbol are printed with their function names and parentheses, so don't parenthesize them again.
            }
        }
        else if (Operand.IsConstant())
        {
            return Operand.GrabConstant().ToString();
        }

        return std::string();
    };

    for (int i = 0; i < mOperations.size(); i++)
    {
        const SOperation& Op = mOperations[i];
        std::string LeftString = StringizeOperand(Op.Left);
        std::string RightString = StringizeOperand(Op.Right);
        Texts[i] = Op.Operator.Stringize(LeftString, RightString);
    }

    return Texts.empty() ? GetEmptySymbol() : Texts.back();
}

std::string CExpression::ToString() const
{
    if (mOperations.empty())
    {
        return GetEmptySymbol();
    }
    else
    {
        return ToString(static_cast<int>(mOperations.size()) - 1);
    }
}

std::string CExpression::ToString(int StartOperationIndex) const
{
    std::vector<int> OpIndexes = GetUsedOperationIndexes(StartOperationIndex);
    std::ranges::sort(OpIndexes);
    return ToString(OpIndexes);
}

std::string CExpression::ToString(const std::vector<int>& OperationIndexes) const
{
    assert(std::ranges::is_sorted(OperationIndexes));

    if (OperationIndexes.empty())
    {
        return GetEmptySymbol();
    }

    std::vector<std::string> Texts(mOperations.size());

    auto StringizeOperand = [&](const COperand& Operand)
    {
        if (Operand.IsFact())
        {
            std::string Text("F");
            Text += std::to_string(Operand.GrabFactIndex());
            return Text;
        }
        else if (Operand.IsOperation())
        {
            const int RefOpIndex = Operand.GrabOperationIndex();
            if (mOperations[RefOpIndex].Operator.GetSymbol())
            {
                assert(!Texts[RefOpIndex].empty());
                return Parenthesize(Texts[RefOpIndex]);
            }
            else
            {
                assert(!Texts[RefOpIndex].empty());
                return Texts[RefOpIndex]; // Operators without a symbol are printed with their function names and parenthses, so needn't parenthesize them again.
            }
        }
        else if (Operand.IsConstant())
        {
            return Operand.GrabConstant().ToString();
        }

        return std::string();
    };

    for (int IndexOfOpIndexes = 0; IndexOfOpIndexes < OperationIndexes.size(); IndexOfOpIndexes++)
    {
        int OpIndex = OperationIndexes[IndexOfOpIndexes];
        const SOperation& Op = mOperations[OpIndex];
        std::string LeftString = StringizeOperand(Op.Left);
        std::string RightString = StringizeOperand(Op.Right);
        Texts[OpIndex] = Op.Operator.Stringize(LeftString, RightString);
    }

    return Texts[OperationIndexes.back()];
}

bool CExpression::IsEmpty() const
{ 
    return mOperations.empty();
}

bool CExpression::IsEqual(const CExpression& Another) const 
{ 
    return mOperations == Another.mOperations; 
}

bool CExpression::IsEquivalent(const CExpression& Another) const
{
    return ToString() == Another.ToString();
}

bool CExpression::IsOnlyOneFact() const
{
    return mOperations.size() == 1 && mOperations.front().IsOnlyOneFact();
}

int CExpression::GrabTheOnlyFactIndex() const
{
    assert(IsOnlyOneFact());

    return mOperations.front().GrabTheOnlyFactIndex();
}

bool CExpression::IsOnlyOneConstant() const
{
    return mOperations.size() == 1 && mOperations.front().IsOnlyOneConstant();
}

bool CExpression::IsInfinity() const
{
    if (!IsOnlyOneConstant())
    {
        return false;
    }

    CNumber Constant = GrabTheOnlyConstant();
    return Constant.IsInfinity();
}

bool CExpression::IsConstant(CNumber Value) const
{
    if (!IsOnlyOneConstant())
    {
        return false;
    }

    CNumber Constant = GrabTheOnlyConstant();
    return Constant == Value;
}

CNumber CExpression::GrabTheOnlyConstant() const
{
    assert(IsOnlyOneConstant());

    return mOperations.front().GrabTheOnlyConstant();
}

bool CExpression::IsOnlyOneOperand() const
{
    return mOperations.size() == 1 && mOperations.front().IsOnlyOneOperand();
}

const COperand& CExpression::GrabTheOnlyOperand() const
{
    assert(IsOnlyOneOperand());
    // Technically, the right operand could be null or an operation reference, but it shouldn't be.
    assert(mOperations.front().Right.IsFact() || mOperations.front().Right.IsConstant());

    return mOperations.front().Right;
}

bool CExpression::IsTransposable() const
{
    if (IsOnlyOneConstant())
    {
        // Infinity cannot be moved to the other side in an equation, otherwise ∞==∞ would be false.
        return !GrabTheOnlyConstant().IsInfinity();
    }
    else
    {
        return true;
    }
}

bool CExpression::ValidateOperation(const SOperation& Operation) const
{
    return ValidateOperation(Operation, static_cast<int>(mOperations.size()));
}

bool CExpression::ValidateOperation(const SOperation& Operation, int OperationLimit) const
{
    auto ValidateOperand = [OperationLimit](const COperand& Operand)
    {
        if (Operand.IsFact())
        {
            return Operand.GrabFactIndex() >= 0;
        }
        else if (Operand.IsOperation())
        {
            return Operand.GrabOperationIndex() >= 0 && Operand.GrabOperationIndex() < OperationLimit;
        }
        else if (Operand.IsConstant())
        {
            return true;
        }

        return false;
    };

    if (Operation.Operator.GetArity() > 1 && !ValidateOperand(Operation.Left))
    {
        return false;
    }

    if (!ValidateOperand(Operation.Right))
    {
        return false;
    }

    return true;
}

std::vector<int> CExpression::GetInvalidOperationIndexes() const
{
    std::vector<int> InvalidOpIndexes;
    InvalidOpIndexes.reserve(mOperations.size());

    for (int i = 0; i < mOperations.size(); i++)
    {
        const SOperation& Operation = mOperations[i];
        if (!ValidateOperation(mOperations[i], i))
        {
            InvalidOpIndexes.push_back(i);
        }
    }

    return InvalidOpIndexes;
}

bool CExpression::IsFactUsed(int FactIndex) const
{
    for (const SOperation& Op : mOperations)
    {
        if (Op.Left.IsFact(FactIndex))
        {
            return true;
        }
        if (Op.Right.IsFact(FactIndex))
        {
            return true;
        }
    }

    return false;
}

CBitVector CExpression::GetUsedFactBits() const
{
    CBitVector Return;

    for (const SOperation& Op : mOperations)
    {
        if (Op.Left.IsFact())
        {
            Return.GetOrAdd(Op.Left.mIndex) = true;
        }
        if (Op.Right.IsFact())
        {
            Return.GetOrAdd(Op.Right.mIndex) = true;
        }
    }

    return Return;
}

int CExpression::AddUpArity() const
{
    int Count = 0;

    for (const SOperation& Op : mOperations)
    {
        Count += Op.Operator.GetArity();
    }

    return Count;
}

int CExpression::AddUpArity(const std::vector<int>& OperationIndexes) const
{
    int Count = 0;

    for (int i = 0; i < OperationIndexes.size(); i++)
    {
        int OpIndex = OperationIndexes[i];
        const SOperation& Op = mOperations[OpIndex];
        Count += Op.Operator.GetArity();
    }

    return Count;
}

CNumber CExpression::GetConstantTerm() const
{
    if (mOperations.empty())
    {
        return CNumber::Null;
    }

    const SOperation& LastOperation = mOperations.back();
    if (!LastOperation.Right.IsConstant())
    {
        return CNumber::Null;
    }

    if (!LastOperation.HasOperator())
    {
        return LastOperation.Right.GrabConstant();
    }
    else if (LastOperation.IsAddition())
    {
        return LastOperation.Right.GrabConstant();
    }
    else if (LastOperation.IsSubtraction())
    {
        CNumber Value = LastOperation.Right.GrabConstant();
        if (Value.IsNull())
        {
            return CNumber::Null;
        }
        else
        {
            return -Value;
        }
    }

    return CNumber::Null;
}

int CExpression::CountReferencedUnivariateOperations() const
{
    if (mOperations.empty())
    {
        return 0;
    }

    int Count = 0;

    std::vector<int> UsedOpIndexes;
    UsedOpIndexes.reserve(mOperations.size());
    UsedOpIndexes.push_back(static_cast<int>(mOperations.size()) - 1);

    auto CountUnivariateOperation = [&](const COperand& Operand)
    {
        if (!Operand.IsOperation())
        {
            return;
        }

        const int RefOpIndex = Operand.GrabOperationIndex();
        if (std::ranges::find(UsedOpIndexes, RefOpIndex) != UsedOpIndexes.end())
        {
            return;
        }

        UsedOpIndexes.push_back(RefOpIndex);

        if (!mOperations[RefOpIndex].HasOperator())
        {
            Count++;
        }
    };

    for (int i = 0; i < UsedOpIndexes.size(); i++)
    {
        const int OpIndex = UsedOpIndexes[i];
        const SOperation& Op = mOperations[OpIndex];
        CountUnivariateOperation(Op.Left);
        CountUnivariateOperation(Op.Right);
    }

    return Count;
}

int CExpression::CountUnusedOperations() const
{
    std::vector<int> UsedOpIndexes = GetUsedOperationIndexes();
    int UnusedOpCount = static_cast<int>(mOperations.size()) - static_cast<int>(UsedOpIndexes.size());
    assert(UnusedOpCount >= 0);
    return UnusedOpCount;
}

std::vector<int> CExpression::GetUsedOperationIndexes() const
{
    if (mOperations.empty())
    {
        return {};
    }
    else
    {
        return GetUsedOperationIndexes(static_cast<int>(mOperations.size()) - 1);
    }
}

std::vector<int> CExpression::GetUsedOperationIndexes(int StartOperationIndex) const
{
    assert(StartOperationIndex >= 0 && StartOperationIndex < mOperations.size());

    // Record the operations directly or indirectly referenced by the given operation.
    std::vector<int> UsedOpIndexes;
    UsedOpIndexes.reserve(StartOperationIndex + 1);
    UsedOpIndexes.push_back(StartOperationIndex);
    for (int i = 0; i < UsedOpIndexes.size(); i++)
    {
        const int OpIndex = UsedOpIndexes[i];
        const SOperation& Op = mOperations[OpIndex];
        if (Op.Left.IsOperation())
        {
            if (std::ranges::find(UsedOpIndexes, Op.Left.GrabOperationIndex()) == UsedOpIndexes.end())
            {
                UsedOpIndexes.push_back(Op.Left.GrabOperationIndex());
            }
        }
        if (Op.Right.IsOperation())
        {
            if (std::ranges::find(UsedOpIndexes, Op.Right.GrabOperationIndex()) == UsedOpIndexes.end())
            {
                UsedOpIndexes.push_back(Op.Right.GrabOperationIndex());
            }            
        }
    }

    return UsedOpIndexes;
}

CExpression CExpression::GetSubExpression(int OperationIndex) const
{
    if (OperationIndex < 0 || OperationIndex >= mOperations.size())
    {
        return {};
    }

    CExpression Return;

    // Copy referenced operations and update their indexes.
    std::vector<int> UsedOpIndexes = GetUsedOperationIndexes(OperationIndex);
    std::ranges::sort(UsedOpIndexes);
    std::vector<int> NewOpIndexes(OperationIndex + 1, InvalidIndex);
    for (int i = 0; i < UsedOpIndexes.size(); i++)
    {
        int OpIndex = UsedOpIndexes[i];
        NewOpIndexes[OpIndex] = static_cast<int>(Return.mOperations.size());
        SOperation& NewOperation = Return.mOperations.emplace_back(mOperations[OpIndex]);

        if (NewOperation.Left.IsOperation())
        {
            int NewIndex = NewOpIndexes[NewOperation.Left.GrabOperationIndex()];
            assert(NewIndex >= 0);
            NewOperation.Left.SetOperation(NewIndex);
        }
        if (NewOperation.Right.IsOperation())
        {
            int NewIndex = NewOpIndexes[NewOperation.Right.GrabOperationIndex()];
            assert(NewIndex >= 0);
            NewOperation.Right.SetOperation(NewIndex);
        }
    }

    return Return;
}

CNumber CExpression::Evaluate(const CStateBase& State) const
{
    if (mOperations.empty())
    {
        return CNumber::Null;
    }

    std::vector<CNumber> Results(mOperations.size(), CNumber::Null);
    auto FetchValue = [&](const COperand& Operand)
    {
        switch (Operand.GetType())
        {
        case EOperand::null:        return CNumber::Null;
        case EOperand::fact:        return State.GetProperty(Operand.GrabFactIndex());
        case EOperand::operation:   return Results[Operand.GrabOperationIndex()];
        case EOperand::constant:    return Operand.GrabConstant();
        }

        assert(!"Invalid operand type");
        return CNumber::Null;
    };

    // Evaluate operations from first to last.
    for (int i = 0; i < mOperations.size(); i++)
    {
        const SOperation& Op = mOperations[i];

        CNumber LeftValue = CNumber::Null;
        if (Op.Operator.GetArity() > 1)
        {
            LeftValue = FetchValue(Op.Left);
            if (LeftValue.IsNull())
            {
                return CNumber::Null;
            }
        }

        CNumber RightValue = FetchValue(Op.Right);
        if (RightValue.IsNull())
        {
            return CNumber::Null;
        }

        Results[i] = Op.Operator(LeftValue, RightValue);
    }

    return Results.back();
}

void CExpression::SetTo(const COperand& Operand)
{
    assert(Operand.IsFact() || Operand.IsConstant());

    mOperations.resize(1);
    mOperations.back().SetTo(Operand);
}

void CExpression::Prepend(EOperator Operator)
{
    assert(!mOperations.empty());
    assert(Operator.GetArity() == 1);

    if (IsOnlyOneOperand())
    {
        mOperations.back().Operator = Operator;
    }
    else
    {
        mOperations.emplace_back(Operator, COperand::Null, SOperationIndex(mOperations.size() - 1));
    }
}

void CExpression::Prepend(EOperator Operator, const COperand& Left)
{
    assert(Operator.GetArity() == 2);
    assert(!mOperations.empty());
    assert(!Left.IsNull());

    if (IsOnlyOneOperand())
    {
        SOperation& Op = mOperations.back();
        Op.Operator = Operator;
        Op.Left = Left;
    }
    else
    {
        mOperations.emplace_back(Operator, Left, SOperationIndex(mOperations.size() - 1));
    }
}

void CExpression::Prepend(EOperator Operator, const CExpression& Left)
{
    assert(!Left.IsEmpty());

    if (IsEmpty())
    {
        assert(Operator.GetArity() == 1); // Verify that the operator is unary because there is only one operand.

        *this = Left;
        if (IsOnlyOneOperand())
        {
            mOperations.back().Operator = Operator;
        }
        else
        {
            mOperations.emplace_back(Operator, SOperationIndex(mOperations.size() - 1));
        }
    }
    else if (IsOnlyOneOperand())
    {
        COperand RightOperand = GrabTheOnlyOperand();

        if (Left.IsOnlyOneOperand())
        {
            COperand LeftOperand = Left.GrabTheOnlyOperand();
            SOperation& Op = mOperations.back();
            Op.Operator = Operator;
            Op.Left = LeftOperand;
            Op.Right = RightOperand;
        }
        else
        {
            *this = Left;
            mOperations.emplace_back(Operator, SOperationIndex(mOperations.size() - 1), RightOperand);
        }
    }
    else
    {
        const int RightSize = static_cast<int>(mOperations.size());
        if (Left.IsOnlyOneOperand())
        {
            mOperations.emplace_back(Operator, Left.GrabTheOnlyOperand(), SOperationIndex(RightSize - 1));
        }
        else
        {
            mOperations.insert(mOperations.end(), Left.mOperations.begin(), Left.mOperations.end());            
            ShiftOperationIndexes(RightSize, RightSize); // Adjust operation references for the operations from the left. 
            mOperations.emplace_back(Operator, SOperationIndex(mOperations.size() - 1), SOperationIndex(RightSize - 1));
        }
    }
}

void CExpression::Append(EOperator Operator, const COperand& Right)
{
    assert(Operator.GetArity() == 2);
    assert(!mOperations.empty());
    assert(!Right.IsNull());

    if (IsOnlyOneOperand())
    {
        COperand LeftOperand = GrabTheOnlyOperand();
        SOperation& Op = mOperations.back();
        Op.Operator = Operator;
        Op.Left = LeftOperand;
        Op.Right = Right;
    }
    else
    {
        mOperations.emplace_back(Operator, SOperationIndex(mOperations.size() - 1), Right);
    }
}

void CExpression::Append(EOperator Operator, const CExpression& Right)
{
    Append(Operator, CExpression(Right));
}

void CExpression::Append(EOperator Operator, CExpression&& Right)
{
    assert(!Right.IsEmpty());

    if (IsEmpty()) 
    {
        assert(Operator.GetArity() == 1); // Verify that the operator is unary because there is only one operand.

        *this = std::move(Right);
        if (IsOnlyOneOperand())
        {
            mOperations.back().Operator = Operator;
        }
        else
        {
            mOperations.emplace_back(Operator, SOperationIndex(mOperations.size() - 1));
        }
    }
    else if (IsOnlyOneOperand())
    {
        COperand LeftOperand = GrabTheOnlyOperand();

        if (Right.IsOnlyOneOperand())
        {
            COperand RightOperand = Right.GrabTheOnlyOperand();
            SOperation& Op = mOperations.back();
            Op.Operator = Operator;
            Op.Left = LeftOperand;
            Op.Right = RightOperand;
        }
        else
        {
            *this = std::move(Right);
            mOperations.emplace_back(Operator, LeftOperand, SOperationIndex(mOperations.size() - 1));
        }
    }
    else
    {
        const int LeftSize = static_cast<int>(mOperations.size());
        if (Right.IsOnlyOneOperand())
        {
            mOperations.emplace_back(Operator, SOperationIndex(LeftSize - 1), Right.GrabTheOnlyOperand());
        }
        else
        {
            std::ranges::move(Right.mOperations, std::back_inserter(mOperations));
            ShiftOperationIndexes(LeftSize, LeftSize); // Adjust operation references for the operations from the right.
            mOperations.emplace_back(Operator, SOperationIndex(LeftSize - 1), SOperationIndex(mOperations.size() - 1));
        }
    }
}

void CExpression::ReplaceFact(const CNumericFact& Fact, CNumber Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CExpression::ReplaceFact(const CNumericFact& Fact, const CNumericFact& Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CExpression::ReplaceFact(const CFact& Fact, const COperand& Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CExpression::ReplaceFact(int FactIndex, const COperand& Replacement)
{
    assert(!Replacement.IsNull());
    assert(!Replacement.IsOperation());

    for (SOperation& Op : mOperations)
    {
        if (Op.Left.IsFact(FactIndex))
        {
            Op.Left = Replacement;
        }
        if (Op.Right.IsFact(FactIndex))
        {
            Op.Right = Replacement;
        }
    }
}

void CExpression::ReplaceFact(const CNumericFact& Fact, const CExpression& Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CExpression::ReplaceFact(const CFact& Fact, const CExpression& Replacement) 
{ 
    ReplaceFact(Fact.GetIndex(), Replacement); 
}

void CExpression::ReplaceFact(int FactIndex, const CExpression& Replacement)
{
    if (!IsFactUsed(FactIndex))
    {
        return;
    }
    if (Replacement.IsEmpty())
    {
        return;
    }

    if (Replacement.IsOnlyOneOperand())
    {
        // Replace the given fact with another or a constant.
        ReplaceFact(FactIndex, Replacement.GrabTheOnlyOperand());
        return;
    }

    if (IsOnlyOneFact())
    {
        // Just replace this expression since the only fact will be overwritten after the replacement.
        mOperations = Replacement.mOperations;
        return;
    }

    const int ReplacementSize = Replacement.GetOperationCount();
    auto ShiftOrReplace = [=](COperand& Operand)
    {
        switch (Operand.mType)
        {
        case EOperand::fact:
            if (Operand.mIndex == FactIndex)
            {
                Operand.SetOperation(ReplacementSize - 1);
            }
            break;
        
        case EOperand::operation:
            Operand.mIndex += ReplacementSize;
            break;
        }
    };

    for (SOperation& Op : mOperations)
    {
        ShiftOrReplace(Op.Left);
        ShiftOrReplace(Op.Right);
    }

    mOperations.insert(mOperations.begin(), Replacement.mOperations.begin(), Replacement.mOperations.end());
}

std::unordered_map<int, int> CExpression::ReplaceOperation(int OperationIndex, CExpression&& Replacement)
{
    assert(OperationIndex >= 0 && OperationIndex < mOperations.size());    

    if (Replacement.IsEmpty())
    {
        return {};
    }

    // Check whether the first operation of the replacement is valid.
    assert(ValidateOperation(Replacement.mOperations.front()));
    // Replace the specified operation with the first operation of the replacement.
    mOperations[OperationIndex] = Replacement.mOperations.front();

    const int ReplacementOpCount = Replacement.GetOperationCount();
    if (ReplacementOpCount == 1)
    {
        return {};
    }

    // Insert the replacement operations except the first one.
    mOperations.insert(mOperations.begin() + OperationIndex + 1, Replacement.mOperations.begin() + 1, Replacement.mOperations.end());

    std::unordered_map<int, int> OpIndexMap;
    OpIndexMap.emplace(OperationIndex, OperationIndex + ReplacementOpCount - 1); // Map the replaced operation to the last operation of the inserted replacement.

    int Offset = ReplacementOpCount - 1;
    for (int i = OperationIndex + ReplacementOpCount; i < mOperations.size(); i++)
    {
        OpIndexMap.emplace(i - Offset, i);

        COperand& Left = mOperations[i].Left;
        COperand& Right = mOperations[i].Right;
        if (Left.IsOperation() && Left.GrabOperationIndex() >= OperationIndex)
        {
            Left.ShiftOperationIndex(Offset);
        }
        if (Right.IsOperation() && Right.GrabOperationIndex() >= OperationIndex)
        {
            Right.ShiftOperationIndex(Offset);
        }
    }

    return OpIndexMap;
}

int CExpression::RemoveOperation(int OperationIndex)
{
    return RemoveOperations(OperationIndex, OperationIndex);
}

int CExpression::RemoveOperations(int StartOperationIndex, int EndOperationIndex)
{
    if (StartOperationIndex < 0 || StartOperationIndex >= mOperations.size())
    {
        return 0;
    }
    if (EndOperationIndex < 0 || EndOperationIndex >= mOperations.size())
    {
        return 0;
    }
    if (StartOperationIndex > EndOperationIndex)
    {
        return 0;
    }

    mOperations.erase(mOperations.begin() + StartOperationIndex, mOperations.begin() + EndOperationIndex + 1);

    const int Delta = EndOperationIndex - StartOperationIndex + 1;
    for (int OpIndex = StartOperationIndex; OpIndex < mOperations.size(); OpIndex++)
    {
        SOperation& Op = mOperations[OpIndex];
        if (Op.Left.IsOperation())
        {
            Op.Left.mIndex -= Delta;
        }
        if (Op.Right.IsOperation())
        {
            Op.Right.mIndex -= Delta;
        }
    }

    return EndOperationIndex - StartOperationIndex + 1;
}

int CExpression::RemoveUnusedOperations()
{
    if (mOperations.empty())
    {
        return 0;
    }

    std::vector<int> NewOpIndexes(mOperations.size());
    for (int i = 0; i < NewOpIndexes.size(); i++)
    {
        NewOpIndexes[i] = i;
    }

    int RemovalCount = 0;
    std::vector<bool> OpsUsed(mOperations.size(), false);
    OpsUsed.back() = true;
    for (int OpIndex = static_cast<int>(mOperations.size()) - 1; OpIndex >= 0; OpIndex--)
    {
        if (OpsUsed[OpIndex])
        {
            mOperations[OpIndex].MarkOperationFlags(OpsUsed);
        }
        else
        {
            mOperations.erase(mOperations.begin() + OpIndex);
            RemovalCount++;

            // The current operation will be deleted, so subsequent operation indexes need to be updated.
            for (int IndexOfOpIndexes = OpIndex + 1; IndexOfOpIndexes < NewOpIndexes.size(); IndexOfOpIndexes++)
            {
                NewOpIndexes[IndexOfOpIndexes]--;
            }
        }
    }

    for (int OpIndex = 0; OpIndex < mOperations.size(); OpIndex++)
    {
        SOperation& Op = mOperations[OpIndex];
        if (Op.Left.IsOperation())
        {
            int NewIndex = NewOpIndexes[Op.Left.GrabOperationIndex()];
            Op.Left.SetOperationIndex(NewIndex);
        }
        if (Op.Right.IsOperation())
        {
            int NewIndex = NewOpIndexes[Op.Right.GrabOperationIndex()];
            Op.Right.SetOperationIndex(NewIndex);
        }
    }

    // Delete the last operation if it simply references the penultimate one.
    if (mOperations.back().IsOperation(GetOperationCount() - 2))
    {
        mOperations.erase(mOperations.end() - 1);
        RemovalCount++;
    }

    return RemovalCount;
}

CExpression CExpression::GetRearranged() const
{
    CExpression Return = *this;
    Return.Rearrange();
    return Return;
}

void CExpression::Rearrange()
{
    ApplyDeMorgansLaws();
    RearrangeBooleanOperations();
    DistributeOperationsOverOthers();
    RemoveIneffectiveOperations();
    RemoveIdentityElements();
    RearrangeFractions();
    RearrangeSubtractions();
    RemoveConstantOperations();
    RearrangeCommutativeOperations();
    RemoveUnusedOperations();
    
    assert(GetInvalidOperationIndexes().empty());
    assert(CountUnusedOperations() == 0);
    assert(CountReferencedUnivariateOperations() == 0);
}

void CExpression::ApplyDeMorgansLaws()
{
    // TODO: Rework this implementation without adding unnecessary operations.

    if (mOperations.empty())
    {
        return;
    }

    std::vector<bool> OpsUsed(mOperations.size(), false);
    OpsUsed.back() = true;
    for (int OpIndex = static_cast<int>(mOperations.size()) - 1; OpIndex >= 0; OpIndex--)
    {
        if (!OpsUsed[OpIndex])
        {
            continue;
        }
        
        SOperation& Op = mOperations[OpIndex];
        if (!Op.IsBooleanNOT())
        {
            Op.MarkOperationFlags(OpsUsed);
            continue;
        }

        // Boolean NOT is unary, so we need to check only the right operand.
        if (!Op.Right.IsOperation())
        {
            continue;
        }

        int OpIndexOffset = 0;
        const int RefOpIndex = Op.Right.GrabOperationIndex();
        const SOperation& RefOp = mOperations[RefOpIndex];

        CExpression Replacement;
        SOperation& ReplacementOp = Replacement.mOperations.emplace_back(RefOp);
        if (!ReplacementOp.HasOperator())
        {
            ReplacementOp.Operator = EOperator::booleanNOT;
        }
        else if (ReplacementOp.IsBooleanNOT())
        {
            ReplacementOp.Operator.SetNil(); // Cancel out the double negation.
        }
        else if (ReplacementOp.IsBooleanAND())
        {
            ReplacementOp.Operator = EOperator::booleanNOT;
            ReplacementOp.Left.SetNull();
            Replacement.mOperations.emplace_back(EOperator::booleanNOT, RefOp.Left);
            Replacement.mOperations.emplace_back(EOperator::booleanOR, SOperationIndex(OpIndex + 1), SOperationIndex(OpIndex));
            OpIndexOffset = 2;
        }
        else if (ReplacementOp.IsBooleanOR())
        {
            ReplacementOp.Operator = EOperator::booleanNOT;
            ReplacementOp.Left.SetNull();
            Replacement.mOperations.emplace_back(EOperator::booleanNOT, RefOp.Left);
            Replacement.mOperations.emplace_back(EOperator::booleanAND, SOperationIndex(OpIndex + 1), SOperationIndex(OpIndex));
            OpIndexOffset = 2;
        }
        else
        {
            // Boolean operations can be used only with other Boolean operations, not with different types of operations.
            assert(!RefOp.Operator.IsBoolean());
            assert(!"Missing implementation for a Boolean operator");
        }

        auto OpIndexMap = ReplaceOperation(OpIndex, std::move(Replacement));

        if (OpIndexOffset > 0)
        {
            OpsUsed.insert(OpsUsed.begin() + OpIndex + 1, OpIndexOffset, true);
            OpIndex += OpIndexOffset;
        }

        mOperations[OpIndex].MarkOperationFlags(OpsUsed);
    }

    std::vector<int> NewOperationIndexes(mOperations.size());
    for (int OpIndex = 0; OpIndex < NewOperationIndexes.size(); OpIndex++)
    {
        NewOperationIndexes[OpIndex] = OpIndex;
    }

    for (int OpIndex = 0; OpIndex < NewOperationIndexes.size(); OpIndex++)
    {
        if (!OpsUsed[OpIndex])
        {
            // The current operation will be deleted, so subsequent operation indexes need to be updated.
            for (int i = OpIndex + 1; i < NewOperationIndexes.size(); i++)
            {
                NewOperationIndexes[i]--;
            }
        }
    }

    auto UpdateOperationIndex = [&](COperand& Operand)
    {
        if (Operand.IsOperation())
        {
            const int RefOpIndex = Operand.GrabOperationIndex();
            Operand.SetOperationIndex(NewOperationIndexes[RefOpIndex]);
        }
    };

    // Delete unused operations and update operation indexes accordingly.
    for (int OpIndex = static_cast<int>(mOperations.size()) - 1; OpIndex >= 0; OpIndex--)
    {
        if (OpsUsed[OpIndex])
        {
            UpdateOperationIndex(mOperations[OpIndex].Left);
            UpdateOperationIndex(mOperations[OpIndex].Right);
        }
        else
        {
            mOperations.erase(mOperations.begin() + OpIndex);
        }
    }
}

void CExpression::FactorOutNegations()
{
    // Apply the reverse of De Morgan's laws to factor out Boolean negation from terms combined with Boolean AND/OR.
    // !x&&!y ≡ !(x||y), !x&&y ≡ !(x||!y), x&&!y ≡ !(!x||y)
}

void CExpression::RemoveDoubleNegations()
{
    // Cancel out double negation: !!x ≡ x
}

void CExpression::RearrangeBooleanOperations()
{
    // TODO: Apply Boolean algebra laws to transform equivalent Boolean expressions into the same form,
    // such as the domination, idempotent, complement, and absorption laws.
}

void CExpression::Negate()
{
    if (mOperations.back().IsBooleanNOT())
    {
        EraseLastOperator();
    }
    else
    {
        Prepend(EOperator::booleanNOT);
    }
}

void CExpression::EraseLastOperator()
{
    assert(mOperations.back().Operator.GetArity() <= 1);

    SOperation& LastOp = mOperations.back();
    if (LastOp.Right.IsOperation(GetOperationCount() - 2))
    {
        mOperations.erase(mOperations.end() - 1);
    }
    else
    {
        LastOp.Operator.SetNil();
    }
}

void CExpression::DistributeOperationsOverOthers()
{
    // TODO: Multiplication distributes over addition and subtraction.
    // TODO: Division distributes over addition and subtraction in the numerator.
    // TODO: Maximum distributes over minimum.
    // TODO: Minimum distributes over maximum.
    // TODO: Boolean AND distributes over Boolean OR.
    // TODO: Boolean OR distributes over Boolean AND.
}

void CExpression::RearrangeFractions()
{
    // To unify equivalent operations between multiplication and division, division is replaced by multiplication by the reciprocal of the divisor.
    // Multiplication is chosen because it is commutative, whereas division is not.
    // Hence, we can sort the factors in the multiplication later in a certain order for ease of comparison.

    if (mOperations.empty())
    {
        return;
    }

    auto CollectFraction = [](SFraction& Fraction, const COperand& Operand, bool Reciprocal)
    {
        if (Operand.IsFact())
        {
            if (Reciprocal)
            {
                Fraction.DenominatorFactIndexes.push_back(Operand.GrabFactIndex());
            }
            else
            {
                Fraction.NumeratorFactIndexes.push_back(Operand.GrabFactIndex());
            }
        }
        else if (Operand.IsOperation())
        {
            if (Reciprocal)
            {
                Fraction.DenominatorOperationIndexes.push_back(Operand.GrabOperationIndex());
            }
            else
            {
                Fraction.NumeratorOperationIndexes.push_back(Operand.GrabOperationIndex());
            }
        }
        else if (Operand.IsConstant())
        {
            if (Reciprocal)
            {
                Fraction.Coefficient /= Operand.GrabConstant();
            }
            else
            {
                Fraction.Coefficient *= Operand.GrabConstant();
            }
        }
    };

    // First PASS: Collect factors of the algebraic fractions and separate them into numerators and denominators.
    std::vector<SFraction> Fractions;
    std::vector<bool> OpsUsed(mOperations.size(), false);
    OpsUsed.back() = true;
    for (int OpIndex = static_cast<int>(mOperations.size()) - 1; OpIndex >= 0; OpIndex--)
    {
        if (!OpsUsed[OpIndex])
        {
            continue;
        }

        SOperation& Op = mOperations[OpIndex];
        Op.MarkOperationFlags(OpsUsed);

        bool Reciprocal = false;
        if (Op.IsMultiplication())
        {
            Reciprocal = false;
        }
        else if (Op.IsDivision())
        {
            Reciprocal = true;
        }
        else
        {
            continue; // Skip if not a factor.
        }

        bool AlreadyInFraction = false;
        auto UpdateFraction = [&](SFraction& Fraction, std::vector<int>& OperationIndexes, bool LeftReciprocal, bool RightReciprocal)
        {
            assert(&OperationIndexes == &Fraction.NumeratorOperationIndexes || &OperationIndexes == &Fraction.DenominatorOperationIndexes);

            for (int OpIndexOfFraction = static_cast<int>(OperationIndexes.size()) - 1; OpIndexOfFraction >= 0; OpIndexOfFraction--)
            {
                if (OpIndex == OperationIndexes[OpIndexOfFraction]) // Check if this operation is referenced by the current fraction.
                {
                    OperationIndexes.erase(OperationIndexes.begin() + OpIndexOfFraction);
                    CollectFraction(Fraction, Op.Left, LeftReciprocal);
                    CollectFraction(Fraction, Op.Right, RightReciprocal);
                    AlreadyInFraction = true;
                }
            }
        };

        // Replace the current operation in fractions with its operands.
        for (int FractionIndex = 0; FractionIndex < Fractions.size(); FractionIndex++)
        {
            SFraction& Fraction = Fractions[FractionIndex];
            UpdateFraction(Fraction, Fraction.NumeratorOperationIndexes, false, Reciprocal);
            UpdateFraction(Fraction, Fraction.DenominatorOperationIndexes, true, !Reciprocal);
        }

        // Add a new fraction starting with the current operation if no corresponding one exists.
        if (!AlreadyInFraction && Op.IsDivision())
        {
            SFraction& Fraction = Fractions.emplace_back(OpIndex);
            CollectFraction(Fraction, Op.Left, false);
            CollectFraction(Fraction, Op.Right, true);
        }
    }

    // Second PASS:
    // 1. Replace the source operations with the corresponding fraction expressions and update operation indexes.
    // 2. Call RemoveUnusedOperations to clean up (later in Rearrange).
    assert(std::ranges::is_sorted(Fractions, SFraction::IsSourceOperationIndexGreaterThanAnother));
    for (int FractionIndex = static_cast<int>(Fractions.size()) - 1; FractionIndex >= 0; FractionIndex--)
    {
        SFraction& Fraction = Fractions[FractionIndex];
        CExpression FractionExpression = Fraction.ToExpression();
        auto OpIndexMap = ReplaceOperation(Fraction.SourceOperationIndex, std::move(FractionExpression));

        // Update only the fractions preceding the current one, since the others have been used.
        for (int i = FractionIndex - 1; i >= 0; i--)
        {
            Fractions[i].UpdateOperationIndexes(OpIndexMap);
        }
    }
}

void CExpression::RearrangeSubtractions()
{
    // TODO: Combine like terms as well.

    if (mOperations.empty())
    {
        return;
    }

    auto CollectSummation = [](SSummation& Summation, const COperand& Operand, bool Negative)
    {
        if (Operand.IsFact())
        {
            if (Negative)
            {
                Summation.SubtrahendFactIndexes.push_back(Operand.GrabFactIndex());
            }
            else
            {
                Summation.AddendFactIndexes.push_back(Operand.GrabFactIndex());
            }
        }
        else if (Operand.IsOperation())
        {
            if (Negative)
            {
                Summation.SubtrahendOperationIndexes.push_back(Operand.GrabOperationIndex());
            }
            else
            {
                Summation.AddendOperationIndexes.push_back(Operand.GrabOperationIndex());
            }
        }
        else if (Operand.IsConstant())
        {
            if (Negative)
            {
                Summation.Sum -= Operand.GrabConstant();
            }
            else
            {
                Summation.Sum += Operand.GrabConstant();
            }
        }
    };

    // First PASS: Collect summands of algebraic summations and separate them into addends and subtrahends.
    std::vector<SSummation> Summations;
    std::vector<bool> OpsUsed(mOperations.size(), false);
    OpsUsed.back() = true;
    for (int OpIndex = static_cast<int>(mOperations.size()) - 1; OpIndex >= 0; OpIndex--)
    {
        if (!OpsUsed[OpIndex])
        {
            continue;
        }

        SOperation& Op = mOperations[OpIndex];
        Op.MarkOperationFlags(OpsUsed);

        // Convert multiplication by -1 to unary minus.
        if (Op.IsMultiplication())
        {
            if (Op.Left.IsConstant(-1))
            {
                Op.Operator = EOperator::minus;
                Op.Left.SetNull();
            }
            else if (Op.Right.IsConstant(-1))
            {
                Op.Operator = EOperator::minus;
                Op.Right = Op.Left;
                Op.Left.SetNull();
            }
        }

        bool Negative = false;
        if (Op.IsAddition())
        {
            Negative = false;
        }
        else if (Op.IsSubtraction())
        {
            Negative = true;
        }
        else if (Op.IsMinus())
        {
            Negative = true;
        }
        else
        {
            continue; // Skip if not a summand.
        }

        bool AlreadyInSummation = false;
        auto UpdateSummation = [&](SSummation& Summation, std::vector<int>& OperationIndexes, bool LeftNegative, bool RightNegative)
        {
            assert(&OperationIndexes == &Summation.AddendOperationIndexes || &OperationIndexes == &Summation.SubtrahendOperationIndexes);

            for (int OpIndexOfSummation = static_cast<int>(OperationIndexes.size()) - 1; OpIndexOfSummation >= 0; OpIndexOfSummation--)
            {
                if (OpIndex == OperationIndexes[OpIndexOfSummation]) // Check if this operation is referenced by the current summation.
                {
                    OperationIndexes.erase(OperationIndexes.begin() + OpIndexOfSummation);
                    CollectSummation(Summation, Op.Left, LeftNegative);
                    CollectSummation(Summation, Op.Right, RightNegative);
                    AlreadyInSummation = true;
                }
            }
        };

        // Replace the current operation in summations with its operands.
        for (int SummationIndex = 0; SummationIndex < Summations.size(); SummationIndex++)
        {
            SSummation& Summation = Summations[SummationIndex];
            UpdateSummation(Summation, Summation.AddendOperationIndexes, false, Negative);
            UpdateSummation(Summation, Summation.SubtrahendOperationIndexes, true, !Negative);
        }

        // Add a new summation starting with the current operation if no corresponding one exists.
        if (!AlreadyInSummation)
        {
            if (Op.IsSubtraction())
            {
                SSummation& Summation = Summations.emplace_back(OpIndex);
                CollectSummation(Summation, Op.Left, false);
                CollectSummation(Summation, Op.Right, true);
            }
            else if (Op.IsMinus())
            {
                SSummation& Summation = Summations.emplace_back(OpIndex);
                CollectSummation(Summation, Op.Right, true);
            }
        }
    }

    // Second PASS:
    // 1. Replace the source operations with the corresponding summation expressions and update operation indexes.
    // 2. Call RemoveUnusedOperations to clean up (later in Rearrange).
    assert(std::ranges::is_sorted(Summations, SSummation::IsSourceOperationIndexGreaterThanAnother));
    for (int SummationIndex = static_cast<int>(Summations.size()) - 1; SummationIndex >= 0; SummationIndex--)
    {
        SSummation& Summation = Summations[SummationIndex];
        CExpression SummationExpression = Summation.ToExpression();
        auto OpIndexMap = ReplaceOperation(Summation.SourceOperationIndex, std::move(SummationExpression));

        // Update only the summations preceding the current one, since the others have been used.
        for (int i = SummationIndex - 1; i >= 0; i--)
        {
            Summations[i].UpdateOperationIndexes(OpIndexMap);
        }
    }
}

void CExpression::RearrangeCommutativeOperations()
{
    if (mOperations.empty())
    {
        return;
    }

    auto CollectRepeatedOperation = [](SRepeatedOperation& RepeatedOperation, const COperand& Operand)
    {
        if (Operand.IsFact())
        {
            RepeatedOperation.FactIndexes.push_back(Operand.GrabFactIndex());
        }
        else if (Operand.IsOperation())
        {
            RepeatedOperation.OperationIndexes.push_back(Operand.GrabOperationIndex());
        }
        else if (Operand.IsConstant())
        {
            if (RepeatedOperation.Constatnt.IsNull())
            {
                RepeatedOperation.Constatnt = Operand.GrabConstant();
            }
            else
            {
                RepeatedOperation.Constatnt = RepeatedOperation.Operator(RepeatedOperation.Constatnt, Operand.GrabConstant());
            }
        }
    };

    // First PASS: Rearrange operands in commutative operations in a specific order and collect repeated operations.
    std::vector<SRepeatedOperation> RepeatedOps;
    std::vector<bool> OpsUsed(mOperations.size(), false);
    OpsUsed.back() = true;
    for (int OpIndex = static_cast<int>(mOperations.size()) - 1; OpIndex >= 0; OpIndex--)
    {
        if (!OpsUsed[OpIndex])
        {
            continue;
        }

        SOperation& Op = mOperations[OpIndex];
        Op.MarkOperationFlags(OpsUsed);

        if (!Op.Operator.IsCommutative())
        {
            continue;
        }

        assert(Op.Operator.GetArity() == 2);

        COperand& Left = Op.Left;
        COperand& Right = Op.Right;
        bool ToAddRepeatedOp = false;
        auto CheckReferencedOperatorRepeated = [&](COperand& Operand)
        {
            if (mOperations[Operand.GrabOperationIndex()].Operator == Op.Operator)
            {
                ToAddRepeatedOp = true;
            }
        };

        // Rearrange the two operands in the following order: fact, operation, and constant.
        if (Left.IsConstant())
        {
            if (Right.IsFact())
            {
                std::swap(Left, Right);
            }
            else if (Right.IsOperation())
            {
                CheckReferencedOperatorRepeated(Right);
                std::swap(Left, Right);
            }
            // Keep the case of two Boolean constants intact, as they will be merged into one in repeated operations.
        }
        else if (Left.IsFact())
        {
            if (Right.IsFact())
            {
                if (Left.GrabFactIndex() > Right.GrabFactIndex())
                {
                    std::swap(Left, Right);
                }
            }
            else if (Right.IsOperation())
            {
                CheckReferencedOperatorRepeated(Right);
            }
        }
        else if (Left.IsOperation())
        {
            if (Right.IsConstant())
            {
                CheckReferencedOperatorRepeated(Left);
            }
            else if (Right.IsFact())
            {
                CheckReferencedOperatorRepeated(Left);
                std::swap(Left, Right);
            }
            else
            {
                assert((Right.IsOperation()));

                if (Left.GrabOperationIndex() == Right.GrabOperationIndex())
                {
                    CheckReferencedOperatorRepeated(Right);
                }
                else
                {
                    // If the two operands reference different operations, they are added to a repeated operation to be sorted in the second pass.
                    ToAddRepeatedOp = true;
                }
            }
        }

        // Replace the current operation in the repeated operations with its operands.
        bool AlreadyInRepeatedOp = false;
        for (int RepeatedOpIndex = 0; RepeatedOpIndex < RepeatedOps.size(); RepeatedOpIndex++)
        {
            SRepeatedOperation& RepeatedOp = RepeatedOps[RepeatedOpIndex];
            if (Op.Operator == RepeatedOp.Operator) // Check if the current operation is repeated.
            {
                for (int OpIndexOfRepeatedOp = static_cast<int>(RepeatedOp.OperationIndexes.size()) - 1; OpIndexOfRepeatedOp >= 0; OpIndexOfRepeatedOp--)
                {
                    if (OpIndex == RepeatedOp.OperationIndexes[OpIndexOfRepeatedOp]) // Check if this operation is referenced by the current repeated operation.
                    {
                        RepeatedOp.OperationIndexes.erase(RepeatedOp.OperationIndexes.begin() + OpIndexOfRepeatedOp);
                        CollectRepeatedOperation(RepeatedOp, Op.Left);
                        CollectRepeatedOperation(RepeatedOp, Op.Right);
                        AlreadyInRepeatedOp = true;
                    }
                }
            }
        }

        // Add a new repeated operation starting with the current operation if no corresponding one exists.
        if (ToAddRepeatedOp && !AlreadyInRepeatedOp)
        {
            SRepeatedOperation& RepeatedOp = RepeatedOps.emplace_back(Op.Operator, OpIndex);
            CollectRepeatedOperation(RepeatedOp, Op.Left);
            CollectRepeatedOperation(RepeatedOp, Op.Right);
        }
    }

    // Second PASS:
    // 1. Rearrange the operands of the repeated operations in order.
    // 2. Replace the source operations with the corresponding repeated operations and update operation indexes.
    // 3. Call RemoveUnusedOperations to clean up (later in Rearrange).
    assert(std::ranges::is_sorted(RepeatedOps, SRepeatedOperation::IsSourceOperationIndexGreaterThanAnother));
    for (int RepeatedOpIndex = static_cast<int>(RepeatedOps.size()) - 1; RepeatedOpIndex >= 0; RepeatedOpIndex--)
    {
        SRepeatedOperation& RepeatedOp = RepeatedOps[RepeatedOpIndex];
        CNumber Constant = RepeatedOp.Constatnt;
        std::vector<int>& FactIndexes = RepeatedOp.FactIndexes;
        std::vector<int>& OpIndexes = RepeatedOp.OperationIndexes;
        assert(RepeatedOp.Operator.IsCommutative());
        assert(RepeatedOp.SourceOperationIndex >= 0);

        // Sort referenced operations and facts.
        SortSubExpressions(OpIndexes);
        std::ranges::sort(RepeatedOp.FactIndexes);

        if (OpIndexes.size() == 2 && FactIndexes.empty()
            && (Constant.IsNull() || Constant == RepeatedOp.Operator.GetRightIdentity()))
        {
            // With only two operation references, we just need to rearrange them.
            SOperation& Op = mOperations[RepeatedOp.SourceOperationIndex];
            Op.Left.SetOperation(OpIndexes[0]);
            Op.Right.SetOperation(OpIndexes[1]);
        }
        else
        {
            CExpression RepeatedOpExpression = RepeatedOp.ToExpression();
            auto OpIndexMap = ReplaceOperation(RepeatedOp.SourceOperationIndex, std::move(RepeatedOpExpression));

            // Update only the repeated operations preceding the current one, since the others have been used.
            for (int i = RepeatedOpIndex - 1; i >= 0; i--)
            {
                RepeatedOps[i].UpdateOperationIndexes(OpIndexMap);
            }
        }
    }
}

void CExpression::SortSubExpressions(std::vector<int>& OperationIndexes) const
{
    if (OperationIndexes.size() <= 1)
    {
        return;
    }

    std::vector<SIndexedSizeAndText> SubExpressionTuples;
    SubExpressionTuples.reserve(OperationIndexes.size());

    for (int i = 0; i < OperationIndexes.size(); i++)
    {
        int OpIndex = OperationIndexes[i];
        std::vector<int> OpIndexes = GetUsedOperationIndexes(OpIndex);
        std::ranges::sort(OpIndexes);
        SubExpressionTuples.emplace_back(OpIndex, AddUpArity(OpIndexes), ToString(OpIndexes));
    }

    std::ranges::sort(SubExpressionTuples, &SIndexedSizeAndText::Compare);

    for (int i = 0; i < OperationIndexes.size(); i++)
    {
        OperationIndexes[i] = SubExpressionTuples[i].Index;
    }
}

void CExpression::RemoveConstantOperations()
{
    if (mOperations.empty())
    {
        return;
    }

    std::vector<COperand> Results(mOperations.size()); // Results of constant or univariate operations

    auto FectchResult = [&](const COperand& Operand)
    {
        if (Operand.IsOperation())
        {
            const int RefOpIndex = Operand.GrabOperationIndex();
            const COperand& Result = Results[RefOpIndex];
            if (!Result.IsNull())
            {
                return Result;
            }
        }

        return Operand;
    };

    // Constant and univariate operations will be deleted after their results are calculated. 
    // Hence, we build a mapping to new operation indexes for updating them after the deletion.
    std::vector<int> NewOperationIndexes(mOperations.size());
    for (int OpIndex = 0; OpIndex < NewOperationIndexes.size(); OpIndex++)
    {
        NewOperationIndexes[OpIndex] = OpIndex;
    }

    // Find constant and univariate operations, then calculate the results. 
    for (int OpIndex = 0; OpIndex < mOperations.size(); OpIndex++)
    {
        const SOperation& Op = mOperations[OpIndex];
        const COperand& Right = FectchResult(Op.Right);
        if (!Op.HasOperator())
        {
            Results[OpIndex] = Right;
        }
        else 
        {
            int Arity = Op.Operator.GetArity();
            if (Arity == 1)
            {
                if (Right.IsConstant())
                {
                    Results[OpIndex] = Op.Operator(CNumber::Null, Right.GrabConstant());
                }
            }
            else if (Arity == 2)
            {
                const COperand& Left = FectchResult(Op.Left);
                if (Left.IsConstant() && Right.IsConstant())
                {
                    Results[OpIndex] = Op.Operator(Left.GrabConstant(), Right.GrabConstant());
                }
                else if (Left.IsConstant() && Left.GrabConstant() == Op.Operator.GetLeftIdentity())
                {
                    Results[OpIndex] = Right;
                }
                else if (Right.IsConstant() && Right.GrabConstant() == Op.Operator.GetRightIdentity())
                {
                    Results[OpIndex] = Left;
                }
            }
        }

        if (!Results[OpIndex].IsNull())
        {
            // The current operation will be deleted, so subsequent operation indexes need to be updated.
            for (int i = OpIndex + 1; i < NewOperationIndexes.size(); i++)
            {
                NewOperationIndexes[i]--;
            }
        }
    }

    auto UpdateOperationIndex = [&](COperand& Operand)
    {
        if (!Operand.IsOperation())
        {
            return;
        }

        const int RefOpIndex = Operand.GrabOperationIndex();
        const COperand& Result = Results[RefOpIndex];
        if (Result.IsNull())
        {
            Operand.SetOperationIndex(NewOperationIndexes[RefOpIndex]);
        }
        else
        {
            Operand = Result;
        }
    };

    const COperand& LastResult = Results.back();
    if (LastResult.IsFact() || LastResult.IsConstant())
    {
        // 1. If all operands are constant, they evaluate to a single constant.
        // 2. If all constants cancel out and only a variable remains, the result is that variable.
        SetTo(LastResult);
    }
    else 
    {
        // Update cached operation indexes for deletion.
        for (int OpIndex = 0; OpIndex < Results.size(); OpIndex++)
        {
            COperand& Result = Results[OpIndex];
            if (Result.IsOperation())
            {
                const int RefOpIndex = Result.GrabOperationIndex();
                Result.SetOperationIndex(NewOperationIndexes[RefOpIndex]);
            }
        }

        // Delete constant or univariate operations and replace the operands referencing them.
        for (int OpIndex = static_cast<int>(mOperations.size()) - 1; OpIndex >= 0; OpIndex--)
        {
            if (Results[OpIndex].IsNull())
            {
                UpdateOperationIndex(mOperations[OpIndex].Left);
                UpdateOperationIndex(mOperations[OpIndex].Right);
            }
            else
            {
                mOperations.erase(mOperations.begin() + OpIndex);
            }
        }
    }
}

void CExpression::RemoveIdentityElements()
{
    if (mOperations.empty())
    {
        return; // Early return to preserve empty expressions.
    }

    std::vector<COperand> Results(mOperations.size()); // Cached results of operations with identity elements
    std::vector<int> NewOpIndexes(mOperations.size());
    for (int OpIndex = 0; OpIndex < NewOpIndexes.size(); OpIndex++)
    {
        NewOpIndexes[OpIndex] = OpIndex;
    }

    for (int OpIndex = 0; OpIndex < mOperations.size(); OpIndex++)
    {
        const SOperation& Op = mOperations[OpIndex];
        if (Op.Operator.GetArity() != 2)
        {
            continue;
        }

        const COperand& Left = Op.Left;
        const COperand& Right = Op.Right;
        if (Left.IsConstant() && Left.GrabConstant() == Op.Operator.GetLeftIdentity())
        {
            Results[OpIndex] = Right;
        }
        else if (Right.IsConstant() && Right.GrabConstant() == Op.Operator.GetRightIdentity())
        {
            Results[OpIndex] = Left;
        }

        if (!Results[OpIndex].IsNull())
        {
            // Update the cache to the new operation index if it's an operation reference.
            if (Results[OpIndex].IsOperation())
            {
                int RefOpIndex = Results[OpIndex].GrabOperationIndex();
                if (Results[RefOpIndex].IsNull())
                {
                    Results[OpIndex].SetOperation(NewOpIndexes[RefOpIndex]);
                }
                else
                {
                    Results[OpIndex] = Results[RefOpIndex];
                }
            }

            // The current operation will be deleted, so subsequent operation indexes need to be updated.
            for (int IndexOfOpIndexes = OpIndex + 1; IndexOfOpIndexes < NewOpIndexes.size(); IndexOfOpIndexes++)
            {
                NewOpIndexes[IndexOfOpIndexes]--;
            }
        }
    }

    auto Replace = [&](COperand& Operand)
    {
        if (Operand.IsOperation())
        {
            const COperand& Result = Results[Operand.GrabOperationIndex()];
            if (Result.IsNull())
            {
                Operand.SetOperation(NewOpIndexes[Operand.GrabOperationIndex()]);
            }
            else
            {
                Operand = Result;
            }
        }
    };

    // Delete the operations with identity elements and replace the operands referencing them.
    for (int OpIndex = static_cast<int>(mOperations.size()) - 1; OpIndex >= 0; OpIndex--)
    {
        if (Results[OpIndex].IsNull())
        {
            Replace(mOperations[OpIndex].Left);
            Replace(mOperations[OpIndex].Right);
        }
        else
        {
            mOperations.erase(mOperations.begin() + OpIndex);
        }
    }

    if (mOperations.empty())
    {
        // Even if all operations involve identity elements, we must leave at least one operand in the expression, or else it will be empty.
        assert(!Results.back().IsNull());
        assert(!Results.back().IsOperation());
        mOperations.emplace_back(EOperator::nil, COperand::Null, Results.back());
    }
}

void CExpression::RemoveIneffectiveOperations()
{
    // TODO: Remove subexpressions multiplied by zero.
    // TODO: Remove modulo whose divisor is always greater than or equal to the dividend.
}

void CExpression::ShiftOperationIndexes(int Offset, int StartOperationIndex)
{
    for (int i = StartOperationIndex; i < mOperations.size(); i++)
    {
        mOperations[i].ShiftOperationIndex(Offset);
    }
}

CNumber CExpression::SplitInto(std::vector<STerm>& oTerms, CNumber OuterScale) const
{
    if (mOperations.empty())
    {
        return CNumber::Null;
    }

    CNullStableNumber ConstantTerm = CNullStableNumber::Null;

    struct ScaledIndex
    {
        int Index = InvalidIndex;
        CNumber Scale = 0;
    };
    std::vector<ScaledIndex> ScaledIndexes;
    ScaledIndexes.reserve(mOperations.size());

    auto AddTerm = [&](const COperand& Operand, CNumber Scale)
    {
        if (Scale == 0)
        {
            return;
        }

        switch (Operand.GetType())
        {
        case EOperand::fact:
            oTerms.emplace_back(Operand, Scale);
            break;
        case EOperand::operation:
            ScaledIndexes.emplace_back(Operand.GrabOperationIndex(), Scale);
            break;
        case EOperand::constant:
            ConstantTerm += Operand.GrabConstant() * Scale;
            break;
        }
    };

    ScaledIndexes.emplace_back(static_cast<int>(mOperations.size()) - 1, OuterScale);
    for (int i = 0; i < ScaledIndexes.size(); i++)
    {
        const int OpIndex = ScaledIndexes[i].Index;
        CNumber Scale = ScaledIndexes[i].Scale;
        const SOperation& Op = mOperations[OpIndex];

        if (Op.Operator.IsNil())
        {
            AddTerm(Op.Right, Scale);
        }
        else
        {
            if (Op.IsAddition())
            {
                AddTerm(Op.Left, Scale);
                AddTerm(Op.Right, Scale);
            }
            else if (Op.IsSubtraction())
            {
                AddTerm(Op.Left, Scale);
                AddTerm(Op.Right, -Scale);
            }
            else if (Op.IsMinus())
            {
                AddTerm(Op.Right, -Scale);
            }
            else if (Op.Left.IsConstant() && Op.IsMultiplication())
            {
                CNumber LeftConstant = Op.Left.GrabConstant();
                Scale *= LeftConstant;
                AddTerm(Op.Right, Scale);
            }
            else if (Op.Right.IsConstant() &&
                (Op.IsMultiplication() || (Op.IsDivision() && Op.Right.GrabConstant() != 0)))
            {
                CNumber RightConstant = Op.Right.GrabConstant();
                if (Op.IsDivision())
                {
                    RightConstant = 1 / RightConstant;
                }
                Scale *= RightConstant;
                AddTerm(Op.Left, Scale);
            }
            else
            {
                oTerms.emplace_back(GetSubExpression(OpIndex), Scale);
            }
        }
    }

    return ConstantTerm;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
